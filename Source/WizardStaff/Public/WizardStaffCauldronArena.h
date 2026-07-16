#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffCauldronArena.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EWizardCauldronIntakeDirection : uint8
{
	None,
	North,
	East,
	South,
	West
};

UCLASS()
class WIZARDSTAFF_API AWizardStaffCauldronArena : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffCauldronArena();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FVector GetAcceptanceCenter() const;
	float GetFloorSurfaceZ() const;
	float GetAcceptanceRadius() const { return 184.0f; }
	FVector GetIntakeWorldLocation(EWizardCauldronIntakeDirection IntakeDirection) const;
	void ConfigureIntakePresentation(float NewDistanceFromCenter, float NewVisualScale, float NewPulseSpeed);
	void SetIntakeState(EWizardCauldronIntakeDirection NewActiveIntake, EWizardCauldronIntakeDirection NewPreviewIntake, bool bNewRelocating);
	void SetIntakeBankingState(bool bNewOccupied, float NewProgressAlpha);
	void TriggerDepositPulse();
	void SetCurseWarningActive(bool bNewWarningActive);
	void SetCurseDepositWarningActive(bool bNewWarningActive);

private:
	UFUNCTION()
	void OnRep_DepositPulseSequence();

	UFUNCTION()
	void OnRep_CurseWarningActive();

	UFUNCTION()
	void OnRep_CurseDepositWarningActive();

	UFUNCTION()
	void OnRep_IntakeState();

	void ApplyCurseWarningPresentation();
	void ApplyIntakePresentation();
	FVector GetIntakeLocalDirection(EWizardCauldronIntakeDirection IntakeDirection) const;
	float GetIntakeYaw(EWizardCauldronIntakeDirection IntakeDirection) const;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CauldronBody;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CauldronRim;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BrewSurface;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> BubbleMeshes;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> IntakeMarkerMeshes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> IntakeMarkerMaterials;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArenaFloor;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> OctagonEdgeMeshes;

	UPROPERTY(ReplicatedUsing = OnRep_DepositPulseSequence)
	int32 ReplicatedDepositPulseSequence = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CurseWarningActive)
	bool bReplicatedCurseWarningActive = false;

	UPROPERTY(ReplicatedUsing = OnRep_CurseDepositWarningActive)
	bool bReplicatedCurseDepositWarningActive = false;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	EWizardCauldronIntakeDirection ReplicatedActiveIntake = EWizardCauldronIntakeDirection::None;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	EWizardCauldronIntakeDirection ReplicatedPreviewIntake = EWizardCauldronIntakeDirection::None;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	bool bReplicatedIntakeRelocating = false;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	float ReplicatedIntakeDistanceFromCenter = 315.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	float ReplicatedIntakeVisualScale = 1.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	float ReplicatedIntakePulseSpeed = 4.5f;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	bool bReplicatedIntakeOccupied = false;

	UPROPERTY(ReplicatedUsing = OnRep_IntakeState)
	float ReplicatedIntakeBankingProgressAlpha = 0.0f;

	float DepositPulseTimeRemaining = 0.0f;
};
