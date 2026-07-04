#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffPrototypeArena.generated.h"

class UArrowComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffPrototypeArena : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffPrototypeArena();

	UFUNCTION(BlueprintCallable, Category = "Prototype Arena|Spawns")
	bool GetPlayerSpawnTransform(int32 PlayerIndex, int32 PlayerCount, FTransform& OutTransform) const;

	UFUNCTION(BlueprintPure, Category = "Prototype Arena|Spawns")
	TArray<FVector> GetMugSpawnLocations() const;

	UFUNCTION(BlueprintPure, Category = "Prototype Arena|Bounds")
	FVector GetArenaBoundsCenter() const;

	UFUNCTION(BlueprintPure, Category = "Prototype Arena|Bounds")
	float GetArenaHalfSize() const { return ArenaHalfSize; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Arena")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Arena|Bounds", meta = (ClampMin = "300.0"))
	float ArenaHalfSize = 950.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Arena|Blocks")
	TArray<TObjectPtr<UStaticMeshComponent>> ArenaBlockMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Arena|Spawns")
	TArray<TObjectPtr<UArrowComponent>> PlayerSpawnMarkers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Arena|Spawns")
	TArray<TObjectPtr<UArrowComponent>> MugSpawnMarkers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Arena|Assets")
	TObjectPtr<UStaticMesh> BlockMeshAsset;

protected:
	UStaticMeshComponent* CreateBlockComponent(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale);
	UArrowComponent* CreateSpawnMarker(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FColor& Color, float ArrowSize);
	void GatherSpawnMarkers(const FString& NamePrefix, TArray<UArrowComponent*>& OutMarkers) const;
};
