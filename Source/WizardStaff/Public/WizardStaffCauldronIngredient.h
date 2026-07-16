#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffCauldronIngredient.generated.h"

class UStaticMeshComponent;

UCLASS()
class WIZARDSTAFF_API AWizardStaffCauldronIngredient : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffCauldronIngredient();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeIngredient(int32 NewVariantIndex);
	bool ApplyAuthoritativeBonk(int32 PlayerIndex, const FVector& Direction, float ImpulseStrength);
	bool MarkScored();

	int32 GetLastAttributionPlayerIndex() const { return LastAttributionPlayerIndex; }
	bool IsScored() const { return bScored; }

private:
	UFUNCTION()
	void OnRep_IngredientPresentation();

	void ApplyPresentation();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> IngredientMesh;

	UPROPERTY(ReplicatedUsing = OnRep_IngredientPresentation)
	int32 VariantIndex = 0;

	UPROPERTY(Replicated)
	int32 LastAttributionPlayerIndex = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_IngredientPresentation)
	bool bScored = false;
};
