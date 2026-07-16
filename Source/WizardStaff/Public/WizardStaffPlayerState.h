#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WizardStaffPlayerState.generated.h"

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AWizardStaffPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool SetWizardPlayerMirror(
		int32 NewDisplaySlot,
		int32 NewColorIndex,
		int32 NewRoundWins,
		int32 NewGrandWizardFavor,
		int32 NewCurrentTrialScore,
		int32 NewStaffsAtDawnScore,
		bool bNewPartyHallReady,
		bool bNewPlaytestBot,
		const FString& NewSummaryText);

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetWizardDisplaySlot() const { return WizardDisplaySlot; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetWizardColorIndex() const { return WizardColorIndex; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetRoundWins() const { return RoundWins; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetGrandWizardFavor() const { return GrandWizardFavor; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetCurrentTrialScore() const { return CurrentTrialScore; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	int32 GetStaffsAtDawnScore() const { return StaffsAtDawnScore; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	bool IsPartyHallReady() const { return bPartyHallReady; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	bool IsPlaytestBotSlot() const { return bPlaytestBot; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Online Scaffold")
	FString GetWizardSummaryText() const { return WizardSummaryText; }

	void SendAuthoritativeSteamMatchResultToOwner(
		int32 MatchGeneration,
		int32 PlayerSlot,
		int32 WinnerSlot,
		int32 FinalGrandWizardFavor,
		int32 FinalRoundWins);

protected:
	UFUNCTION(Client, Reliable)
	void ClientSubmitAuthoritativeSteamMatchResult(
		int32 MatchGeneration,
		int32 PlayerSlot,
		int32 WinnerSlot,
		int32 FinalGrandWizardFavor,
		int32 FinalRoundWins);

	UFUNCTION()
	void OnRep_WizardDisplaySlot();

	UPROPERTY(ReplicatedUsing = OnRep_WizardDisplaySlot, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 WizardDisplaySlot = INDEX_NONE;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 WizardColorIndex = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 RoundWins = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 GrandWizardFavor = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 CurrentTrialScore = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	int32 StaffsAtDawnScore = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	bool bPartyHallReady = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	bool bPlaytestBot = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Online Scaffold")
	FString WizardSummaryText;
};
