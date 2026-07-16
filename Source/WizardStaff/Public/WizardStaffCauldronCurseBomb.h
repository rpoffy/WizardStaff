#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffCauldronCurseBomb.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class WIZARDSTAFF_API AWizardStaffCauldronCurseBomb : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffCauldronCurseBomb();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeBomb(const FVector& NewOrigin, const FVector& NewTargetLocation, float NewBlastRadius, float NewLaunchDelay, float NewFlightDuration, float NewHorizontalKnockback, float NewVerticalKnockback);

private:
	UFUNCTION()
	void OnRep_BombPresentation();

	void ApplyPresentation();
	void ApplyExplosion();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ExplosionMesh;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> TargetMarkerSegments;

	UPROPERTY(ReplicatedUsing = OnRep_BombPresentation)
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_BombPresentation)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_BombPresentation)
	float BlastRadius = 200.0f;

	UPROPERTY(ReplicatedUsing = OnRep_BombPresentation)
	float LaunchDelay = 1.0f;

	UPROPERTY(ReplicatedUsing = OnRep_BombPresentation)
	float FlightDuration = 0.55f;

	UPROPERTY(Replicated)
	float HorizontalKnockback = 2325.0f;

	UPROPERTY(Replicated)
	float VerticalKnockback = 937.5f;

	float ElapsedTime = 0.0f;
	float ExplosionVisualTimeRemaining = 0.0f;
	bool bExplosionApplied = false;
};