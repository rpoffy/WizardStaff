#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffPartyHall.generated.h"

class UArrowComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;
class USoundBase;

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffPartyHall : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffPartyHall();

	virtual void Tick(float DeltaSeconds) override;

	static FName GetReadyBellComponentTag();

	UFUNCTION(BlueprintCallable, Category = "Party Hall|Spawns")
	bool GetPlayerSpawnTransform(int32 PlayerIndex, int32 PlayerCount, FTransform& OutTransform) const;

	UFUNCTION(BlueprintCallable, Category = "Party Hall|Signs")
	void UpdateIntermissionSigns(const FString& StandingsText, const FString& CountdownText, const FString& PresetText, const FString& LeaderText);

	UFUNCTION(BlueprintCallable, Category = "Party Hall|Ready Bell")
	void PlayReadyBellFeedback(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Party Hall|Ready Bell")
	void ResetReadyBellFeedback();

	UFUNCTION(BlueprintPure, Category = "Party Hall|Ready Bell")
	bool GetReadyBellWorldLocation(FVector& OutLocation) const;

	UFUNCTION(BlueprintPure, Category = "Party Hall|Bounds")
	FVector GetHallBoundsCenter() const;

	UFUNCTION(BlueprintPure, Category = "Party Hall|Bounds")
	float GetHallHalfSize() const { return HallHalfSize; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Bounds", meta = (ClampMin = "300.0"))
	float HallHalfSize = 640.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Blocks")
	TArray<TObjectPtr<UStaticMeshComponent>> HallBlockMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Props")
	TArray<TObjectPtr<UStaticMeshComponent>> PhysicsPropMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Signs")
	TArray<TObjectPtr<UTextRenderComponent>> HallSignTexts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Spawns")
	TArray<TObjectPtr<UArrowComponent>> PlayerSpawnMarkers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Assets")
	TObjectPtr<UStaticMesh> BlockMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Assets")
	TObjectPtr<UStaticMesh> CylinderMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Ready Bell")
	TObjectPtr<USoundBase> ReadyBellSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Ready Bell", meta = (ClampMin = "0.0"))
	float ReadyBellPulseDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Ready Bell", meta = (ClampMin = "1.0"))
	float ReadyBellPulseScale = 1.22f;

protected:
	virtual void BeginPlay() override;

	UStaticMeshComponent* CreateBlockComponent(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale, const FRotator& RelativeRotation = FRotator::ZeroRotator, bool bEnableCollision = true);
	UStaticMeshComponent* CreatePhysicsPropComponent(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FVector& RelativeScale, bool bUseCylinderMesh = false);
	UTextRenderComponent* CreateSignTextComponent(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FString& InitialText, const FColor& Color, float TextSize);
	UArrowComponent* CreateSpawnMarker(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FColor& Color, float ArrowSize);
	void GatherSpawnMarkers(const FString& NamePrefix, TArray<UArrowComponent*>& OutMarkers) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Signs")
	TObjectPtr<UTextRenderComponent> StandingsSignText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Signs")
	TObjectPtr<UTextRenderComponent> CountdownSignText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Signs")
	TObjectPtr<UTextRenderComponent> PresetSignText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Signs")
	TObjectPtr<UTextRenderComponent> LeaderSignText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Ready Bell")
	TObjectPtr<UStaticMeshComponent> ReadyBellBaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Ready Bell")
	TObjectPtr<UStaticMeshComponent> ReadyBellMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party Hall|Ready Bell")
	TObjectPtr<UTextRenderComponent> ReadyBellText;

	float ReadyBellPulseRemaining = 0.0f;
	FVector ReadyBellDefaultScale = FVector::OneVector;
};
