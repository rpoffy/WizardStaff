#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffStaffsAtDawnArena.generated.h"

class UArrowComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffStaffsAtDawnArena : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffStaffsAtDawnArena();

	UFUNCTION(BlueprintCallable, Category = "Staffs At Dawn Arena|Spawns")
	bool GetPlayerSpawnTransform(int32 PlayerIndex, int32 PlayerCount, FTransform& OutTransform) const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn Arena|Spawns")
	TArray<FTransform> GetPlayerSpawnTransforms() const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn Arena|Spawns")
	int32 GetPlayerSpawnCount() const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn Arena|Spawns")
	TArray<FVector> GetFuturePowerupSpawnLocations() const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn Arena|Spawns")
	int32 GetFuturePowerupSpawnCount() const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn Arena|Bounds")
	FVector GetArenaBoundsCenter() const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn Arena|Bounds")
	float GetArenaHalfSize() const { return ArenaHalfSize; }

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn Arena|Bounds")
	float GetRingOutFallZ() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staffs At Dawn Arena")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn Arena|Bounds", meta = (ClampMin = "300.0"))
	float ArenaHalfSize = 2100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn Arena|Bounds", meta = (ClampMin = "0.0"))
	float RingOutFallDistanceBelowArena = 120.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staffs At Dawn Arena|Blocks")
	TArray<TObjectPtr<UStaticMeshComponent>> ArenaBlockMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staffs At Dawn Arena|Spawns")
	TArray<TObjectPtr<UArrowComponent>> PlayerSpawnMarkers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staffs At Dawn Arena|Spawns")
	TArray<TObjectPtr<UArrowComponent>> FuturePowerupSpawnMarkers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn Arena|Assets")
	TObjectPtr<UStaticMesh> BlockMeshAsset;

protected:
	UStaticMeshComponent* CreateBlockComponent(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale, const FRotator& RelativeRotation = FRotator::ZeroRotator);
	UArrowComponent* CreateSpawnMarker(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FColor& Color, float ArrowSize);
	void GatherSpawnMarkers(const FString& NamePrefix, TArray<UArrowComponent*>& OutMarkers) const;
};
