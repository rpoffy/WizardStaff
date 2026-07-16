#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffCauldronHazard.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EWizardCauldronHazardType : uint8
{
	Slippery,
	Sticky
};

UCLASS()
class WIZARDSTAFF_API AWizardStaffCauldronHazard : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffCauldronHazard();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeHazard(EWizardCauldronHazardType NewType, float NewRadius, const FVector& NewEruptionOrigin);
	EWizardCauldronHazardType GetHazardType() const { return HazardType; }
	float GetHazardRadius() const { return HazardRadius; }

private:
	UFUNCTION()
	void OnRep_HazardPresentation();

	void ApplyPresentation();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> HazardMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> EruptionMesh;

	UPROPERTY(ReplicatedUsing = OnRep_HazardPresentation)
	EWizardCauldronHazardType HazardType = EWizardCauldronHazardType::Slippery;

	UPROPERTY(ReplicatedUsing = OnRep_HazardPresentation)
	float HazardRadius = 180.0f;

	UPROPERTY(ReplicatedUsing = OnRep_HazardPresentation)
	FVector EruptionOrigin = FVector::ZeroVector;

	float EruptionTimeRemaining = 0.65f;
};
