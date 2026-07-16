#include "WizardStaffGameState.h"

#include "Net/UnrealNetwork.h"

namespace
{
constexpr int32 MaxReplicatedGameplayEvents = 8;
constexpr int32 MaxReplicatedGameplayEventTextLength = 96;
constexpr float ReplicatedTimerMinimumDelta = 0.1f;
constexpr float ReplicatedProgressMinimumDelta = 0.01f;

template <typename TValue>
void AssignMirrorIfChanged(TValue& CurrentValue, const TValue& NewValue)
{
	if (CurrentValue != NewValue)
	{
		CurrentValue = NewValue;
	}
}

void AssignFloatMirrorIfMeaningfullyChanged(float& CurrentValue, float NewValue, float MinimumDelta)
{
	const float SafeNewValue = FMath::IsFinite(NewValue) ? NewValue : 0.0f;
	const bool bEndpointChanged = (CurrentValue == 0.0f) != (SafeNewValue == 0.0f);
	if (bEndpointChanged || FMath::Abs(CurrentValue - SafeNewValue) >= MinimumDelta)
	{
		CurrentValue = SafeNewValue;
	}
}
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
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedCauldronRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedCauldronCursedPlayerIndex);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedCauldronCurseRemainingTime);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedCauldronBankingPlayerIndex);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedCauldronBankingTransferredCount);
	DOREPLIFETIME(AWizardStaffGameState, ReplicatedCauldronBankingProgressAlpha);
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
	if (!HasAuthority())
	{
		return;
	}

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

	AssignMirrorIfChanged(ReplicatedPartyMatchState, NewPartyMatchState);
	AssignMirrorIfChanged(ReplicatedActiveTrialState, NewActiveTrialState);
	AssignMirrorIfChanged(ReplicatedActiveTrialType, NewActiveTrialType);
	AssignMirrorIfChanged(ReplicatedActiveTuningPreset, NewActiveTuningPreset);
	AssignMirrorIfChanged(ReplicatedCompletedTrialCount, NewCompletedTrialCount);
	AssignMirrorIfChanged(ReplicatedFinalCandidateIndex, NewFinalCandidateIndex);
	AssignMirrorIfChanged(ReplicatedFinalWinnerIndex, NewFinalWinnerIndex);
	AssignMirrorIfChanged(bReplicatedFinalCandidateVulnerable, bSafeFinalCandidateVulnerable);
	AssignMirrorIfChanged(ReplicatedFinalStealingPlayerIndex, SafeFinalStealingPlayerIndex);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedFinalStealProgressAlpha, SafeFinalStealProgressAlpha, ReplicatedProgressMinimumDelta);
	AssignMirrorIfChanged(ReplicatedResultMessage, NewResultMessage);
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
	if (!HasAuthority())
	{
		return;
	}

	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedMugRunRemainingTime, FMath::Max(NewMugRunRemainingTime, 0.0f), ReplicatedTimerMinimumDelta);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedStaffsAtDawnRemainingTime, FMath::Max(NewStaffsAtDawnRemainingTime, 0.0f), ReplicatedTimerMinimumDelta);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedTrialCountdownRemainingTime, FMath::Max(NewTrialCountdownRemainingTime, 0.0f), ReplicatedTimerMinimumDelta);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedTrialResultsRemainingTime, FMath::Max(NewTrialResultsRemainingTime, 0.0f), ReplicatedTimerMinimumDelta);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedIntermissionRemainingTime, FMath::Max(NewIntermissionRemainingTime, 0.0f), ReplicatedTimerMinimumDelta);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedFinalRoundRemainingTime, FMath::Max(NewFinalRoundRemainingTime, 0.0f), ReplicatedTimerMinimumDelta);
}

void AWizardStaffGameState::SetCauldronMirror(float NewRemainingTime, int32 NewCursedPlayerIndex, float NewCurseRemainingTime, int32 NewBankingPlayerIndex, int32 NewBankingTransferredCount, float NewBankingProgressAlpha)
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bCauldronActive = ReplicatedPartyMatchState == EWizardPartyMatchState::Trial
		&& ReplicatedActiveTrialState == EWizardTrialState::Active
		&& ReplicatedActiveTrialType == EWizardTrialType::CauldronCatastrophe;
	const float SafeRemainingTime = bCauldronActive ? FMath::Max(NewRemainingTime, 0.0f) : 0.0f;
	const int32 SafeCursedPlayerIndex = bCauldronActive ? NewCursedPlayerIndex : INDEX_NONE;
	const float SafeCurseRemainingTime = SafeCursedPlayerIndex == INDEX_NONE
		? 0.0f
		: FMath::Max(NewCurseRemainingTime, 0.0f);
	const int32 SafeBankingPlayerIndex = bCauldronActive ? NewBankingPlayerIndex : INDEX_NONE;
	const int32 SafeBankingTransferredCount = SafeBankingPlayerIndex == INDEX_NONE ? 0 : FMath::Max(NewBankingTransferredCount, 0);
	const float SafeBankingProgressAlpha = SafeBankingPlayerIndex == INDEX_NONE ? 0.0f : FMath::Clamp(NewBankingProgressAlpha, 0.0f, 1.0f);

	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedCauldronRemainingTime, SafeRemainingTime, ReplicatedTimerMinimumDelta);
	AssignMirrorIfChanged(ReplicatedCauldronCursedPlayerIndex, SafeCursedPlayerIndex);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedCauldronCurseRemainingTime, SafeCurseRemainingTime, ReplicatedTimerMinimumDelta);
	AssignMirrorIfChanged(ReplicatedCauldronBankingPlayerIndex, SafeBankingPlayerIndex);
	AssignMirrorIfChanged(ReplicatedCauldronBankingTransferredCount, SafeBankingTransferredCount);
	AssignFloatMirrorIfMeaningfullyChanged(ReplicatedCauldronBankingProgressAlpha, SafeBankingProgressAlpha, ReplicatedProgressMinimumDelta);
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

	if (ReplicatedGameplayEvents.IsEmpty())
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

	AssignMirrorIfChanged(ReplicatedPrototypeSessionMode, NewPrototypeSessionMode);
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

	switch (ReplicatedActiveTrialType)
	{
	case EWizardTrialType::StaffsAtDawn:
		return ReplicatedStaffsAtDawnRemainingTime;
	case EWizardTrialType::CauldronCatastrophe:
		return ReplicatedCauldronRemainingTime;
	case EWizardTrialType::MugRun:
	default:
		return ReplicatedMugRunRemainingTime;
	}
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
	switch (ReplicatedActiveTrialType)
	{
	case EWizardTrialType::StaffsAtDawn:
		return TEXT("Staffs at Dawn");
	case EWizardTrialType::CauldronCatastrophe:
		return TEXT("Cauldron Catastrophe");
	case EWizardTrialType::MugRun:
	default:
		return TEXT("Mug Run");
	}
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
