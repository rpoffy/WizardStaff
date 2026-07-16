#include "WizardStaffPlayerState.h"

#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "WizardStaffGameInstance.h"
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

bool AWizardStaffPlayerState::SetWizardPlayerMirror(
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
	if (!HasAuthority())
	{
		return false;
	}

	const int32 SafeColorIndex = FMath::Max(NewColorIndex, 0);
	const int32 SafeRoundWins = FMath::Max(NewRoundWins, 0);
	const int32 SafeGrandWizardFavor = FMath::Max(NewGrandWizardFavor, 0);
	const FString SafeSummaryText = NewSummaryText.Left(160);
	const bool bChanged = WizardDisplaySlot != NewDisplaySlot
		|| WizardColorIndex != SafeColorIndex
		|| RoundWins != SafeRoundWins
		|| GrandWizardFavor != SafeGrandWizardFavor
		|| CurrentTrialScore != NewCurrentTrialScore
		|| StaffsAtDawnScore != NewStaffsAtDawnScore
		|| bPartyHallReady != bNewPartyHallReady
		|| bPlaytestBot != bNewPlaytestBot
		|| WizardSummaryText != SafeSummaryText;
	if (!bChanged)
	{
		return false;
	}

	WizardDisplaySlot = NewDisplaySlot;
	WizardColorIndex = SafeColorIndex;
	RoundWins = SafeRoundWins;
	GrandWizardFavor = SafeGrandWizardFavor;
	CurrentTrialScore = NewCurrentTrialScore;
	StaffsAtDawnScore = NewStaffsAtDawnScore;
	bPartyHallReady = bNewPartyHallReady;
	bPlaytestBot = bNewPlaytestBot;
	WizardSummaryText = SafeSummaryText;
	return true;
}

void AWizardStaffPlayerState::SendAuthoritativeSteamMatchResultToOwner(
	int32 MatchGeneration,
	int32 PlayerSlot,
	int32 WinnerSlot,
	int32 FinalGrandWizardFavor,
	int32 FinalRoundWins)
{
	if (!HasAuthority()
		|| bPlaytestBot
		|| MatchGeneration <= 0
		|| PlayerSlot < 0
		|| PlayerSlot != WizardDisplaySlot)
	{
		return;
	}

	ClientSubmitAuthoritativeSteamMatchResult(
		MatchGeneration,
		PlayerSlot,
		WinnerSlot,
		FMath::Max(FinalGrandWizardFavor, 0),
		FMath::Max(FinalRoundWins, 0));
}

void AWizardStaffPlayerState::ClientSubmitAuthoritativeSteamMatchResult_Implementation(
	int32 MatchGeneration,
	int32 PlayerSlot,
	int32 WinnerSlot,
	int32 FinalGrandWizardFavor,
	int32 FinalRoundWins)
{
	if (bPlaytestBot || PlayerSlot < 0 || PlayerSlot != WizardDisplaySlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam result delivery rejected on client: deliveredSlot=%d mirroredSlot=%d bot=%s."),
			PlayerSlot,
			WizardDisplaySlot,
			bPlaytestBot ? TEXT("true") : TEXT("false"));
		return;
	}

	UWorld* World = GetWorld();
	UWizardStaffGameInstance* WizardGameInstance = World ? Cast<UWizardStaffGameInstance>(World->GetGameInstance()) : nullptr;
	if (!WizardGameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam result delivery could not find WizardStaffGameInstance for P%d."), PlayerSlot + 1);
		return;
	}

	WizardGameInstance->SubmitAuthoritativeSteamMatchResult(
		MatchGeneration,
		PlayerSlot,
		WinnerSlot,
		FinalGrandWizardFavor,
		FinalRoundWins);
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
