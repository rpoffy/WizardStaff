#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "WizardStaffGameInstance.generated.h"

UCLASS()
class WIZARDSTAFF_API UWizardStaffGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Shutdown() override;

	UFUNCTION(Exec)
	void WizardSteamHost();

	UFUNCTION(Exec)
	void DebugSteamHostSession();

	UFUNCTION(Exec)
	void WizardSteamJoinFirstSession();

	UFUNCTION(Exec)
	void DebugSteamFindAndJoinSession();

	void SubmitAuthoritativeSteamMatchResult(
		int32 MatchGeneration,
		int32 PlayerSlot,
		int32 WinnerSlot,
		int32 FinalGrandWizardFavor,
		int32 FinalRoundWins);

private:
	IOnlineSessionPtr GetSteamSessionInterface() const;
	IOnlineLeaderboardsPtr GetSteamLeaderboardsInterface() const;

	void CreateSteamSmokeSession();
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
	TSet<int32> SubmittedSteamMatchGenerations;
	int32 PendingSteamLeaderboardMatchGeneration = INDEX_NONE;
	bool bCreateSteamSessionAfterDestroy = false;
};
