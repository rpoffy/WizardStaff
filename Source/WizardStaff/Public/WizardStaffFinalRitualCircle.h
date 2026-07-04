#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffFinalRitualCircle.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffFinalRitualCircle : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffFinalRitualCircle();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetCircleState(const FVector& Center, float Radius, const FLinearColor& Color, bool bVisible);
	void SetCircleVisible(bool bVisible);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grand Wizard Final")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grand Wizard Final")
	TObjectPtr<UStaticMeshComponent> CircleMesh;

protected:
	UFUNCTION()
	void OnRep_CircleState();

	void ApplyCircleState();
	void ApplyCircleColor();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grand Wizard Final")
	TObjectPtr<UMaterialInterface> CircleMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CircleMaterialInstance;

	UPROPERTY(ReplicatedUsing = OnRep_CircleState)
	FVector ReplicatedCircleCenter = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_CircleState)
	float ReplicatedCircleRadius = 220.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CircleState)
	FLinearColor ReplicatedCircleColor = FLinearColor(0.25f, 0.95f, 1.0f, 1.0f);

	UPROPERTY(ReplicatedUsing = OnRep_CircleState)
	bool bReplicatedCircleVisible = false;
};
