#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffCauldronDepositArc.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class WIZARDSTAFF_API AWizardStaffCauldronDepositArc : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffCauldronDepositArc();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeDepositArc(UStaticMesh* SegmentMesh, const FTransform& StartTransform, const FVector& EndLocation, const FLinearColor& SegmentColor, float DurationSeconds = 0.48f);

private:
	UFUNCTION()
	void OnRep_DepositArcPresentation();

	void ApplyPresentation();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArcMesh;

	UPROPERTY(ReplicatedUsing = OnRep_DepositArcPresentation)
	TObjectPtr<UStaticMesh> ReplicatedSegmentMesh;

	UPROPERTY(ReplicatedUsing = OnRep_DepositArcPresentation)
	FVector ReplicatedStartLocation = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_DepositArcPresentation)
	FRotator ReplicatedStartRotation = FRotator::ZeroRotator;

	UPROPERTY(ReplicatedUsing = OnRep_DepositArcPresentation)
	FVector ReplicatedStartScale = FVector::OneVector;

	UPROPERTY(ReplicatedUsing = OnRep_DepositArcPresentation)
	FVector ReplicatedEndLocation = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_DepositArcPresentation)
	FLinearColor ReplicatedSegmentColor = FLinearColor::White;

	UPROPERTY(ReplicatedUsing = OnRep_DepositArcPresentation)
	float ReplicatedDuration = 0.48f;

	float ElapsedTime = 0.0f;
	bool bPresentationReady = false;
};