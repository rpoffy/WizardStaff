#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffManaMugPickup.generated.h"

class AWizardStaffWizardCharacter;
class UMaterialInterface;
class USphereComponent;
class UStaticMesh;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FWizardManaMugPickupTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug", meta = (ClampMin = "0.0"))
	float RespawnDelay = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug", meta = (ClampMin = "1.0"))
	float PickupRadius = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug")
	bool bRespawnAfterPickup = true;
};

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffManaMugPickup : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffManaMugPickup();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Mug")
	void Collect(AWizardStaffWizardCharacter* CollectingWizard);

	UFUNCTION(BlueprintCallable, Category = "Mug")
	void Respawn();

	UFUNCTION(BlueprintCallable, Category = "Mug")
	void SetPickupActive(bool bNewActive);

	UFUNCTION(BlueprintPure, Category = "Mug")
	bool IsPickupActive() const { return bIsPickupActive; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mug")
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mug")
	TObjectPtr<UStaticMeshComponent> MugBodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mug")
	TObjectPtr<UStaticMeshComponent> ManaTopMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mug")
	TObjectPtr<UStaticMeshComponent> HandleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug|Tuning")
	FWizardManaMugPickupTuning PickupTuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug|Visuals")
	FLinearColor MugColor = FLinearColor(0.75f, 0.55f, 0.28f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug|Visuals")
	FLinearColor ManaColor = FLinearColor(0.10f, 0.75f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug|Assets")
	TObjectPtr<UStaticMesh> MugMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug|Assets")
	TObjectPtr<UStaticMesh> ManaMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug|Assets")
	TObjectPtr<UMaterialInterface> MugMaterial;

protected:
	UFUNCTION()
	void HandlePickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_PickupActive();

	void ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const;
	void ApplyPickupActiveState();

private:
	FTimerHandle RespawnTimerHandle;
	UPROPERTY(ReplicatedUsing = OnRep_PickupActive)
	bool bIsPickupActive = true;
};
