#include "WizardStaffGameInstance.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

namespace
{
const FName WizardSteamSubsystemName(TEXT("Steam"));
const FName WizardSteamSmokeSessionName = NAME_GameSession;
const FName WizardSteamMapSettingKey(TEXT("WIZARDSTAFF_MAP"));
const FName WizardSteamBuildSettingKey(TEXT("WIZARDSTAFF_BUILD"));
const FString WizardSteamPrototypeMapPath(TEXT("/Game/Maps/WizardStaff_Prototype"));
const FString WizardSteamSmokeBuildValue(TEXT("RealAppSteamSmoke1"));
const FString WizardSteamFavorLeaderboardName(TEXT("WizardStaff_BestGrandWizardFavor"));
const FString WizardSteamFavorRatedStatName(TEXT("GrandWizardFavor"));
constexpr int32 WizardSteamSmokeMaxPlayers = 2;

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

void UWizardStaffGameInstance::Shutdown()
{
	ClearSteamLeaderboardDelegate(GetSteamLeaderboardsInterface());
	ClearSteamSessionDelegates(GetSteamSessionInterface());
	SteamSessionSearch.Reset();
	ResetSteamMatchSubmissionTracking();

	Super::Shutdown();
}

void UWizardStaffGameInstance::WizardSteamHost()
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost is disabled in shipping builds."));
	return;
#else
	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost ignored on a remote client."));
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost failed: Steam session interface is unavailable. Is Steam running, and is OnlineSubsystemSteam enabled?"));
		LogSteamUnavailableHint(TEXT("WizardSteamHost"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamHost using OnlineSubsystem '%s' with configured Wizard Staff AppID 4954290."), *WizardSteamSubsystemName.ToString());

	ClearSteamSessionDelegates(SessionInterface);
	if (SessionInterface->GetNamedSession(WizardSteamSmokeSessionName))
	{
		bCreateSteamSessionAfterDestroy = true;
		DestroySteamSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnDestroySteamSmokeSessionComplete));

		UE_LOG(LogTemp, Log, TEXT("WizardSteamHost destroying existing smoke session before recreate."));
		if (!SessionInterface->DestroySession(WizardSteamSmokeSessionName))
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySteamSessionCompleteDelegateHandle);
			DestroySteamSessionCompleteDelegateHandle.Reset();
			bCreateSteamSessionAfterDestroy = false;
			UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost could not destroy existing session; trying create path anyway."));
			CreateSteamSmokeSession();
		}
		return;
	}

	CreateSteamSmokeSession();
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
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession failed: Steam session interface is unavailable. Is Steam running, and is OnlineSubsystemSteam enabled?"));
		LogSteamUnavailableHint(TEXT("WizardSteamJoinFirstSession"));
		return;
	}

	ClearSteamSessionDelegates(SessionInterface);
	SteamSessionSearch = MakeShared<FOnlineSessionSearch>();
	SteamSessionSearch->bIsLanQuery = false;
	SteamSessionSearch->MaxSearchResults = 10;
	SteamSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SteamSessionSearch->QuerySettings.Set(WizardSteamMapSettingKey, WizardSteamPrototypeMapPath, EOnlineComparisonOp::Equals);
	SteamSessionSearch->QuerySettings.Set(WizardSteamBuildSettingKey, WizardSteamSmokeBuildValue, EOnlineComparisonOp::Equals);

	FindSteamSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnFindSteamSmokeSessionsComplete));

	UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession searching for temporary Steam lobby smoke sessions: map=%s build=%s."),
		*WizardSteamPrototypeMapPath,
		*WizardSteamSmokeBuildValue);
	if (!SessionInterface->FindSessions(0, SteamSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSteamSessionsCompleteDelegateHandle);
		FindSteamSessionsCompleteDelegateHandle.Reset();
		SteamSessionSearch.Reset();
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession failed to start session search."));
	}
#endif
}

void UWizardStaffGameInstance::DebugSteamFindAndJoinSession()
{
	WizardSteamJoinFirstSession();
}

IOnlineSessionPtr UWizardStaffGameInstance::GetSteamSessionInterface() const
{
	IOnlineSubsystem* SteamSubsystem = IOnlineSubsystem::Get(WizardSteamSubsystemName);
	if (!SteamSubsystem)
	{
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardStaff Steam smoke subsystem: %s."), *SteamSubsystem->GetSubsystemName().ToString());
	return SteamSubsystem->GetSessionInterface();
}

IOnlineLeaderboardsPtr UWizardStaffGameInstance::GetSteamLeaderboardsInterface() const
{
	IOnlineSubsystem* SteamSubsystem = IOnlineSubsystem::Get(WizardSteamSubsystemName);
	return SteamSubsystem ? SteamSubsystem->GetLeaderboardsInterface() : nullptr;
}

void UWizardStaffGameInstance::SubmitAuthoritativeSteamMatchResult(
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

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	const FUniqueNetIdPtr LocalUserId = PlayerState ? PlayerState->GetUniqueId().GetUniqueNetId() : nullptr;
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
	UE_LOG(LogTemp, Log, TEXT("WizardStaff Steam leaderboard queued authoritative result: generation=%d P%d favor=%d roundWins=%d finalWinner=%s leaderboard=%s."),
		MatchGeneration,
		PlayerSlot + 1,
		SafeFavor,
		SafeRoundWins,
		bFinalWinner ? TEXT("true") : TEXT("false"),
		*WizardSteamFavorLeaderboardName);
}

void UWizardStaffGameInstance::CreateSteamSmokeSession()
{
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost failed: Steam session interface disappeared before create."));
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.NumPublicConnections = WizardSteamSmokeMaxPlayers;
	SessionSettings.BuildUniqueId = 1;
	SessionSettings.Set(WizardSteamMapSettingKey, WizardSteamPrototypeMapPath, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(WizardSteamBuildSettingKey, WizardSteamSmokeBuildValue, EOnlineDataAdvertisementType::ViaOnlineService);

	CreateSteamSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnCreateSteamSmokeSessionComplete));

	UE_LOG(LogTemp, Log, TEXT("WizardSteamHost creating smoke session: map=%s maxPlayers=%d build=%s."),
		*WizardSteamPrototypeMapPath,
		WizardSteamSmokeMaxPlayers,
		*WizardSteamSmokeBuildValue);

	if (!SessionInterface->CreateSession(0, WizardSteamSmokeSessionName, SessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSteamSessionCompleteDelegateHandle);
		CreateSteamSessionCompleteDelegateHandle.Reset();
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost failed to start session creation."));
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
	bCreateSteamSessionAfterDestroy = false;

	UE_LOG(LogTemp, Log, TEXT("WizardSteamHost destroy existing session complete: session=%s success=%s."),
		*SessionName.ToString(),
		bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (bShouldCreate)
	{
		CreateSteamSmokeSession();
	}
}

void UWizardStaffGameInstance::OnCreateSteamSmokeSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSteamSessionCompleteDelegateHandle);
	}
	CreateSteamSessionCompleteDelegateHandle.Reset();

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost session creation failed: session=%s."), *SessionName.ToString());
		return;
	}

	ResetSteamMatchSubmissionTracking();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamHost created session but could not open listen map because World is null."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamHost created session '%s'; opening %s?listen."),
		*SessionName.ToString(),
		*WizardSteamPrototypeMapPath);
	UGameplayStatics::OpenLevel(World, FName(*WizardSteamPrototypeMapPath), true, TEXT("listen"));
}

void UWizardStaffGameInstance::OnFindSteamSmokeSessionsComplete(bool bWasSuccessful)
{
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

	if (!bWasSuccessful || !SessionInterface.IsValid() || !SteamSessionSearch.IsValid() || ResultCount <= 0)
	{
		if (bWasSuccessful && ResultCount <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession found zero Steam smoke sessions. Use two machines/two Steam accounts if same-machine Steam API initialization blocks the client."));
		}
		SteamSessionSearch.Reset();
		return;
	}

	int32 SelectedResultIndex = INDEX_NONE;
	for (int32 ResultIndex = 0; ResultIndex < SteamSessionSearch->SearchResults.Num(); ++ResultIndex)
	{
		const FOnlineSessionSearchResult& SearchResult = SteamSessionSearch->SearchResults[ResultIndex];
		FString ResultMap;
		FString ResultBuild;
		SearchResult.Session.SessionSettings.Get(WizardSteamMapSettingKey, ResultMap);
		SearchResult.Session.SessionSettings.Get(WizardSteamBuildSettingKey, ResultBuild);

		UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession result %d: map=%s build=%s open=%d/%d ping=%d."),
			ResultIndex,
			ResultMap.IsEmpty() ? TEXT("<unset>") : *ResultMap,
			ResultBuild.IsEmpty() ? TEXT("<unset>") : *ResultBuild,
			SearchResult.Session.NumOpenPublicConnections,
			SearchResult.Session.SessionSettings.NumPublicConnections,
			SearchResult.PingInMs);

		if (ResultMap == WizardSteamPrototypeMapPath && ResultBuild == WizardSteamSmokeBuildValue)
		{
			SelectedResultIndex = ResultIndex;
			break;
		}
	}

	if (SelectedResultIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession found no compatible Wizard's Staff smoke session. Refusing to join unmatched Steam results."));
		SteamSessionSearch.Reset();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession selected compatible result %d for join."), SelectedResultIndex);

	JoinSteamSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UWizardStaffGameInstance::OnJoinSteamSmokeSessionComplete));

	if (!SessionInterface->JoinSession(0, WizardSteamSmokeSessionName, SteamSessionSearch->SearchResults[SelectedResultIndex]))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSteamSessionCompleteDelegateHandle);
		JoinSteamSessionCompleteDelegateHandle.Reset();
		SteamSessionSearch.Reset();
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession failed to start join for result %d."), SelectedResultIndex);
	}
}

void UWizardStaffGameInstance::OnJoinSteamSmokeSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr SessionInterface = GetSteamSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSteamSessionCompleteDelegateHandle);
	}
	JoinSteamSessionCompleteDelegateHandle.Reset();
	SteamSessionSearch.Reset();

	if (!SessionInterface.IsValid() || Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession join failed: session=%s result=%s (%d)."),
			*SessionName.ToString(),
			SteamJoinResultToText(Result),
			static_cast<int32>(Result));
		return;
	}

	ResetSteamMatchSubmissionTracking();

	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectString) || ConnectString.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession joined session but could not resolve a connect string."));
		return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardSteamJoinFirstSession resolved connect string but found no local PlayerController."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardSteamJoinFirstSession join succeeded; client traveling to %s."), *ConnectString);
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
