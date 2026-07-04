#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WizardStaffComponent.generated.h"

class UMaterialInterface;
class UBoxComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWizardStaffSegmentSnappedSignature, int32, NewSegmentCount, float, RemainingStress);

USTRUCT(BlueprintType)
struct FWizardStaffVisualTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (ClampMin = "1.0"))
	float BaseStaffFallbackHeight = 170.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FVector BaseStaffVisualScale = FVector(0.10f, 0.10f, 1.70f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (ClampMin = "1.0"))
	float SegmentFallbackHeight = 34.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FVector SegmentVisualScale = FVector(0.38f, 0.38f, 0.34f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals", meta = (ClampMin = "0.0"))
	float SegmentSpacing = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "0", ClampMax = "50"))
	int32 MaxTestSegments = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor BaseStaffColor = FLinearColor(0.42f, 0.24f, 0.10f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor SegmentColor = FLinearColor(0.05f, 0.82f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor SegmentAlternateColor = FLinearColor(1.0f, 0.58f, 0.12f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	bool bAlternateSegmentColors = true;
};

USTRUCT(BlueprintType)
struct FWizardStaffCollisionTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bStaffCollisionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "1.0"))
	float BaseCollisionLength = 170.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "1.0"))
	float CollisionLengthPerSegment = 38.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "1.0"))
	float CollisionThickness = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FVector CollisionLocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ObstructedControlMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "0.0"))
	float ObstructionRecoverySpeed = 8.0f;
};

USTRUCT(BlueprintType)
struct FWizardStaffStressTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "1.0"))
	float MaxStaffStress = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float StressGainedPerBonk = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float StressGainedPerWallImpact = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float StressMultiplierPerSegment = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float StressDecayRate = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "1.0"))
	float WallImpactSpeedForFullStress = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float CaughtStressPerSecond = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StressAfterSnapRatio = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float SnapImpulseForce = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float SnapUpwardImpulse = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float SnappedSegmentLinearDamping = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress", meta = (ClampMin = "0.0"))
	float SnappedSegmentAngularDamping = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress")
	bool bSnapImpulseIgnoresSegmentMass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress")
	bool bShowStressDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardStaffStuckTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck")
	bool bEnableStuckFailsafe = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float MovementInputThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float MovementInputGraceTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float OwnerMoveSpeedThreshold = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float StuckTimeBeforeStressBoost = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float StuckStressPerSecond = 26.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float StuckTimeBeforeCollisionRelief = 2.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float CollisionReliefDuration = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float CollisionReliefCooldown = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CollisionReliefControlMultiplier = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck", meta = (ClampMin = "0.0"))
	float GentleNudgeDistance = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stuck")
	bool bShowStuckDebug = true;
};

UCLASS(ClassGroup = (Wizard), Blueprintable, meta = (BlueprintSpawnableComponent))
class WIZARDSTAFF_API UWizardStaffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWizardStaffComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff")
	void InitializeStaff(USceneComponent* InAttachParent);

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff")
	int32 AddStaffSegment();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff")
	void RebuildStaffSegmentsForCount(int32 TargetSegmentCount);

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff")
	bool RemoveTopStaffSegment(bool bSpawnPhysicsSegment = false);

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff")
	void ClearStaffSegments();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff")
	void ResetForNewMatch(bool bResetSegments);

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Stuck")
	void NotifyOwnerMovementInput(float InputAmount);

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Stress")
	bool AddStaffStress(float BaseStressAmount, FName StressSource);

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff|Stress")
	bool SnapTopStaffSegment();

	UFUNCTION(BlueprintPure, Category = "Wizard Staff")
	int32 GetSegmentCount() const { return SegmentCount; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Stress")
	float GetStaffStressAlpha() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Stress")
	float GetStressMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Collision")
	float GetStaffCollisionLength() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Collision")
	UBoxComponent* GetStaffCollisionBox() const { return StaffCollisionBox.Get(); }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Collision")
	float GetControlInputMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Wizard Staff|Collision")
	bool IsStaffObstructed() const { return bIsStaffObstructed; }

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff")
	int32 SegmentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Sockets")
	FName TopSocketName = TEXT("TopSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Sockets")
	FName BottomSocketName = TEXT("BottomSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Tuning")
	FWizardStaffVisualTuning VisualTuning;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Stress")
	float StaffStress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Tuning")
	FWizardStaffStressTuning StressTuning;

	UPROPERTY(BlueprintAssignable, Category = "Wizard Staff|Stress")
	FWizardStaffSegmentSnappedSignature OnStaffSegmentSnapped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Tuning")
	FWizardStaffCollisionTuning CollisionTuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Tuning")
	FWizardStaffStuckTuning StuckTuning;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Collision")
	bool bIsStaffObstructed = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Collision")
	float StaffObstructionAlpha = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Stuck")
	bool bIsStaffStuck = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Stuck")
	float StaffStuckTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Stuck")
	float StaffCollisionReliefRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Assets")
	TObjectPtr<UStaticMesh> BaseStaffMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Assets")
	TObjectPtr<UStaticMesh> SegmentMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wizard Staff|Assets")
	TObjectPtr<UMaterialInterface> StaffMaterial;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Visuals")
	TObjectPtr<USceneComponent> BaseAnchor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Visuals")
	TObjectPtr<UStaticMeshComponent> BaseStaffMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff|Collision")
	TObjectPtr<UBoxComponent> StaffCollisionBox;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	USceneComponent* CreateRuntimeAnchor(FName Name, USceneComponent* AttachParent, const FTransform& RelativeTransform, FName AttachSocketName = NAME_None);
	UStaticMeshComponent* CreateRuntimeMesh(FName Name, USceneComponent* AttachParent, UStaticMesh* MeshAsset, const FVector& RelativeLocation, const FVector& VisualScale, const FLinearColor& Color);
	FTransform GetFallbackSegmentAnchorTransform() const;
	void ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const;
	void CreateStaffCollision();
	void UpdateStaffCollision();
	void RefreshStaffObstruction(float DeltaTime);
	void UpdateStuckTimers(float DeltaTime);
	void UpdateStuckFailsafe(float DeltaTime, bool bHasBlockingObstacle, float OwnerMoveSpeed);
	void ResetStuckState();
	void TriggerCollisionRelief();
	void ApplyGentleOwnerNudge();
	void UpdateStressDecay(float DeltaTime);
	void UpdateStressVisuals(float DeltaTime);
	void SpawnSnappedPhysicsSegment(const FTransform& SegmentTransform);
	void RemoveTopSegmentComponents();
	void NotifyOwnerSegmentCountChanged() const;
	void NotifyOwnerStaffStressChanged(bool bForce = false) const;
	void DrawStressDebug() const;
	int32 AddStaffSegmentInternal(bool bRecordTelemetry);

private:
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> StaffAttachParent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USceneComponent>> SegmentAnchors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SegmentMeshes;

	bool bWasStaffObstructed = false;
	FVector LastCollisionLocation = FVector::ZeroVector;
	bool bHasLastCollisionLocation = false;
	FVector LastOwnerLocation = FVector::ZeroVector;
	bool bHasLastOwnerLocation = false;
	float RecentOwnerMoveInputTime = 0.0f;
	float StuckReliefCooldownRemaining = 0.0f;
	float StressRattlePhase = 0.0f;
};
