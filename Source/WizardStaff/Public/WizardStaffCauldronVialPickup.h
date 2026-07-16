#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffCauldronVialTypes.h"
#include "WizardStaffCauldronVialPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class WIZARDSTAFF_API AWizardStaffCauldronVialPickup : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffCauldronVialPickup();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeVial(EWizardCauldronVialType NewVialType, float NewPickupRadius);
	EWizardCauldronVialType GetVialType() const { return VialType; }

private:
	UFUNCTION()
	void HandlePickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_VialPresentation();

	void ApplyPresentation();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> VialBodyMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> VialGlowMesh;

	UPROPERTY(ReplicatedUsing = OnRep_VialPresentation)
	EWizardCauldronVialType VialType = EWizardCauldronVialType::None;

	UPROPERTY(ReplicatedUsing = OnRep_VialPresentation)
	bool bPickupActive = true;

	float PickupRadius = 70.0f;
};
