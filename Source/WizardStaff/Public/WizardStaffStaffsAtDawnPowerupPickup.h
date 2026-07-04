#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffStaffsAtDawnPowerupTypes.h"
#include "WizardStaffStaffsAtDawnPowerupPickup.generated.h"

class AWizardStaffWizardCharacter;
class UMaterialInterface;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffStaffsAtDawnPowerupPickup : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffStaffsAtDawnPowerupPickup();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Staffs At Dawn|Powerup")
	void SetPickupActive(bool bNewActive);

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn|Powerup")
	bool IsPickupActive() const { return bIsPickupActive; }

	UFUNCTION(BlueprintCallable, Category = "Staffs At Dawn|Powerup")
	void SetPowerupType(EWizardStaffsAtDawnPowerupType NewPowerupType);

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn|Powerup")
	EWizardStaffsAtDawnPowerupType GetPowerupType() const { return PowerupType; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staffs At Dawn|Powerup")
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staffs At Dawn|Powerup")
	TObjectPtr<UStaticMeshComponent> PowerupBaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staffs At Dawn|Powerup")
	TObjectPtr<UStaticMeshComponent> PowerupGlowMesh;

	UPROPERTY(ReplicatedUsing = OnRep_PowerupType, EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerup")
	EWizardStaffsAtDawnPowerupType PowerupType = EWizardStaffsAtDawnPowerupType::MegaStaffBrew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerup|Tuning", meta = (ClampMin = "1.0"))
	float PickupRadius = 78.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerup|Visuals")
	FLinearColor MegaStaffBrewColor = FLinearColor(0.20f, 0.95f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerup|Visuals")
	FLinearColor EmptyPowerupColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerup|Assets")
	TObjectPtr<UStaticMesh> BaseMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerup|Assets")
	TObjectPtr<UStaticMesh> GlowMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerup|Assets")
	TObjectPtr<UMaterialInterface> PowerupMaterial;

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

	UFUNCTION()
	void OnRep_PowerupType();

	void ApplyPowerupVisuals() const;
	void ApplyPickupActiveState();
	void ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const;
	FLinearColor GetPowerupColor() const;

private:
	UPROPERTY(ReplicatedUsing = OnRep_PickupActive)
	bool bIsPickupActive = true;
};
