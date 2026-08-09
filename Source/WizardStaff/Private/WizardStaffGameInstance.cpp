#include "WizardStaffGameInstance.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/NetworkVersion.h"
#include "Modules/ModuleManager.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "WizardStaffMainMenuPlayerController.h"
#include "WizardStaffPlayerState.h"

namespace
{
const FName WizardSteamSubsystemName(TEXT("Steam"));
const FName WizardSteamSmokeSessionName = NAME_GameSession;
const FName WizardSteamMapSettingKey(TEXT("WIZARDSTAFF_MAP"));
const FName WizardSteamBuildSettingKey(TEXT("WIZARDSTAFF_BUILD"));
const FString WizardSteamPrototypeMapPath(TEXT("/Game/Maps/WizardStaff_Prototype"));
const FString WizardStaffMainMenuMapPath(TEXT("/Game/Maps/WizardStaff_MainMenu"));
const FString WizardSteamFavorLeaderboardName(TEXT("WizardStaff_BestGrandWizardFavor"));
const FString WizardSteamFavorRatedStatName(TEXT("GrandWizardFavor"));
constexpr int32 WizardSteamSmokeMaxPlayers = 2;
constexpr float WizardSteamSearchTimeoutSeconds = 30.0f;
constexpr float WizardSteamJoinTimeoutSeconds = 30.0f;

uint32 GetWizardSteamNetworkVersion()
{
	return FNetworkVersion::GetLocalNetworkVersion();
}

int32 GetWizardSteamBuildUniqueId()
{
	return static_cast<int32>(GetWizardSteamNetworkVersion());
}

FString GetWizardSteamBuildValue()
{
	const FString& ProjectVersion = FNetworkVersion::GetProjectVersion();
	return FString::Printf(
		TEXT("%s-%08X"),
		ProjectVersion.IsEmpty() ? TEXT("Unversioned") : *ProjectVersion,
		GetWizardSteamNetworkVersion());
}

bool IsWizardPrototypeGameplayWorld(const UWorld* World)
{
	return World && World->GetMapName().Contains(TEXT("WizardStaff_Prototype"));
}

const TCHAR* SteamJoinResultToText(EOnJoinSessionCompleteResult::Type Result)
{
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		return TEXT("Success");
	case EOnJoinSessionCompleteResult::SessionIsFull:
		return TEXT("SessionIsFull");
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		return TEXT("SessionDoesNotExist");
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		return TEXT("CouldNotRetrieveAddress");
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		return TEXT("AlreadyInSession");
	case EOnJoinSessionCompleteResult::UnknownError:
	default:
		return TEXT("UnknownError");
	}
}
}

void UWizardStaffGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff network compatibility: projectVersion=%s build=%s buildUniqueId=%d."),
		*FNetworkVersion::GetProjectVersion(),
		*GetWizardSteamBuildValue(),
		GetWizardSteamBuildUniqueId());

	if (GEngine)
	{
		NetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject(this, &UWizardStaffGameInstance::OnNetworkFailure);
		TravelFailureDelegateHandle = GEngine->OnTravelFailure().AddUObject(this, &UWizardStaffGameInstance::OnTravelFailure);
	}
}

void UWizardStaffGameInstance::Shutdown()
{
	ClearFrontendRequestTimeout();
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
		GEngine->OnTravelFailure().Remove(TravelFailureDelegateHandle);
	}
	NetworkFailureDelegateHandle.Reset();
	TravelFailureDelegateHandle.Reset();
	ClearSteamLeaderboardDelegate(GetSteamLeaderboardsInterface(false));
	ClearSteamSessionDelegates(GetSteamSessionInterface(false));
	SteamSessionSearch.Reset();
	ResetSteamMatchSubmissionTracking();

	Super::Shutdown();
}

bool UWizardStaffGameInstance::IsFrontendRequestBusy() const
{
	return FrontendState == EWizardStaffFrontendState::StartingLocal
		|| FrontendState == EWizardStaffFrontendState::CreatingHostSession
		|| FrontendState == EWizardStaffFrontendState::HostTraveling
		|| FrontendState == EWizardStaffFrontendState::SearchingSessions
		|| FrontendState == EWizardStaffFrontendState::JoiningSession
		|| FrontendState == EWizardStaffFrontendState::ClientTraveling;
}

int32 UWizardStaffGameInstance::BeginFrontendRequest(EWizardStaffFrontendState NewState, const FText& StatusText)
{
	if (IsFrontendRequestBusy())
	{
		UE_LOG(LogTemp, Warning, TEXT("Wizard Staff frontend ignored a duplicate request while state %d is active."), static_cast<int32>(FrontendState));
		return INDEX_NONE;
	}

	++FrontendRequestGeneration;
	SetFrontendState(NewState, StatusText);
	return FrontendRequestGeneration;
}

bool UWizardStaffGameInstance::IsCurrentFrontendRequest(int32 RequestGeneration) const
{
	return RequestGeneration != INDEX_NONE && RequestGeneration == FrontendRequestGeneration;
}

void UWizardStaffGameInstance::SetFrontendState(EWizardStaffFrontendState NewState, const FText& StatusText)
{
	FrontendState = NewState;
	FrontendStatusText = StatusText;
}

void UWizardStaffGameInstance::FailFrontendRequest(int32 RequestGeneration, const FText& StatusText, const TCHAR* LogContext)
{
	if (RequestGeneration != INDEX_NONE && !IsCurrentFrontendRequest(RequestGeneration))
	{
		UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend ignored stale failure callback: %s."), LogContext ? LogContext : TEXT("unknown"));
		return;
	}

	ClearFrontendRequestTimeout();
	SetFrontendState(EWizardStaffFrontendState::Error, StatusText);
	UE_LOG(LogTemp, Warning, TEXT("Wizard Staff frontend request failed: %s."), LogContext ? LogContext : TEXT("unknown"));
}

void UWizardStaffGameInstance::ArmFrontendRequestTimeout(int32 RequestGeneration, EWizardStaffFrontendState ExpectedState, float TimeoutSeconds)
{
	ClearFrontendRequestTimeout();
	UWorld* World = GetWorld();
	if (!World || RequestGeneration == INDEX_NONE)
	{
		return;
	}

	FTimerDelegate TimeoutDelegate;
	TimeoutDelegate.BindUObject(this, &UWizardStaffGameInstance::OnFrontendRequestTimeout, RequestGeneration, ExpectedState);
	World->GetTimerManager().SetTimer(FrontendRequestTimeoutTimerHandle, TimeoutDelegate, TimeoutSeconds, false);
}

void UWizardStaffGameInstance::ClearFrontendRequestTimeout()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FrontendRequestTimeoutTimerHandle);
	}
	FrontendRequestTimeoutTimerHandle.Invalidate();
}

void UWizardStaffGameInstance::OnFrontendRequestTimeout(int32 RequestGeneration, EWizardStaffFrontendState ExpectedState)
{
	if (!IsCurrentFrontendRequest(RequestGeneration) || FrontendState != ExpectedState)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface(false);
	if (SessionInterface.IsValid()
		&& ExpectedState == EWizardStaffFrontendState::SearchingSessions
		&& SteamSessionSearch.IsValid())
	{
		SessionInterface->CancelFindSessions();
	}
	if (bFindSteamSessionAfterDestroy)
	{
		ClearSteamSessionDelegates(SessionInterface);
	}
	else
	{
		ClearSteamFindAndJoinDelegates(SessionInterface);
	}
	SteamSessionSearch.Reset();
	bFindSteamSessionAfterDestroy = false;
	PendingSteamFindRequestGeneration = INDEX_NONE;
	PendingSteamJoinRequestGeneration = INDEX_NONE;

	const bool bSearchTimedOut = ExpectedState == EWizardStaffFrontendState::SearchingSessions;
	FailFrontendRequest(
		RequestGeneration,
		FText::FromString(bSearchTimedOut
			? TEXT("Search timed out. Confirm both players use the same Steam beta and try again.")
			: TEXT("Joining timed out. Please try again.")),
		bSearchTimedOut ? TEXT("Steam lobby search timed out") : TEXT("Steam lobby join timed out"));
}

void UWizardStaffGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Warning, TEXT("Wizard Staff network failure: type=%s driver=%s error=%s frontendState=%d."),
		ENetworkFailure::ToString(FailureType),
		NetDriver ? *NetDriver->GetDescription() : TEXT("<none>"),
		ErrorString.IsEmpty() ? TEXT("<none>") : *ErrorString,
		static_cast<int32>(FrontendState));

	if (World && World->GetNetMode() == NM_Client && IsWizardPrototypeGameplayWorld(World))
	{
		if (!bReturningToMenuAfterGameplayNetworkFailure)
		{
			bReturningToMenuAfterGameplayNetworkFailure = true;
			GameplayNetworkFailureStatusText = FText::FromString(TEXT("Connection to the host was lost. You can host or join another game."));
			World->GetTimerManager().SetTimerForNextTick(this, &UWizardStaffGameInstance::ReturnToMainMenuAfterGameplayNetworkFailure);
		}
		return;
	}

	if (FrontendState == EWizardStaffFrontendState::ClientTraveling)
	{
		SetFrontendState(EWizardStaffFrontendState::Error, FText::FromString(TEXT("Connection failed or timed out. Please try again.")));
		return;
	}
}

void UWizardStaffGameInstance::OnTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	if (World && World->GetNetMode() == NM_Client && IsWizardPrototypeGameplayWorld(World))
	{
		if (!bReturningToMenuAfterGameplayNetworkFailure)
		{
			bReturningToMenuAfterGameplayNetworkFailure = true;
			GameplayNetworkFailureStatusText = FText::FromString(TEXT("The online game could not continue. You can host or join another game."));
			World->GetTimerManager().SetTimerForNextTick(this, &UWizardStaffGameInstance::ReturnToMainMenuAfterGameplayNetworkFailure);
		}
	}
	else if (FrontendState == EWizardStaffFrontendState::ClientTraveling)
	{
		SetFrontendState(EWizardStaffFrontendState::Error, FText::FromString(TEXT("Could not travel to the hosted game.")));
	}
	else
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Wizard Staff Steam client travel failure: type=%s error=%s."),
		ETravelFailure::ToString(FailureType),
		ErrorString.IsEmpty() ? TEXT("<none>") : *ErrorString);
}

void UWizardStaffGameInstance::ReturnToMainMenuAfterGameplayNetworkFailure()
{
	bReturningToMenuAfterGameplayNetworkFailure = false;
	const FText StatusText = GameplayNetworkFailureStatusText;
	GameplayNetworkFailureStatusText = FText::GetEmpty();
	ReturnToMainMenuInternal(StatusText);
}

void UWizardStaffGameInstance::PrepareFrontendForGameplayTravel()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (AWizardStaffMainMenuPlayerController* MenuController = Cast<AWizardStaffMainMenuPlayerController>(World->GetFirstPlayerController()))
	{
		MenuController->PrepareForGameplayTravel();
	}
}

void UWizardStaffGameInstance::RemoveExtraLocalPlayersForOnlineTravel(const TCHAR* Context)
{
	int32 RemovedPlayerCount = 0;
	for (int32 LocalPlayerIndex = GetNumLocalPlayers() - 1; LocalPlayerIndex >= 1; --LocalPlayerIndex)
	{
		if (ULocalPlayer* ExtraLocalPlayer = GetLocalPlayerByIndex(LocalPlayerIndex))
		{
			if (RemoveLocalPlayer(ExtraLocalPlayer))
			{
				++RemovedPlayerCount;
			}
		}
	}

	if (RemovedPlayerCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend removed %d extra local player(s) before %s."),
			RemovedPlayerCount,
			Context ? Context : TEXT("online travel"));
	}
}

void UWizardStaffGameInstance::StartLocalPrototypeFromMenu()
{
	const int32 RequestGeneration = BeginFrontendRequest(EWizardStaffFrontendState::StartingLocal, FText::FromString(TEXT("Starting local game...")));
	if (RequestGeneration == INDEX_NONE)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not start the gameplay map.")), TEXT("local start has no World"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend local game requested; opening %s."), *WizardSteamPrototypeMapPath);
	PrepareFrontendForGameplayTravel();
	UGameplayStatics::OpenLevel(World, FName(*WizardSteamPrototypeMapPath));
}

void UWizardStaffGameInstance::StartSteamHostFromMenu()
{
	StartSteamHostRequest(true);
}

void UWizardStaffGameInstance::StartSteamJoinFromMenu()
{
	StartSteamJoinRequest(true);
}

void UWizardStaffGameInstance::CancelFrontendRequest()
{
	if (FrontendState != EWizardStaffFrontendState::SearchingSessions && FrontendState != EWizardStaffFrontendState::JoiningSession)
	{
		return;
	}

	ClearFrontendRequestTimeout();
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (SessionInterface.IsValid()
		&& FrontendState == EWizardStaffFrontendState::SearchingSessions
		&& SteamSessionSearch.IsValid())
	{
		SessionInterface->CancelFindSessions();
	}
	if (bFindSteamSessionAfterDestroy)
	{
		ClearSteamSessionDelegates(SessionInterface);
	}
	else
	{
		ClearSteamFindAndJoinDelegates(SessionInterface);
	}
	SteamSessionSearch.Reset();
	bFindSteamSessionAfterDestroy = false;
	PendingSteamFindRequestGeneration = INDEX_NONE;
	PendingSteamJoinRequestGeneration = INDEX_NONE;
	++FrontendRequestGeneration;
	SetFrontendState(EWizardStaffFrontendState::Idle, FText::FromString(TEXT("Search canceled.")));
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend search/join request canceled."));
}

void UWizardStaffGameInstance::ReturnToMainMenu()
{
	ReturnToMainMenuInternal(FText::GetEmpty());
}

void UWizardStaffGameInstance::ReturnToMainMenuInternal(const FText& StatusText)
{
	ClearFrontendRequestTimeout();
	ClearSteamFindAndJoinDelegates(GetSteamSessionInterface(false));
	SteamSessionSearch.Reset();
	PendingSteamFindRequestGeneration = INDEX_NONE;
	PendingSteamJoinRequestGeneration = INDEX_NONE;
	PendingSteamHostRequestGeneration = INDEX_NONE;
	PendingSteamCreateRequestGeneration = INDEX_NONE;
	bCreateSteamSessionAfterDestroy = false;
	bFindSteamSessionAfterDestroy = false;
	++FrontendRequestGeneration;
	SetFrontendState(
		StatusText.IsEmpty() ? EWizardStaffFrontendState::Idle : EWizardStaffFrontendState::Error,
		StatusText);
	ResetSteamMatchSubmissionTracking();

	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface(false);
	if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(WizardSteamSmokeSessionName))
	{
		ClearSteamSessionDelegates(SessionInterface);
		bReturnToMainMenuAfterSessionDestroy = true;
		DestroySteamSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnDestroySteamSmokeSessionComplete));

		UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend leaving Steam session before returning to the main menu."));
		if (SessionInterface->DestroySession(WizardSteamSmokeSessionName))
		{
			return;
		}

		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySteamSessionCompleteDelegateHandle);
		DestroySteamSessionCompleteDelegateHandle.Reset();
		bReturnToMainMenuAfterSessionDestroy = false;
		UE_LOG(LogTemp, Warning, TEXT("Wizard Staff frontend could not start Steam session teardown; returning to the main menu anyway."));
	}

	OpenMainMenuAfterSessionTeardown();
}

void UWizardStaffGameInstance::SetSteamSessionJoinability(bool bJoinable, const TCHAR* Context)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() != NM_ListenServer)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface(false);
	FNamedOnlineSession* NamedSession = SessionInterface.IsValid()
		? SessionInterface->GetNamedSession(WizardSteamSmokeSessionName)
		: nullptr;
	if (!NamedSession)
	{
		return;
	}

	FOnlineSessionSettings UpdatedSettings = NamedSession->SessionSettings;
	const bool bAlreadyMatches = UpdatedSettings.bShouldAdvertise == bJoinable
		&& UpdatedSettings.bAllowJoinInProgress == bJoinable
		&& UpdatedSettings.bAllowJoinViaPresence == bJoinable;
	if (bAlreadyMatches)
	{
		return;
	}

	UpdatedSettings.bShouldAdvertise = bJoinable;
	UpdatedSettings.bAllowJoinInProgress = bJoinable;
	UpdatedSettings.bAllowJoinViaPresence = bJoinable;
	UpdatedSettings.bAllowJoinViaPresenceFriendsOnly = false;
	if (!SessionInterface->UpdateSession(WizardSteamSmokeSessionName, UpdatedSettings, true))
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff could not update Steam session joinability to %s at %s."),
			bJoinable ? TEXT("open") : TEXT("closed"),
			Context ? Context : TEXT("unknown boundary"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardStaff Steam session joinability update requested: %s at %s."),
		bJoinable ? TEXT("open") : TEXT("closed"),
		Context ? Context : TEXT("unknown boundary"));
}

void UWizardStaffGameInstance::OpenMainMenuAfterSessionTeardown()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend returning to the main menu."));
	UGameplayStatics::OpenLevel(World, FName(*WizardStaffMainMenuMapPath));
}

void UWizardStaffGameInstance::WizardSteamHost()
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost is disabled in shipping builds."));
	return;
#else
	StartSteamHostRequest(false);
#endif
}

void UWizardStaffGameInstance::DebugSteamHostSession()
{
	WizardSteamHost();
}

void UWizardStaffGameInstance::WizardSteamJoinFirstSession()
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession is disabled in shipping builds."));
	return;
#else
	StartSteamJoinRequest(false);
#endif
}

void UWizardStaffGameInstance::DebugSteamFindAndJoinSession()
{
	WizardSteamJoinFirstSession();
}

void UWizardStaffGameInstance::StartSteamHostRequest(bool bFromFrontend)
{
	const int32 RequestGeneration = bFromFrontend
		? BeginFrontendRequest(EWizardStaffFrontendState::CreatingHostSession, FText::FromString(TEXT("Creating online session...")))
		: INDEX_NONE;
	if (bFromFrontend && RequestGeneration == INDEX_NONE)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost ignored on a remote client."));
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not create an online game.")), TEXT("host requested on remote client"));
		}
		return;
	}

	RemoveExtraLocalPlayersForOnlineTravel(TEXT("Steam host setup"));

	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost failed: Steam session interface is unavailable. Is Steam running, and is OnlineSubsystemSteam enabled?"));
		LogSteamUnavailableHint(TEXT("WizardSteamHost"));
		if (bFromFrontend)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Steam is unavailable.")), TEXT("Steam session interface unavailable"));
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamHost using OnlineSubsystem '%s' with configured Wizard Staff AppID 4954290."), *WizardSteamSubsystemName.ToString());

	ClearSteamSessionDelegates(SessionInterface);
	bFindSteamSessionAfterDestroy = false;
	PendingSteamHostRequestGeneration = RequestGeneration;
	if (SessionInterface->GetNamedSession(WizardSteamSmokeSessionName))
	{
		bCreateSteamSessionAfterDestroy = true;
		DestroySteamSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnDestroySteamSmokeSessionComplete));

		UE_LOG(LogTemp, Log, TEXT("WizardSteamHost destroying existing local smoke session before recreate."));
		if (!SessionInterface->DestroySession(WizardSteamSmokeSessionName))
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySteamSessionCompleteDelegateHandle);
			DestroySteamSessionCompleteDelegateHandle.Reset();
			bCreateSteamSessionAfterDestroy = false;
			UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost could not destroy existing local session; trying create path anyway."));
			CreateSteamSmokeSession(RequestGeneration);
		}
		return;
	}

	CreateSteamSmokeSession(RequestGeneration);
}

void UWizardStaffGameInstance::StartSteamJoinRequest(bool bFromFrontend)
{
	const int32 RequestGeneration = bFromFrontend
		? BeginFrontendRequest(EWizardStaffFrontendState::SearchingSessions, FText::FromString(TEXT("Searching for games...")))
		: INDEX_NONE;
	if (bFromFrontend && RequestGeneration == INDEX_NONE)
	{
		return;
	}

	RemoveExtraLocalPlayersForOnlineTravel(TEXT("Steam join setup"));

	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession failed: Steam session interface is unavailable. Is Steam running, and is OnlineSubsystemSteam enabled?"));
		LogSteamUnavailableHint(TEXT("WizardSteamJoinFirstSession"));
		if (bFromFrontend)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Steam is unavailable.")), TEXT("Steam session interface unavailable"));
		}
		return;
	}

	ClearSteamSessionDelegates(SessionInterface);
	bCreateSteamSessionAfterDestroy = false;
	bReturnToMainMenuAfterSessionDestroy = false;
	PendingSteamFindRequestGeneration = RequestGeneration;
	if (SessionInterface->GetNamedSession(WizardSteamSmokeSessionName))
	{
		bFindSteamSessionAfterDestroy = true;
		DestroySteamSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnDestroySteamSmokeSessionComplete));

		if (RequestGeneration != INDEX_NONE && IsCurrentFrontendRequest(RequestGeneration))
		{
			SetFrontendState(EWizardStaffFrontendState::SearchingSessions, FText::FromString(TEXT("Leaving previous online session...")));
		}
		UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession destroying existing local GameSession before search."));
		if (SessionInterface->DestroySession(WizardSteamSmokeSessionName))
		{
			ArmFrontendRequestTimeout(RequestGeneration, EWizardStaffFrontendState::SearchingSessions, WizardSteamSearchTimeoutSeconds);
			return;
		}

		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySteamSessionCompleteDelegateHandle);
		DestroySteamSessionCompleteDelegateHandle.Reset();
		bFindSteamSessionAfterDestroy = false;
		PendingSteamFindRequestGeneration = INDEX_NONE;
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not leave the previous online session.")), TEXT("previous joined session teardown did not start"));
		}
		return;
	}

	FindSteamSmokeSessions(RequestGeneration);
}

void UWizardStaffGameInstance::FindSteamSmokeSessions(int32 RequestGeneration)
{
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid())
	{
		PendingSteamFindRequestGeneration = INDEX_NONE;
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Steam is unavailable.")), TEXT("Steam session interface unavailable before search"));
		}
		return;
	}

	SteamSessionSearch = MakeShared<FOnlineSessionSearch>();
	SteamSessionSearch->bIsLanQuery = false;
	SteamSessionSearch->MaxSearchResults = 50;
	SteamSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	PendingSteamFindRequestGeneration = RequestGeneration;

	FindSteamSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnFindSteamSmokeSessionsComplete));

	UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession searching Steam lobbies before local compatibility filtering: map=%s build=%s buildUniqueId=%d."),
		*WizardSteamPrototypeMapPath,
		*GetWizardSteamBuildValue(),
		GetWizardSteamBuildUniqueId());
	if (!SessionInterface->FindSessions(0, SteamSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSteamSessionsCompleteDelegateHandle);
		FindSteamSessionsCompleteDelegateHandle.Reset();
		SteamSessionSearch.Reset();
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not search for games.")), TEXT("session search did not start"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession failed to start session search."));
		}
		return;
	}

	ArmFrontendRequestTimeout(RequestGeneration, EWizardStaffFrontendState::SearchingSessions, WizardSteamSearchTimeoutSeconds);
}

IOnlineSubsystem* UWizardStaffGameInstance::GetSteamSubsystem(bool bLoadOnDemand) const
{
	IOnlineSubsystem* SteamSubsystem = IOnlineSubsystem::Get(WizardSteamSubsystemName);
	if (SteamSubsystem || !bLoadOnDemand)
	{
		return SteamSubsystem;
	}

	if (!FModuleManager::Get().LoadModule(TEXT("OnlineSubsystemSteam")))
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff could not load OnlineSubsystemSteam for a Steam session request."));
		return nullptr;
	}

	SteamSubsystem = IOnlineSubsystem::Get(WizardSteamSubsystemName);
	if (SteamSubsystem)
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff loaded Steam OnlineSubsystem on demand: %s."), *SteamSubsystem->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff loaded OnlineSubsystemSteam but the Steam subsystem instance is unavailable."));
	}

	return SteamSubsystem;
}

IOnlineSessionPtr UWizardStaffGameInstance::GetSteamSessionInterface(bool bLoadOnDemand) const
{
	IOnlineSubsystem* SteamSubsystem = GetSteamSubsystem(bLoadOnDemand);
	return SteamSubsystem ? SteamSubsystem->GetSessionInterface() : nullptr;
}

IOnlineLeaderboardsPtr UWizardStaffGameInstance::GetSteamLeaderboardsInterface(bool bLoadOnDemand) const
{
	IOnlineSubsystem* SteamSubsystem = GetSteamSubsystem(bLoadOnDemand);
	return SteamSubsystem ? SteamSubsystem->GetLeaderboardsInterface() : nullptr;
}

void UWizardStaffGameInstance::SubmitServerDeliveredSteamMatchResult(
	const AWizardStaffPlayerState* DeliveryPlayerState,
	int32 MatchGeneration,
	int32 PlayerSlot,
	int32 WinnerSlot,
	int32 FinalGrandWizardFavor,
	int32 FinalRoundWins)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Standalone || MatchGeneration <= 0 || PlayerSlot < 0)
	{
		return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	const AWizardStaffPlayerState* LocalPlayerState = PlayerController
		? PlayerController->GetPlayerState<AWizardStaffPlayerState>()
		: nullptr;
	if (!DeliveryPlayerState
		|| DeliveryPlayerState != LocalPlayerState
		|| DeliveryPlayerState->IsPlaytestBotSlot()
		|| PlayerSlot != DeliveryPlayerState->GetWizardDisplaySlot())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam leaderboard rejected non-owner result delivery: deliveredState=%s localState=%s deliveredSlot=%d localSlot=%d bot=%s."),
			*GetNameSafe(DeliveryPlayerState),
			*GetNameSafe(LocalPlayerState),
			PlayerSlot,
			LocalPlayerState ? LocalPlayerState->GetWizardDisplaySlot() : INDEX_NONE,
			DeliveryPlayerState && DeliveryPlayerState->IsPlaytestBotSlot() ? TEXT("true") : TEXT("false"));
		return;
	}

	if (SubmittedSteamMatchGenerations.Contains(MatchGeneration))
	{
		UE_LOG(LogTemp, Verbose, TEXT("WizardStaff Steam leaderboard ignored duplicate match generation %d for P%d."),
			MatchGeneration,
			PlayerSlot + 1);
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid() || !SessionInterface->GetNamedSession(WizardSteamSmokeSessionName))
	{
		UE_LOG(LogTemp, Verbose, TEXT("WizardStaff Steam leaderboard skipped generation %d: no active Steam GameSession (direct-connect/local remains unchanged)."), MatchGeneration);
		return;
	}

	const FUniqueNetIdPtr LocalUserId = LocalPlayerState->GetUniqueId().GetUniqueNetId();
	if (!LocalUserId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam leaderboard skipped generation %d for P%d: local Steam user ID is unavailable."),
			MatchGeneration,
			PlayerSlot + 1);
		return;
	}

	IOnlineLeaderboardsPtr LeaderboardsInterface = GetSteamLeaderboardsInterface();
	if (!LeaderboardsInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam leaderboard skipped generation %d: Steam leaderboards interface is unavailable."), MatchGeneration);
		return;
	}

	const int32 SafeFavor = FMath::Max(FinalGrandWizardFavor, 0);
	const int32 SafeRoundWins = FMath::Max(FinalRoundWins, 0);
	const bool bFinalWinner = WinnerSlot != INDEX_NONE && PlayerSlot == WinnerSlot;

	FOnlineLeaderboardWrite WriteObject;
	WriteObject.LeaderboardNames.Add(WizardSteamFavorLeaderboardName);
	WriteObject.RatedStat = WizardSteamFavorRatedStatName;
	WriteObject.DisplayFormat = ELeaderboardFormat::Number;
	WriteObject.SortMethod = ELeaderboardSort::Descending;
	WriteObject.UpdateMethod = ELeaderboardUpdateMethod::KeepBest;
	WriteObject.SetIntStat(WizardSteamFavorRatedStatName, SafeFavor);

	ClearSteamLeaderboardDelegate(LeaderboardsInterface);
	SteamLeaderboardFlushCompleteDelegateHandle = LeaderboardsInterface->AddOnLeaderboardFlushCompleteDelegate_Handle(
		FOnLeaderboardFlushCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnSteamLeaderboardFlushComplete));

	if (!LeaderboardsInterface->WriteLeaderboards(WizardSteamSmokeSessionName, *LocalUserId, WriteObject))
	{
		ClearSteamLeaderboardDelegate(LeaderboardsInterface);
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam leaderboard failed to queue generation %d for P%d."),
			MatchGeneration,
			PlayerSlot + 1);
		return;
	}

	if (!LeaderboardsInterface->FlushLeaderboards(WizardSteamSmokeSessionName))
	{
		ClearSteamLeaderboardDelegate(LeaderboardsInterface);
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam leaderboard queued a write but failed to queue flush for generation %d P%d."),
			MatchGeneration,
			PlayerSlot + 1);
		return;
	}

	SubmittedSteamMatchGenerations.Add(MatchGeneration);
	PendingSteamLeaderboardMatchGeneration = MatchGeneration;
	UE_LOG(LogTemp, Log, TEXT("WizardStaff Steam leaderboard queued server-delivered owner result: generation=%d P%d favor=%d roundWins=%d finalWinner=%s leaderboard=%s."),
		MatchGeneration,
		PlayerSlot + 1,
		SafeFavor,
		SafeRoundWins,
		bFinalWinner ? TEXT("true") : TEXT("false"),
		*WizardSteamFavorLeaderboardName);
}

void UWizardStaffGameInstance::CreateSteamSmokeSession(int32 RequestGeneration)
{
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost failed: Steam session interface disappeared before create."));
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Steam is unavailable.")), TEXT("session interface disappeared before create"));
		}
		return;
	}
	PendingSteamCreateRequestGeneration = RequestGeneration;
	const int32 LocalBuildUniqueId = GetWizardSteamBuildUniqueId();
	const FString LocalBuildValue = GetWizardSteamBuildValue();

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.NumPublicConnections = WizardSteamSmokeMaxPlayers;
	SessionSettings.BuildUniqueId = LocalBuildUniqueId;
	SessionSettings.Set(WizardSteamMapSettingKey, WizardSteamPrototypeMapPath, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(WizardSteamBuildSettingKey, LocalBuildValue, EOnlineDataAdvertisementType::ViaOnlineService);

	CreateSteamSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnCreateSteamSmokeSessionComplete));

	UE_LOG(LogTemp, Log, TEXT("WizardSteamHost creating smoke session: map=%s maxPlayers=%d build=%s buildUniqueId=%d."),
		*WizardSteamPrototypeMapPath,
		WizardSteamSmokeMaxPlayers,
		*LocalBuildValue,
		LocalBuildUniqueId);

	if (!SessionInterface->CreateSession(0, WizardSteamSmokeSessionName, SessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSteamSessionCompleteDelegateHandle);
		CreateSteamSessionCompleteDelegateHandle.Reset();
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not create an online game.")), TEXT("session creation did not start"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost failed to start session creation."));
		}
	}
}

void UWizardStaffGameInstance::OnDestroySteamSmokeSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySteamSessionCompleteDelegateHandle);
	}
	DestroySteamSessionCompleteDelegateHandle.Reset();

	const bool bShouldCreate = bCreateSteamSessionAfterDestroy;
	const bool bShouldFind = bFindSteamSessionAfterDestroy;
	const bool bShouldReturnToMenu = bReturnToMainMenuAfterSessionDestroy;
	const int32 HostRequestGeneration = PendingSteamHostRequestGeneration;
	const int32 FindRequestGeneration = PendingSteamFindRequestGeneration;
	bCreateSteamSessionAfterDestroy = false;
	bFindSteamSessionAfterDestroy = false;
	bReturnToMainMenuAfterSessionDestroy = false;
	PendingSteamHostRequestGeneration = INDEX_NONE;

	UE_LOG(LogTemp, Log, TEXT("Wizard Staff Steam session destroy complete: session=%s success=%s continuation=%s."),
		*SessionName.ToString(),
		bWasSuccessful ? TEXT("true") : TEXT("false"),
		bShouldReturnToMenu ? TEXT("menu") : bShouldCreate ? TEXT("host") : bShouldFind ? TEXT("find") : TEXT("none"));

	if (bShouldReturnToMenu)
	{
		UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend Steam session teardown complete: session=%s success=%s."),
			*SessionName.ToString(),
			bWasSuccessful ? TEXT("true") : TEXT("false"));
		OpenMainMenuAfterSessionTeardown();
		return;
	}

	if (bShouldCreate)
	{
		CreateSteamSmokeSession(HostRequestGeneration);
		return;
	}

	if (bShouldFind)
	{
		if (!bWasSuccessful)
		{
			PendingSteamFindRequestGeneration = INDEX_NONE;
			if (FindRequestGeneration != INDEX_NONE)
			{
				FailFrontendRequest(FindRequestGeneration, FText::FromString(TEXT("Could not leave the previous online session.")), TEXT("previous joined session teardown failed"));
			}
			return;
		}

		FindSteamSmokeSessions(FindRequestGeneration);
	}
}

void UWizardStaffGameInstance::OnCreateSteamSmokeSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const int32 RequestGeneration = PendingSteamCreateRequestGeneration;
	PendingSteamCreateRequestGeneration = INDEX_NONE;
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSteamSessionCompleteDelegateHandle);
	}
	CreateSteamSessionCompleteDelegateHandle.Reset();

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost session creation failed: session=%s."), *SessionName.ToString());
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not create an online game.")), TEXT("session creation failed"));
		}
		return;
	}

	ResetSteamMatchSubmissionTracking();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost created session but could not open listen map because World is null."));
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not start the gameplay map.")), TEXT("host world unavailable after session creation"));
		}
		return;
	}
	if (RequestGeneration != INDEX_NONE && IsCurrentFrontendRequest(RequestGeneration))
	{
		SetFrontendState(EWizardStaffFrontendState::HostTraveling, FText::FromString(TEXT("Starting online game...")));
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamHost created session '%s'; opening %s?listen."),
		*SessionName.ToString(),
		*WizardSteamPrototypeMapPath);
	PrepareFrontendForGameplayTravel();
	UGameplayStatics::OpenLevel(World, FName(*WizardSteamPrototypeMapPath), true, TEXT("listen"));
}

void UWizardStaffGameInstance::OnFindSteamSmokeSessionsComplete(bool bWasSuccessful)
{
	ClearFrontendRequestTimeout();
	const int32 RequestGeneration = PendingSteamFindRequestGeneration;
	PendingSteamFindRequestGeneration = INDEX_NONE;
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSteamSessionsCompleteDelegateHandle);
	}
	FindSteamSessionsCompleteDelegateHandle.Reset();

	const int32 ResultCount = SteamSessionSearch.IsValid() ? SteamSessionSearch->SearchResults.Num() : 0;
	UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession search complete: success=%s results=%d."),
		bWasSuccessful ? TEXT("true") : TEXT("false"),
		ResultCount);

	if (RequestGeneration != INDEX_NONE && !IsCurrentFrontendRequest(RequestGeneration))
	{
		UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend ignored stale Steam search completion."));
		SteamSessionSearch.Reset();
		return;
	}

	if (!bWasSuccessful || !SessionInterface.IsValid() || !SteamSessionSearch.IsValid() || ResultCount <= 0)
	{
		if (bWasSuccessful && ResultCount <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession found zero Steam smoke sessions. Use two machines/two Steam accounts if same-machine Steam API initialization blocks the client."));
		}
		SteamSessionSearch.Reset();
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(
				RequestGeneration,
				bWasSuccessful && ResultCount <= 0
					? FText::FromString(TEXT("No compatible game found."))
					: FText::FromString(TEXT("Could not search for games.")),
				bWasSuccessful && ResultCount <= 0 ? TEXT("zero compatible sessions") : TEXT("session search failed"));
		}
		return;
	}

	int32 SelectedResultIndex = INDEX_NONE;
	const int32 LocalBuildUniqueId = GetWizardSteamBuildUniqueId();
	const FString LocalBuildValue = GetWizardSteamBuildValue();
	for (int32 ResultIndex = 0; ResultIndex < SteamSessionSearch->SearchResults.Num(); ++ResultIndex)
	{
		const FOnlineSessionSearchResult& SearchResult = SteamSessionSearch->SearchResults[ResultIndex];
		FString ResultMap;
		FString ResultBuild;
		SearchResult.Session.SessionSettings.Get(WizardSteamMapSettingKey, ResultMap);
		SearchResult.Session.SessionSettings.Get(WizardSteamBuildSettingKey, ResultBuild);

		const int32 ResultBuildUniqueId = SearchResult.Session.SessionSettings.BuildUniqueId;
		UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession result %d: map=%s build=%s buildUniqueId=%d expectedBuild=%s expectedBuildUniqueId=%d open=%d/%d ping=%d."),
			ResultIndex,
			ResultMap.IsEmpty() ? TEXT("<unset>") : *ResultMap,
			ResultBuild.IsEmpty() ? TEXT("<unset>") : *ResultBuild,
			ResultBuildUniqueId,
			*LocalBuildValue,
			LocalBuildUniqueId,
			SearchResult.Session.NumOpenPublicConnections,
			SearchResult.Session.SessionSettings.NumPublicConnections,
			SearchResult.PingInMs);

		if (ResultMap == WizardSteamPrototypeMapPath
			&& ResultBuild == LocalBuildValue)
		{
			if (ResultBuildUniqueId != LocalBuildUniqueId)
			{
				UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession accepting versioned metadata match despite Steam result BuildUniqueId translation: result=%d local=%d."),
					ResultBuildUniqueId,
					LocalBuildUniqueId);
			}
			SelectedResultIndex = ResultIndex;
			break;
		}
	}

	if (SelectedResultIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession found no compatible Wizard's Staff smoke session. Refusing to join unmatched Steam results."));
		SteamSessionSearch.Reset();
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(
				RequestGeneration,
				FText::FromString(TEXT("A game was found, but it uses a different Steam beta or build.")),
				TEXT("all search results failed compatibility checks"));
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession selected compatible result %d for join."), SelectedResultIndex);
	PendingSteamJoinRequestGeneration = RequestGeneration;
	if (RequestGeneration != INDEX_NONE && IsCurrentFrontendRequest(RequestGeneration))
	{
		SetFrontendState(EWizardStaffFrontendState::JoiningSession, FText::FromString(TEXT("Joining game...")));
	}

	JoinSteamSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnJoinSteamSmokeSessionComplete));

	if (!SessionInterface->JoinSession(0, WizardSteamSmokeSessionName, SteamSessionSearch->SearchResults[SelectedResultIndex]))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSteamSessionCompleteDelegateHandle);
		JoinSteamSessionCompleteDelegateHandle.Reset();
		SteamSessionSearch.Reset();
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not join the game.")), TEXT("session join did not start"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession failed to start join for result %d."), SelectedResultIndex);
		}
		return;
	}

	ArmFrontendRequestTimeout(RequestGeneration, EWizardStaffFrontendState::JoiningSession, WizardSteamJoinTimeoutSeconds);
}

void UWizardStaffGameInstance::OnJoinSteamSmokeSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	ClearFrontendRequestTimeout();
	const int32 RequestGeneration = PendingSteamJoinRequestGeneration;
	PendingSteamJoinRequestGeneration = INDEX_NONE;
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSteamSessionCompleteDelegateHandle);
	}
	JoinSteamSessionCompleteDelegateHandle.Reset();
	SteamSessionSearch.Reset();
	if (RequestGeneration != INDEX_NONE && !IsCurrentFrontendRequest(RequestGeneration))
	{
		UE_LOG(LogTemp, Log, TEXT("Wizard Staff frontend ignored stale Steam join completion."));
		return;
	}

	if (!SessionInterface.IsValid() || Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession join failed: session=%s result=%s (%d)."),
			*SessionName.ToString(),
			SteamJoinResultToText(Result),
			static_cast<int32>(Result));
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not join the game.")), TEXT("session join failed"));
		}
		return;
	}

	ResetSteamMatchSubmissionTracking();

	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectString) || ConnectString.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession joined session but could not resolve a connect string."));
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not join the game.")), TEXT("connect string unavailable"));
		}
		return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession resolved connect string but found no local PlayerController."));
		if (RequestGeneration != INDEX_NONE)
		{
			FailFrontendRequest(RequestGeneration, FText::FromString(TEXT("Could not join the game.")), TEXT("local player controller unavailable"));
		}
		return;
	}
	if (RequestGeneration != INDEX_NONE && IsCurrentFrontendRequest(RequestGeneration))
	{
		SetFrontendState(EWizardStaffFrontendState::ClientTraveling, FText::FromString(TEXT("Joining game...")));
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession join succeeded; client traveling to %s."), *ConnectString);
	PrepareFrontendForGameplayTravel();
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
}

void UWizardStaffGameInstance::OnSteamLeaderboardFlushComplete(FName SessionName, bool bWasSuccessful)
{
	const int32 CompletedGeneration = PendingSteamLeaderboardMatchGeneration;
	ClearSteamLeaderboardDelegate(GetSteamLeaderboardsInterface());
	PendingSteamLeaderboardMatchGeneration = INDEX_NONE;

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff Steam leaderboard flush complete: session=%s generation=%d success=true."),
			*SessionName.ToString(),
			CompletedGeneration);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam leaderboard flush complete: session=%s generation=%d success=false."),
			*SessionName.ToString(),
			CompletedGeneration);
	}
}

void UWizardStaffGameInstance::LogSteamUnavailableHint(const TCHAR* CommandName) const
{
	UE_LOG(LogTemp, Warning, TEXT("%s uses Wizard Staff AppID 4954290 and requires Steam to initialize OnlineSubsystemSteam. Same-machine multi-process Steam testing may still require separate Steam accounts or machines before changing net drivers."),
		CommandName ? CommandName : TEXT("WizardSteam command"));
}

void UWizardStaffGameInstance::ClearSteamSessionDelegates(const IOnlineSessionPtr& SessionInterface)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (DestroySteamSessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySteamSessionCompleteDelegateHandle);
		DestroySteamSessionCompleteDelegateHandle.Reset();
	}
	if (CreateSteamSessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSteamSessionCompleteDelegateHandle);
		CreateSteamSessionCompleteDelegateHandle.Reset();
	}
	if (FindSteamSessionsCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSteamSessionsCompleteDelegateHandle);
		FindSteamSessionsCompleteDelegateHandle.Reset();
	}
	if (JoinSteamSessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSteamSessionCompleteDelegateHandle);
		JoinSteamSessionCompleteDelegateHandle.Reset();
	}
}

void UWizardStaffGameInstance::ClearSteamFindAndJoinDelegates(const IOnlineSessionPtr& SessionInterface)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	if (FindSteamSessionsCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSteamSessionsCompleteDelegateHandle);
		FindSteamSessionsCompleteDelegateHandle.Reset();
	}
	if (JoinSteamSessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSteamSessionCompleteDelegateHandle);
		JoinSteamSessionCompleteDelegateHandle.Reset();
	}
}

void UWizardStaffGameInstance::ClearSteamLeaderboardDelegate(const IOnlineLeaderboardsPtr& LeaderboardsInterface)
{
	if (LeaderboardsInterface.IsValid() && SteamLeaderboardFlushCompleteDelegateHandle.IsValid())
	{
		LeaderboardsInterface->ClearOnLeaderboardFlushCompleteDelegate_Handle(SteamLeaderboardFlushCompleteDelegateHandle);
	}
	SteamLeaderboardFlushCompleteDelegateHandle.Reset();
}

void UWizardStaffGameInstance::ResetSteamMatchSubmissionTracking()
{
	SubmittedSteamMatchGenerations.Reset();
	PendingSteamLeaderboardMatchGeneration = INDEX_NONE;
}
