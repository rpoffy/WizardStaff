#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WizardStaffPlaytestBotComponent.generated.h"

class AWizardStaffGameMode;
class AWizardStaffManaMugPickup;
class AWizardStaffStaffsAtDawnPowerupPickup;
class AWizardStaffWizardCharacter;

UCLASS(ClassGroup = (Wizard), Blueprintable, meta = (BlueprintSpawnableComponent))
class WIZARDSTAFF_API UWizardStaffPlaytestBotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWizardStaffPlaytestBotComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Wizard|Playtest Bot")
	void SetBotEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category = "Wizard|Playtest Bot")
	bool IsBotEnabled() const { return bBotEnabled; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Playtest Bot")
	FString GetBotDebugState() const { return BotDebugState; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Playtest Bot")
	FVector GetCurrentTargetLocation() const { return CurrentTargetLocation; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Playtest Bot")
	bool HasCurrentTarget() const { return bHasTarget; }

protected:
	AWizardStaffWizardCharacter* GetWizardOwner() const;
	AWizardStaffGameMode* GetWizardGameMode() const;

	void ResetBotState();
	void UpdateStateSignature();
	void Think(float DeltaTime);
	void ApplyMovement(float DeltaTime);
	void UpdateStuckHandling(float DeltaTime);
	void DrawDebug() const;

	bool ChooseTarget(FVector& OutTargetLocation, FString& OutReason);
	bool ChoosePartyHallTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason);
	bool ChooseMugRunTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason);
	bool ChooseStaffsAtDawnTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason);
	bool ChooseFinalRoundTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason);
	bool ChooseWanderTarget(AWizardStaffGameMode* GameMode, FVector& OutTargetLocation, FString& OutReason, const FString& Reason);

	AWizardStaffWizardCharacter* FindNearestOpponent(const AWizardStaffWizardCharacter* Wizard, float MaxDistance = 0.0f) const;
	AWizardStaffManaMugPickup* FindNearestActiveMug(const AWizardStaffWizardCharacter* Wizard) const;
	AWizardStaffStaffsAtDawnPowerupPickup* FindNearestActiveStaffsAtDawnPowerup(const AWizardStaffWizardCharacter* Wizard, float MaxDistance) const;

	void MaybeUseActions(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, float DeltaTime);
	void MaybeUseBroomRecovery(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard);

	int32 GetOwnerPlayerIndex() const;
	float GetDistanceToTarget2D(const AWizardStaffWizardCharacter* Wizard) const;
	bool IsNearArenaEdge(const AWizardStaffGameMode* GameMode, const FVector& Location, float EdgeAlpha = 0.72f) const;

private:
	UPROPERTY(Transient)
	bool bBotEnabled = false;

	UPROPERTY(Transient)
	bool bHasTarget = false;

	UPROPERTY(Transient)
	FVector CurrentTargetLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FString BotDebugState = TEXT("Disabled");

	UPROPERTY(Transient)
	FString TargetReason = TEXT("None");

	float ThinkTimeRemaining = 0.0f;
	float TargetRefreshRemaining = 0.0f;
	float ReadyBellAllowedTime = 0.0f;
	float ReverseTimeRemaining = 0.0f;
	float ReverseTurnDirection = 1.0f;
	float StuckTime = 0.0f;
	float LastBroomAttemptTime = -1000.0f;
	FVector LastProgressLocation = FVector::ZeroVector;
	FString LastStateSignature;
};
