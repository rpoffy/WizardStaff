#include "WizardStaffPlayerState.h"

#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "WizardStaffWizardCharacter.h"
#include "Net/UnrealNetwork.h"

AWizardStaffPlayerState::AWizardStaffPlayerState()
{
	bReplicates = true;
}

void AWizardStaffPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffPlayerState, WizardDisplaySlot);
	DOREPLIFETIME(AWizardStaffPlayerState, WizardColorIndex);
	DOREPLIFETIME(AWizardStaffPlayerState, RoundWins);
	DOREPLIFETIME(AWizardStaffPlayerState, GrandWizardFavor);
	DOREPLIFETIME(AWizardStaffPlayerState, CurrentTrialScore);
	DOREPLIFETIME(AWizardStaffPlayerState, StaffsAtDawnScore);
	DOREPLIFETIME(AWizardStaffPlayerState, bPartyHallReady);
	DOREPLIFETIME(AWizardStaffPlayerState, bPlaytestBot);
	DOREPLIFETIME(AWizardStaffPlayerState, WizardSummaryText);
}

void AWizardStaffPlayerState::SetWizardPlayerMirror(
	int32 NewDisplaySlot,
	int32 NewColorIndex,
	int32 NewRoundWins,
	int32 NewGrandWizardFavor,
	int32 NewCurrentTrialScore,
	int32 NewStaffsAtDawnScore,
	bool bNewPartyHallReady,
	bool bNewPlaytestBot,
	const FString& NewSummaryText)
{
	WizardDisplaySlot = NewDisplaySlot;
	WizardColorIndex = FMath::Max(NewColorIndex, 0);
	RoundWins = FMath::Max(NewRoundWins, 0);
	GrandWizardFavor = FMath::Max(NewGrandWizardFavor, 0);
	CurrentTrialScore = NewCurrentTrialScore;
	StaffsAtDawnScore = NewStaffsAtDawnScore;
	bPartyHallReady = bNewPartyHallReady;
	bPlaytestBot = bNewPlaytestBot;
	WizardSummaryText = NewSummaryText.Left(160);
}

void AWizardStaffPlayerState::OnRep_WizardDisplaySlot()
{
#if !UE_BUILD_SHIPPING
	if (GetNetMode() == NM_Client && WizardDisplaySlot != INDEX_NONE)
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff client PlayerState mirror: slot=P%d color=%d bot=%s."),
			WizardDisplaySlot + 1,
			WizardColorIndex,
			bPlaytestBot ? TEXT("true") : TEXT("false"));
	}
#endif

	if (AController* OwningController = Cast<AController>(GetOwner()))
	{
		if (AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(OwningController->GetPawn()))
		{
			Wizard->ApplyPlayerColor(WizardColorIndex);
			return;
		}
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AWizardStaffWizardCharacter> It(World); It; ++It)
		{
			AWizardStaffWizardCharacter* Wizard = *It;
			if (Wizard && Wizard->GetPlayerState() == this)
			{
				Wizard->ApplyPlayerColor(WizardColorIndex);
				return;
			}
		}
	}
}
