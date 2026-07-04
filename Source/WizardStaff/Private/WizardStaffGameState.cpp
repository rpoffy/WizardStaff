#include "WizardStaffGameState.h"

#include "Net/UnrealNetwork.h"

namespace
{
constexpr int32 MaxReplicatedGameplayEvents = 8;
constexpr int32 MaxReplicatedGameplayEventTextLength = 96;
}

AWizardStaffGameState::AWizardStaffGameState()
{
	bReplicates = true;
}

void AWizardStaffGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffGameState, ReplicatedPartyMatchState);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedActiveTrialState);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedActiveTrialType);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedActiveTuningPreset);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedMugRunRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedStaffsAtDawnRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedTrialCountdownRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedTrialResultsRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedIntermissionRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedFinalRoundRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedCompletedTrialCount);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedFinalCandidateIndex);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedFinalWinnerIndex);
	DOREPLIFETIME(AWizardStaffGameState, bReplicatedFinalCandidateVulnerable);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedFinalStealingPlayerIndex);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedFinalStealProgressAlpha);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedFinalReadableSequence);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedMatchSessionGeneration);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedResultMessage);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedGameplayEvents);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedGameplayEventSequence);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedPrototypeSessionMode);
}

void AWizardStaffGameState::SetMatchStateMirror(
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
	const FString& NewResultMessage)
{
	const bool bFinalActive = NewPartyMatchState == EWizardPartyMatchState::FinalRound
		&& NewActiveTrialState == EWizardTrialState::Active
		&& NewFinalWinnerIndex == INDEX_NONE;
	const bool bSafeFinalCandidateVulnerable = NewPartyMatchState == EWizardPartyMatchState::FinalRound
		&& NewActiveTrialState == EWizardTrialState::Active
		&& NewFinalWinnerIndex == INDEX_NONE
		&& bNewFinalCandidateVulnerable;
	const int32 SafeFinalStealingPlayerIndex = bFinalActive ? NewFinalStealingPlayerIndex : INDEX_NONE;
	const float SafeFinalStealProgressAlpha = SafeFinalStealingPlayerIndex == INDEX_NONE
		? 0.0f
		: FMath::Clamp(NewFinalStealProgressAlpha, 0.0f, 1.0f);
	const int32 PreviousStealProgressBucket = FMath::FloorToInt(FMath::Clamp(ReplicatedFinalStealProgressAlpha, 0.0f, 1.0f) * 4.0f);
	const int32 NewStealProgressBucket = FMath::FloorToInt(SafeFinalStealProgressAlpha * 4.0f);
	const bool bFinalReadableChanged =
		ReplicatedPartyMatchState != NewPartyMatchState
		|| ReplicatedActiveTrialState != NewActiveTrialState
		|| ReplicatedFinalCandidateIndex != NewFinalCandidateIndex
		|| ReplicatedFinalWinnerIndex != NewFinalWinnerIndex
		|| bReplicatedFinalCandidateVulnerable != bSafeFinalCandidateVulnerable
		|| ReplicatedFinalStealingPlayerIndex != SafeFinalStealingPlayerIndex
		|| PreviousStealProgressBucket != NewStealProgressBucket
		|| ReplicatedResultMessage != NewResultMessage;

	ReplicatedPartyMatchState = NewPartyMatchState;
	ReplicatedActiveTrialState = NewActiveTrialState;
	ReplicatedActiveTrialType = NewActiveTrialType;
	ReplicatedActiveTuningPreset = NewActiveTuningPreset;
	ReplicatedCompletedTrialCount = NewCompletedTrialCount;
	ReplicatedFinalCandidateIndex = NewFinalCandidateIndex;
	ReplicatedFinalWinnerIndex = NewFinalWinnerIndex;
	bReplicatedFinalCandidateVulnerable = bSafeFinalCandidateVulnerable;
	ReplicatedFinalStealingPlayerIndex = SafeFinalStealingPlayerIndex;
	ReplicatedFinalStealProgressAlpha = SafeFinalStealProgressAlpha;
	ReplicatedResultMessage = NewResultMessage;
	if (bFinalReadableChanged)
	{
		++ReplicatedFinalReadableSequence;
	}
}

void AWizardStaffGameState::SetTimerMirror(
	float NewMugRunRemainingTime,
	float NewStaffsAtDawnRemainingTime,
	float NewTrialCountdownRemainingTime,
	float NewTrialResultsRemainingTime,
	float NewIntermissionRemainingTime,
	float NewFinalRoundRemainingTime)
{
	ReplicatedMugRunRemainingTime = NewMugRunRemainingTime;
	ReplicatedStaffsAtDawnRemainingTime = NewStaffsAtDawnRemainingTime;
	ReplicatedTrialCountdownRemainingTime = NewTrialCountdownRemainingTime;
	ReplicatedTrialResultsRemainingTime = NewTrialResultsRemainingTime;
	ReplicatedIntermissionRemainingTime = NewIntermissionRemainingTime;
	ReplicatedFinalRoundRemainingTime = NewFinalRoundRemainingTime;
}

void AWizardStaffGameState::IncrementMatchSessionGeneration()
{
	++ReplicatedMatchSessionGeneration;
	ClearReplicatedGameplayEvents();
}

void AWizardStaffGameState::AddReplicatedGameplayEvent(
	EWizardReplicatedGameplayEventType EventType,
	const FString& DisplayText,
	int32 PrimaryPlayerIndex,
	int32 SecondaryPlayerIndex,
	float NumericValue)
{
	if (!HasAuthority() || EventType == EWizardReplicatedGameplayEventType::None || DisplayText.IsEmpty())
	{
		return;
	}

	FWizardReplicatedGameplayEvent NewEvent;
	NewEvent.Sequence = ++ReplicatedGameplayEventSequence;
	NewEvent.EventType = EventType;
	NewEvent.PrimaryPlayerIndex = PrimaryPlayerIndex;
	NewEvent.SecondaryPlayerIndex = SecondaryPlayerIndex;
	NewEvent.NumericValue = NumericValue;
	NewEvent.ServerWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	NewEvent.MatchSessionGeneration = ReplicatedMatchSessionGeneration;
	NewEvent.DisplayText = DisplayText.Left(MaxReplicatedGameplayEventTextLength);

	ReplicatedGameplayEvents.Add(NewEvent);
	while (ReplicatedGameplayEvents.Num() > MaxReplicatedGameplayEvents)
	{
		ReplicatedGameplayEvents.RemoveAt(0);
	}

	ForceNetUpdate();
}

void AWizardStaffGameState::ClearReplicatedGameplayEvents()
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedGameplayEvents.Reset();
	++ReplicatedGameplayEventSequence;
	ForceNetUpdate();
}

void AWizardStaffGameState::SetPrototypeSessionModeMirror(EWizardPrototypeSessionMode NewPrototypeSessionMode)
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedPrototypeSessionMode = NewPrototypeSessionMode;
}

EWizardPrototypeSessionMode AWizardStaffGameState::GetObservedPrototypeSessionMode() const
{
	return GetNetMode() == NM_Client
		? EWizardPrototypeSessionMode::OnlineClient
		: ReplicatedPrototypeSessionMode;
}

FString AWizardStaffGameState::GetReplicatedPrototypeSessionModeText() const
{
	switch (ReplicatedPrototypeSessionMode)
	{
	case EWizardPrototypeSessionMode::LocalPrototype:
		return TEXT("Local Prototype");
	case EWizardPrototypeSessionMode::LocalWithBots:
		return TEXT("Local With Bots");
	case EWizardPrototypeSessionMode::OnlineListenServer:
		return TEXT("Online Listen Server");
	case EWizardPrototypeSessionMode::OnlineClient:
		return TEXT("Online Client");
	default:
		return TEXT("Unknown");
	}
}

FString AWizardStaffGameState::GetObservedPrototypeSessionModeText() const
{
	switch (GetObservedPrototypeSessionMode())
	{
	case EWizardPrototypeSessionMode::LocalPrototype:
		return TEXT("Local Prototype");
	case EWizardPrototypeSessionMode::LocalWithBots:
		return TEXT("Local With Bots");
	case EWizardPrototypeSessionMode::OnlineListenServer:
		return TEXT("Online Listen Server");
	case EWizardPrototypeSessionMode::OnlineClient:
		return TEXT("Online Client");
	default:
		return TEXT("Unknown");
	}
}

void AWizardStaffGameState::OnRep_ReplicatedPartyMatchState()
{
#if !UE_BUILD_SHIPPING
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff client GameState mirror: party=%s trial=%s timer=%.1f."),
			*GetReplicatedPartyMatchStateText(),
			*GetReplicatedActiveTrialName(),
			GetActiveReplicatedTimer());
	}
#endif
}

void AWizardStaffGameState::OnRep_ReplicatedMatchSessionGeneration()
{
#if !UE_BUILD_SHIPPING
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff client GameState match generation: %d."), ReplicatedMatchSessionGeneration);
	}
#endif
}

void AWizardStaffGameState::OnRep_ReplicatedPrototypeSessionMode()
{
#if !UE_BUILD_SHIPPING
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff client GameState session mode: authority=%s observed=%s."),
			*GetReplicatedPrototypeSessionModeText(),
			*GetObservedPrototypeSessionModeText());
	}
#endif
}

float AWizardStaffGameState::GetActiveReplicatedTimer() const
{
	if (ReplicatedPartyMatchState == EWizardPartyMatchState::FinalRound)
	{
		return ReplicatedActiveTrialState == EWizardTrialState::Results
			? ReplicatedTrialResultsRemainingTime
			: ReplicatedFinalRoundRemainingTime;
	}

	if ((ReplicatedPartyMatchState == EWizardPartyMatchState::PartyHall || ReplicatedPartyMatchState == EWizardPartyMatchState::Intermission)
		&& ReplicatedActiveTrialState == EWizardTrialState::WaitingToStart)
	{
		return ReplicatedIntermissionRemainingTime;
	}

	if (ReplicatedActiveTrialState == EWizardTrialState::Countdown)
	{
		return ReplicatedTrialCountdownRemainingTime;
	}

	if (ReplicatedActiveTrialState == EWizardTrialState::Results)
	{
		return ReplicatedTrialResultsRemainingTime;
	}

	return ReplicatedActiveTrialType == EWizardTrialType::StaffsAtDawn
		? ReplicatedStaffsAtDawnRemainingTime
		: ReplicatedMugRunRemainingTime;
}

FString AWizardStaffGameState::GetReplicatedPartyMatchStateText() const
{
	switch (ReplicatedPartyMatchState)
	{
	case EWizardPartyMatchState::PartyHall:
		return TEXT("Party Hall");
	case EWizardPartyMatchState::Intermission:
		return TEXT("Intermission");
	case EWizardPartyMatchState::Trial:
		return TEXT("Trial");
	case EWizardPartyMatchState::Results:
		return TEXT("Results");
	case EWizardPartyMatchState::FinalRound:
		return TEXT("Grand Wizard Final");
	default:
		return TEXT("Unknown");
	}
}

FString AWizardStaffGameState::GetReplicatedActiveTrialStateText() const
{
	switch (ReplicatedActiveTrialState)
	{
	case EWizardTrialState::WaitingToStart:
		return TEXT("Waiting");
	case EWizardTrialState::Countdown:
		return TEXT("Countdown");
	case EWizardTrialState::Active:
		return TEXT("Active");
	case EWizardTrialState::Results:
		return TEXT("Results");
	case EWizardTrialState::Finished:
		return TEXT("Finished");
	default:
		return TEXT("Unknown");
	}
}

FString AWizardStaffGameState::GetReplicatedActiveTrialName() const
{
	return ReplicatedActiveTrialType == EWizardTrialType::StaffsAtDawn
		? TEXT("Staffs at Dawn")
		: TEXT("Mug Run");
}

FString AWizardStaffGameState::GetReplicatedTuningPresetText() const
{
	switch (ReplicatedActiveTuningPreset)
	{
	case EWizardPrototypeTuningPreset::Stable:
		return TEXT("Stable");
	case EWizardPrototypeTuningPreset::Absurd:
		return TEXT("Arcane Catastrophe");
	case EWizardPrototypeTuningPreset::Chaotic:
	default:
		return TEXT("Chaotic");
	}
}
