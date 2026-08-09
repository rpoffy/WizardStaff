#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "WizardStaffGameInstance.generated.h"

class IOnlineSubsystem;
class AWizardStaffPlayerState;
class UNetDriver;

UENUM(BlueprintType)
enum class EWizardStaffFrontendState : uint8
{
	Idle,
	StartingLocal,
	CreatingHostSession,
	HostTraveling,
	SearchingSessions,
	JoiningSession,
	ClientTraveling,
	Error
};

UCLASS()
class WIZARDSTAFF_API UWizardStaffGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(Exec)
	void WizardSteamHost();

	UFUNCTION(Exec)
	void DebugSteamHostSession();

	UFUNCTION(Exec)
	void WizardSteamJoinFirstSession();

	UFUNCTION(Exec)
	void DebugSteamFindAndJoinSession();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Frontend")
	void StartLocalPrototypeFromMenu();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Frontend")
	void StartSteamHostFromMenu();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Frontend")
	void StartSteamJoinFromMenu();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Frontend")
	void CancelFrontendRequest();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Frontend")
	void ReturnToMainMenu();

	void SetSteamSessionJoinability(bool bJoinable, const TCHAR* Context);

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Frontend")
	EWizardStaffFrontendState GetFrontendState() const { return FrontendState; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Frontend")
	FText GetFrontendStatusText() const { return FrontendStatusText; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Frontend")
	bool IsFrontendRequestBusy() const;

private:
	friend class AWizardStaffPlayerState;

	void SubmitServerDeliveredSteamMatchResult(
		const AWizardStaffPlayerState* DeliveryPlayerState,
		int32 MatchGeneration,
		int32 PlayerSlot,
		int32 WinnerSlot,
		int32 FinalGrandWizardFavor,
		int32 FinalRoundWins);

	void StartSteamHostRequest(bool bFromFrontend);
	void StartSteamJoinRequest(bool bFromFrontend);
	int32 BeginFrontendRequest(EWizardStaffFrontendState NewState, const FText& StatusText);
	bool IsCurrentFrontendRequest(int32 RequestGeneration) const;
	void SetFrontendState(EWizardStaffFrontendState NewState, const FText& StatusText);
	void FailFrontendRequest(int32 RequestGeneration, const FText& StatusText, const TCHAR* LogContext);
	void ArmFrontendRequestTimeout(int32 RequestGeneration, EWizardStaffFrontendState ExpectedState, float TimeoutSeconds);
	void ClearFrontendRequestTimeout();
	void OnFrontendRequestTimeout(int32 RequestGeneration, EWizardStaffFrontendState ExpectedState);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void OnTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);
	void PrepareFrontendForGameplayTravel();
	void RemoveExtraLocalPlayersForOnlineTravel(const TCHAR* Context);
	void ClearSteamFindAndJoinDelegates(const IOnlineSessionPtr& SessionInterface);
	void ReturnToMainMenuInternal(const FText& StatusText);
	void ReturnToMainMenuAfterGameplayNetworkFailure();
	void OpenMainMenuAfterSessionTeardown();

	IOnlineSubsystem* GetSteamSubsystem(bool bLoadOnDemand = true) const;
	IOnlineSessionPtr GetSteamSessionInterface(bool bLoadOnDemand = true) const;
	IOnlineLeaderboardsPtr GetSteamLeaderboardsInterface(bool bLoadOnDemand = true) const;

	void CreateSteamSmokeSession(int32 RequestGeneration = INDEX_NONE);
	void FindSteamSmokeSessions(int32 RequestGeneration = INDEX_NONE);
	void OnDestroySteamSmokeSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnCreateSteamSmokeSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSteamSmokeSessionsComplete(bool bWasSuccessful);
	void OnJoinSteamSmokeSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSteamLeaderboardFlushComplete(FName SessionName, bool bWasSuccessful);

	void ClearSteamSessionDelegates(const IOnlineSessionPtr& SessionInterface);
	void ClearSteamLeaderboardDelegate(const IOnlineLeaderboardsPtr& LeaderboardsInterface);
	void ResetSteamMatchSubmissionTracking();
	void LogSteamUnavailableHint(const TCHAR* CommandName) const;

	TSharedPtr<FOnlineSessionSearch> SteamSessionSearch;
	FDelegateHandle DestroySteamSessionCompleteDelegateHandle;
	FDelegateHandle CreateSteamSessionCompleteDelegateHandle;
	FDelegateHandle FindSteamSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSteamSessionCompleteDelegateHandle;
	FDelegateHandle SteamLeaderboardFlushCompleteDelegateHandle;
	FDelegateHandle NetworkFailureDelegateHandle;
	FDelegateHandle TravelFailureDelegateHandle;
	FTimerHandle FrontendRequestTimeoutTimerHandle;
	TSet<int32> SubmittedSteamMatchGenerations;
	int32 PendingSteamLeaderboardMatchGeneration = INDEX_NONE;
	bool bCreateSteamSessionAfterDestroy = false;
	bool bFindSteamSessionAfterDestroy = false;
	bool bReturnToMainMenuAfterSessionDestroy = false;
	bool bReturningToMenuAfterGameplayNetworkFailure = false;
	int32 PendingSteamHostRequestGeneration = INDEX_NONE;
	int32 PendingSteamCreateRequestGeneration = INDEX_NONE;
	int32 PendingSteamFindRequestGeneration = INDEX_NONE;
	int32 PendingSteamJoinRequestGeneration = INDEX_NONE;
	int32 FrontendRequestGeneration = 0;
	EWizardStaffFrontendState FrontendState = EWizardStaffFrontendState::Idle;
	FText FrontendStatusText;
	FText GameplayNetworkFailureStatusText;
};
