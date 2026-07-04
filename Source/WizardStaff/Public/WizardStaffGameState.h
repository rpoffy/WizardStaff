#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffGameState.generated.h"

UENUM(BlueprintType)
enum class EWizardReplicatedGameplayEventType : uint8
{
	None UMETA(DisplayName = "None"),
	MugPickup UMETA(DisplayName = "Mug Pickup"),
	BrewRewardGranted UMETA(DisplayName = "Brew Reward Granted"),
	BrewRewardUsed UMETA(DisplayName = "Brew Reward Used"),
	ArcanePinballShell UMETA(DisplayName = "Arcane Pinball Shell"),
	ArcanePinballCast UMETA(DisplayName = "Arcane Pinball Cast"),
	ArcanePinballHit UMETA(DisplayName = "Arcane Pinball Hit"),
	RingOutPending UMETA(DisplayName = "Ring-Out Pending"),
	RespawnComplete UMETA(DisplayName = "Respawn Complete"),
	StaffsPowerupCollected UMETA(DisplayName = "Staffs Powerup Collected"),
	MegaStaffGranted UMETA(DisplayName = "Mega Staff Granted"),
	MegaStaffExpired UMETA(DisplayName = "Mega Staff Expired"),
	StaffClashStarted UMETA(DisplayName = "Staff Clash Started"),
	StaffClashResolved UMETA(DisplayName = "Staff Clash Resolved"),
	GrandWizardCandidateChanged UMETA(DisplayName = "Grand Wizard Candidate Changed"),
	FinalWinner UMETA(DisplayName = "Final Winner"),
	RematchStarted UMETA(DisplayName = "Rematch Started")
};

USTRUCT(BlueprintType)
struct FWizardReplicatedGameplayEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 Sequence = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	EWizardReplicatedGameplayEventType EventType = EWizardReplicatedGameplayEventType::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 PrimaryPlayerIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 SecondaryPlayerIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float NumericValue = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ServerWorldTimeSeconds = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 MatchSessionGeneration = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	FString DisplayText;
};

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AWizardStaffGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetMatchStateMirror(
		EWizardPartyMatchState NewPartyMatchState,
		EWizardTrialState NewActiveTrialState,
		EWizardTrialType NewActiveTrialType,
		EWizardPrototypeTuningPreset NewActiveTuningPreset,
		int32 NewCompletedTrialCount,
		int32 NewFinalCandidateIndex,
		int32 NewFinalWinnerIndex,
		bool bNewFinalCandidateVulnerable,
		int32 NewFinalStealingPlayerIndex,
		float NewFinalStealProgressAlpha,
		const FString& NewResultMessage);

	void SetTimerMirror(
		float NewMugRunRemainingTime,
		float NewStaffsAtDawnRemainingTime,
		float NewTrialCountdownRemainingTime,
		float NewTrialResultsRemainingTime,
		float NewIntermissionRemainingTime,
		float NewFinalRoundRemainingTime);

	void IncrementMatchSessionGeneration();
	void AddReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType EventType,
		const FString& DisplayText,
		int32 PrimaryPlayerIndex = INDEX_NONE,
		int32 SecondaryPlayerIndex = INDEX_NONE,
		float NumericValue = 0.0f);
	void ClearReplicatedGameplayEvents();

	const TArray<FWizardReplicatedGameplayEvent>& GetReplicatedGameplayEvents() const { return ReplicatedGameplayEvents; }

	void SetPrototypeSessionModeMirror(EWizardPrototypeSessionMode NewPrototypeSessionMode);

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	EWizardPrototypeSessionMode GetReplicatedPrototypeSessionMode() const { return ReplicatedPrototypeSessionMode; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	EWizardPrototypeSessionMode GetObservedPrototypeSessionMode() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetReplicatedPrototypeSessionModeText() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetObservedPrototypeSessionModeText() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	EWizardPartyMatchState GetReplicatedPartyMatchState() const { return ReplicatedPartyMatchState; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	EWizardTrialState GetReplicatedActiveTrialState() const { return ReplicatedActiveTrialState; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	EWizardTrialType GetReplicatedActiveTrialType() const { return ReplicatedActiveTrialType; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	EWizardPrototypeTuningPreset GetReplicatedActiveTuningPreset() const { return ReplicatedActiveTuningPreset; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetReplicatedCompletedTrialCount() const { return ReplicatedCompletedTrialCount; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetReplicatedFinalCandidateIndex() const { return ReplicatedFinalCandidateIndex; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetReplicatedFinalWinnerIndex() const { return ReplicatedFinalWinnerIndex; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	bool IsReplicatedGrandWizardFinalActive() const { return ReplicatedPartyMatchState == EWizardPartyMatchState::FinalRound; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	bool IsReplicatedFinalCandidateVulnerable() const { return bReplicatedFinalCandidateVulnerable; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetReplicatedFinalStealingPlayerIndex() const { return ReplicatedFinalStealingPlayerIndex; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetReplicatedFinalStealProgressAlpha() const { return ReplicatedFinalStealProgressAlpha; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetReplicatedFinalReadableSequence() const { return ReplicatedFinalReadableSequence; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetReplicatedMatchSessionGeneration() const { return ReplicatedMatchSessionGeneration; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetReplicatedResultMessage() const { return ReplicatedResultMessage; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetReplicatedGameplayEventSequence() const { return ReplicatedGameplayEventSequence; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetReplicatedMugRunRemainingTime() const { return ReplicatedMugRunRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetReplicatedStaffsAtDawnRemainingTime() const { return ReplicatedStaffsAtDawnRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetReplicatedTrialCountdownRemainingTime() const { return ReplicatedTrialCountdownRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetReplicatedTrialResultsRemainingTime() const { return ReplicatedTrialResultsRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetReplicatedIntermissionRemainingTime() const { return ReplicatedIntermissionRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetReplicatedFinalRoundRemainingTime() const { return ReplicatedFinalRoundRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	float GetActiveReplicatedTimer() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetReplicatedPartyMatchStateText() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetReplicatedActiveTrialStateText() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetReplicatedActiveTrialName() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetReplicatedTuningPresetText() const;

protected:
	UFUNCTION()
	void OnRep_ReplicatedPartyMatchState();

	UFUNCTION()
	void OnRep_ReplicatedMatchSessionGeneration();

	UFUNCTION()
	void OnRep_ReplicatedPrototypeSessionMode();

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPartyMatchState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	EWizardPartyMatchState ReplicatedPartyMatchState = EWizardPartyMatchState::PartyHall;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	EWizardTrialState ReplicatedActiveTrialState = EWizardTrialState::WaitingToStart;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	EWizardTrialType ReplicatedActiveTrialType = EWizardTrialType::MugRun;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	EWizardPrototypeTuningPreset ReplicatedActiveTuningPreset = EWizardPrototypeTuningPreset::Chaotic;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ReplicatedMugRunRemainingTime = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ReplicatedStaffsAtDawnRemainingTime = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ReplicatedTrialCountdownRemainingTime = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ReplicatedTrialResultsRemainingTime = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ReplicatedIntermissionRemainingTime = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ReplicatedFinalRoundRemainingTime = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 ReplicatedCompletedTrialCount = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 ReplicatedFinalCandidateIndex = INDEX_NONE;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 ReplicatedFinalWinnerIndex = INDEX_NONE;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	bool bReplicatedFinalCandidateVulnerable = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 ReplicatedFinalStealingPlayerIndex = INDEX_NONE;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	float ReplicatedFinalStealProgressAlpha = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 ReplicatedFinalReadableSequence = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedMatchSessionGeneration, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 ReplicatedMatchSessionGeneration = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	FString ReplicatedResultMessage;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	TArray<FWizardReplicatedGameplayEvent> ReplicatedGameplayEvents;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 ReplicatedGameplayEventSequence = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPrototypeSessionMode, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	EWizardPrototypeSessionMode ReplicatedPrototypeSessionMode = EWizardPrototypeSessionMode::LocalPrototype;
};
