#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffWizardCharacter.h"
#include "WizardStaffArcanePinballProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffArcanePinballProjectile : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffArcanePinballProjectile();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Arcane Pinball")
	void InitializeArcanePinball(AWizardStaffWizardCharacter* InCaster, const FWizardArcanePinballTuning& InTuning, const FVector& LaunchDirection);

	UFUNCTION(BlueprintCallable, Category = "Arcane Pinball")
	void InitializeArcanePinballReadabilityShell(AWizardStaffWizardCharacter* InCaster, const FWizardArcanePinballTuning& InTuning, const FVector& LaunchDirection);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arcane Pinball")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arcane Pinball")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arcane Pinball")
	TObjectPtr<UStaticMeshComponent> TrailMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arcane Pinball")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball")
	FWizardArcanePinballTuning Tuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball|Visuals")
	TObjectPtr<UStaticMesh> ProjectileMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball|Visuals")
	TObjectPtr<UMaterialInterface> ProjectileMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball|Visuals")
	FLinearColor ProjectileColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball|Visuals")
	FLinearColor TrailColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball|Visuals")
	FVector TrailRelativeLocation = FVector(-78.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball|Visuals")
	FVector TrailScale = FVector(1.2f, 0.07f, 0.07f);

protected:
	UFUNCTION()
	void HandleProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	void ApplyHitToWizard(AWizardStaffWizardCharacter* HitWizard, const FHitResult& Hit);
	bool CanHitWizard(AWizardStaffWizardCharacter* HitWizard) const;
	int32 GetWizardPlayerIndex(const AWizardStaffWizardCharacter* Wizard) const;
	void ConstrainToLaunchHeight();
	void ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const;
	void ConfigureArcanePinball(AWizardStaffWizardCharacter* InCaster, const FWizardArcanePinballTuning& InTuning, const FVector& LaunchDirection, bool bInReadabilityOnlyShell);
	void ApplyReadabilityShellState();
	void DeactivateArcanePinballGameplay();

	UFUNCTION()
	void OnRep_ReadabilityOnlyShell();

	TWeakObjectPtr<AWizardStaffWizardCharacter> CasterWizard;

	TMap<TWeakObjectPtr<AWizardStaffWizardCharacter>, float> RecentWizardHitTimes;

	int32 BounceCount = 0;
	float PlayerHitCooldown = 0.35f;
	float LockedHeightZ = 0.0f;
	bool bHasLockedHeight = false;
	bool bArcanePinballGameplayEnded = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReadabilityOnlyShell)
	bool bReadabilityOnlyShell = false;
};
