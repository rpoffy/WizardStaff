#include "WizardStaffComponent.h"

#include "CollisionQueryParams.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffHUD.h"
#include "WizardStaffWizardCharacter.h"
#include "WorldCollision.h"

UWizardStaffComponent::UWizardStaffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	if (CylinderMesh.Succeeded())
	{
		BaseStaffMeshAsset = CylinderMesh.Object;
		SegmentMeshAsset = CylinderMesh.Object;
	}

	if (BasicMaterial.Succeeded())
	{
		StaffMaterial = BasicMaterial.Object;
	}
}

void UWizardStaffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWizardStaffComponent::InitializeStaff(USceneComponent* InAttachParent)
{
	if (!InAttachParent || BaseAnchor)
	{
		return;
	}

	StaffAttachParent = InAttachParent;
	BaseAnchor = CreateRuntimeAnchor(TEXT("StaffBaseAnchor"), StaffAttachParent, FTransform::Identity);
	if (!BaseAnchor)
	{
		return;
	}

	BaseStaffMesh = CreateRuntimeMesh(
		TEXT("BaseStaffMesh"),
		BaseAnchor,
		BaseStaffMeshAsset,
		FVector(0.0f, 0.0f, VisualTuning.BaseStaffFallbackHeight * 0.5f),
		VisualTuning.BaseStaffVisualScale,
		VisualTuning.BaseStaffColor);

	CreateStaffCollision();
	UpdateStaffCollision();
}

int32 UWizardStaffComponent::AddStaffSegment()
{
	const int32 NewSegmentCount = AddStaffSegmentInternal(true);
	NotifyOwnerSegmentCountChanged();
	return NewSegmentCount;
}

void UWizardStaffComponent::RebuildStaffSegmentsForCount(int32 TargetSegmentCount)
{
	const float SavedStress = StaffStress;
	const int32 SafeTargetSegmentCount = FMath::Clamp(TargetSegmentCount, 0, FMath::Max(VisualTuning.MaxTestSegments, 0));
	ClearStaffSegments();

	for (int32 SegmentIndex = 0; SegmentIndex < SafeTargetSegmentCount; ++SegmentIndex)
	{
		const int32 PreviousCount = SegmentCount;
		const int32 NewCount = AddStaffSegmentInternal(false);
		if (NewCount <= PreviousCount)
		{
			break;
		}
	}

	StaffStress = SegmentCount > 0 ? FMath::Clamp(SavedStress, 0.0f, FMath::Max(StressTuning.MaxStaffStress, 1.0f)) : 0.0f;
	UpdateStaffCollision();
	NotifyOwnerSegmentCountChanged();
	NotifyOwnerStaffStressChanged(true);
}

int32 UWizardStaffComponent::AddStaffSegmentInternal(bool bRecordTelemetry)
{
	if (SegmentCount >= VisualTuning.MaxTestSegments)
	{
		return SegmentCount;
	}

	if (!BaseAnchor)
	{
		if (AActor* Owner = GetOwner())
		{
			InitializeStaff(Owner->GetRootComponent());
		}
	}

	if (!BaseAnchor)
	{
		return SegmentCount;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return SegmentCount;
	}

	const int32 NewSegmentIndex = SegmentCount;
	const FName AnchorName = *FString::Printf(TEXT("StaffSegmentAnchor_%02d"), NewSegmentIndex);
	const FName MeshName = *FString::Printf(TEXT("StaffSegmentMesh_%02d"), NewSegmentIndex);

	USceneComponent* AttachParent = SegmentAnchors.Num() > 0 ? SegmentAnchors.Last().Get() : BaseAnchor.Get();
	FName AttachSocketName = NAME_None;
	FTransform AnchorTransform = GetFallbackSegmentAnchorTransform();

	UStaticMeshComponent* PreviousMesh = SegmentMeshes.Num() > 0 ? SegmentMeshes.Last().Get() : BaseStaffMesh.Get();
	if (PreviousMesh && PreviousMesh->DoesSocketExist(TopSocketName))
	{
		AttachParent = PreviousMesh;
		AttachSocketName = TopSocketName;
		AnchorTransform = FTransform::Identity;
	}

	USceneComponent* NewAnchor = CreateRuntimeAnchor(AnchorName, AttachParent, AnchorTransform, AttachSocketName);
	if (!NewAnchor)
	{
		return SegmentCount;
	}

	const FLinearColor SegmentColor = VisualTuning.bAlternateSegmentColors && (NewSegmentIndex % 2 == 1)
		? VisualTuning.SegmentAlternateColor
		: VisualTuning.SegmentColor;

	UStaticMeshComponent* NewSegmentMesh = CreateRuntimeMesh(
		MeshName,
		NewAnchor,
		SegmentMeshAsset,
		FVector(0.0f, 0.0f, VisualTuning.SegmentFallbackHeight * 0.5f),
		VisualTuning.SegmentVisualScale,
		SegmentColor);

	if (!NewSegmentMesh)
	{
		NewAnchor->DestroyComponent();
		return SegmentCount;
	}

	SegmentAnchors.Add(NewAnchor);
	SegmentMeshes.Add(NewSegmentMesh);
	SegmentCount = SegmentMeshes.Num();
	UpdateStaffCollision();

	if (bRecordTelemetry)
	{
		if (UWorld* World = GetWorld())
		{
			if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
			{
				GameMode->RecordTelemetryStaffSegmentGained(Cast<AWizardStaffWizardCharacter>(Owner));
			}
		}
	}

	return SegmentCount;
}

bool UWizardStaffComponent::RemoveTopStaffSegment(bool bSpawnPhysicsSegment)
{
	if (SegmentMeshes.Num() <= 0 || SegmentAnchors.Num() <= 0)
	{
		SegmentCount = 0;
		StaffStress = 0.0f;
		UpdateStaffCollision();
		NotifyOwnerSegmentCountChanged();
		NotifyOwnerStaffStressChanged(true);
		return false;
	}

	const FTransform RemovedSegmentTransform = SegmentMeshes.Last()->GetComponentTransform();
	if (bSpawnPhysicsSegment)
	{
		SpawnSnappedPhysicsSegment(RemovedSegmentTransform);
	}

	RemoveTopSegmentComponents();
	SegmentCount = SegmentMeshes.Num();
	if (SegmentCount <= 0)
	{
		StaffStress = 0.0f;
	}
	UpdateStaffCollision();
	NotifyOwnerSegmentCountChanged();
	NotifyOwnerStaffStressChanged(true);

	return true;
}

void UWizardStaffComponent::ClearStaffSegments()
{
	for (UStaticMeshComponent* SegmentMesh : SegmentMeshes)
	{
		if (SegmentMesh)
		{
			SegmentMesh->DestroyComponent();
		}
	}

	for (USceneComponent* SegmentAnchor : SegmentAnchors)
	{
		if (SegmentAnchor)
		{
			SegmentAnchor->DestroyComponent();
		}
	}

	SegmentMeshes.Reset();
	SegmentAnchors.Reset();
	SegmentCount = 0;
	StaffStress = 0.0f;
	UpdateStaffCollision();
	NotifyOwnerSegmentCountChanged();
	NotifyOwnerStaffStressChanged(true);
}

void UWizardStaffComponent::ResetForNewMatch(bool bResetSegments)
{
	if (bResetSegments)
	{
		ClearStaffSegments();
	}
	else
	{
		StaffStress = 0.0f;
		UpdateStaffCollision();
		NotifyOwnerStaffStressChanged(true);
	}

	bIsStaffObstructed = false;
	bWasStaffObstructed = false;
	StaffObstructionAlpha = 0.0f;
	bIsStaffStuck = false;
	StaffStuckTime = 0.0f;
	StaffCollisionReliefRemaining = 0.0f;
	RecentOwnerMoveInputTime = 0.0f;
	StuckReliefCooldownRemaining = 0.0f;
	bHasLastCollisionLocation = false;
	bHasLastOwnerLocation = false;
	StressRattlePhase = 0.0f;

	if (BaseAnchor)
	{
		BaseAnchor->SetRelativeRotation(FRotator::ZeroRotator);
	}
	UpdateStaffCollision();
	NotifyOwnerSegmentCountChanged();
	NotifyOwnerStaffStressChanged(true);
}

void UWizardStaffComponent::NotifyOwnerMovementInput(float InputAmount)
{
	if (InputAmount >= StuckTuning.MovementInputThreshold)
	{
		RecentOwnerMoveInputTime = FMath::Max(RecentOwnerMoveInputTime, StuckTuning.MovementInputGraceTime);
	}
}

bool UWizardStaffComponent::AddStaffStress(float BaseStressAmount, FName StressSource)
{
	if (BaseStressAmount <= 0.0f || SegmentCount <= 0)
	{
		return false;
	}

	const AWizardStaffWizardCharacter* WizardOwner = Cast<AWizardStaffWizardCharacter>(GetOwner());
	const float EffectStressMultiplier = WizardOwner ? WizardOwner->GetStaffStressEffectMultiplier() : 1.0f;
	const float StressToAdd = BaseStressAmount * GetStressMultiplier() * FMath::Max(EffectStressMultiplier, 0.0f);
	StaffStress = FMath::Clamp(StaffStress + StressToAdd, 0.0f, FMath::Max(StressTuning.MaxStaffStress, 1.0f));
	NotifyOwnerStaffStressChanged();

	UE_LOG(LogTemp, Log, TEXT("Staff stress +%.1f from %s. Stress %.1f/%.1f, segments %d."),
		StressToAdd,
		*StressSource.ToString(),
		StaffStress,
		StressTuning.MaxStaffStress,
		SegmentCount);

	if (StaffStress >= StressTuning.MaxStaffStress)
	{
		return SnapTopStaffSegment();
	}

	return false;
}

bool UWizardStaffComponent::SnapTopStaffSegment()
{
	if (SegmentMeshes.Num() <= 0 || SegmentAnchors.Num() <= 0)
	{
		StaffStress = 0.0f;
		NotifyOwnerStaffStressChanged(true);
		return false;
	}

	RemoveTopStaffSegment(true);
	const float SafeMaxStress = FMath::Max(StressTuning.MaxStaffStress, 1.0f);
	StaffStress = SegmentCount > 0 ? FMath::Clamp(SafeMaxStress * StressTuning.StressAfterSnapRatio, 0.0f, SafeMaxStress - 1.0f) : 0.0f;
	NotifyOwnerStaffStressChanged(true);

	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RecordTelemetryStaffSegmentSnapped(Cast<AWizardStaffWizardCharacter>(GetOwner()));
		}
	}

	OnStaffSegmentSnapped.Broadcast(SegmentCount, StaffStress);

	AWizardStaffHUD::PushGameplayMessage(this, TEXT("STAFF SNAP! Top segment broke off."), FColor::Red, 2.0f, EWizardHudMessageCategory::Gameplay);
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()) + 3100ULL, 1.25f, FColor::Red, TEXT("STAFF SNAP! Top segment broke off."));
	}

	UE_LOG(LogTemp, Warning, TEXT("Staff segment snapped. Remaining segments %d, stress %.1f/%.1f."),
		SegmentCount,
		StaffStress,
		StressTuning.MaxStaffStress);

	return true;
}

float UWizardStaffComponent::GetStaffStressAlpha() const
{
	return FMath::Clamp(StaffStress / FMath::Max(StressTuning.MaxStaffStress, 1.0f), 0.0f, 1.0f);
}

float UWizardStaffComponent::GetStressMultiplier() const
{
	return 1.0f + (static_cast<float>(SegmentCount) * StressTuning.StressMultiplierPerSegment);
}

float UWizardStaffComponent::GetStaffCollisionLength() const
{
	return CollisionTuning.BaseCollisionLength + (static_cast<float>(SegmentCount) * CollisionTuning.CollisionLengthPerSegment);
}

float UWizardStaffComponent::GetControlInputMultiplier() const
{
	if (!CollisionTuning.bStaffCollisionEnabled)
	{
		return 1.0f;
	}

	const float ObstructedMultiplier = StaffCollisionReliefRemaining > 0.0f
		? StuckTuning.CollisionReliefControlMultiplier
		: CollisionTuning.ObstructedControlMultiplier;
	const float TargetMultiplier = FMath::Lerp(1.0f, ObstructedMultiplier, StaffObstructionAlpha);
	return FMath::Clamp(TargetMultiplier, 0.0f, 1.0f);
}

void UWizardStaffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AActor* Owner = GetOwner();
	if (Owner && !Owner->HasAuthority())
	{
		UpdateStaffCollision();
		return;
	}

	UpdateStuckTimers(DeltaTime);
	UpdateStaffCollision();
	RefreshStaffObstruction(DeltaTime);
	UpdateStressDecay(DeltaTime);
	UpdateStressVisuals(DeltaTime);
	DrawStressDebug();
}

USceneComponent* UWizardStaffComponent::CreateRuntimeAnchor(FName Name, USceneComponent* AttachParent, const FTransform& RelativeTransform, FName AttachSocketName)
{
	AActor* Owner = GetOwner();
	if (!Owner || !AttachParent)
	{
		return nullptr;
	}

	const FName UniqueName = MakeUniqueObjectName(Owner, USceneComponent::StaticClass(), Name);
	USceneComponent* Anchor = NewObject<USceneComponent>(Owner, UniqueName);
	if (!Anchor)
	{
		return nullptr;
	}

	Anchor->SetMobility(EComponentMobility::Movable);
	Anchor->SetAbsolute(false, false, true);
	Anchor->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform, AttachSocketName);
	Anchor->SetRelativeTransform(RelativeTransform);
	Owner->AddInstanceComponent(Anchor);
	Anchor->RegisterComponent();

	return Anchor;
}

UStaticMeshComponent* UWizardStaffComponent::CreateRuntimeMesh(FName Name, USceneComponent* AttachParent, UStaticMesh* MeshAsset, const FVector& RelativeLocation, const FVector& VisualScale, const FLinearColor& Color)
{
	AActor* Owner = GetOwner();
	if (!Owner || !AttachParent || !MeshAsset)
	{
		return nullptr;
	}

	const FName UniqueName = MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(), Name);
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(Owner, UniqueName);
	if (!MeshComponent)
	{
		return nullptr;
	}

	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(MeshAsset);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetRelativeLocation(RelativeLocation);
	MeshComponent->SetRelativeScale3D(VisualScale);

	if (StaffMaterial)
	{
		MeshComponent->SetMaterial(0, StaffMaterial);
	}

	MeshComponent->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
	Owner->AddInstanceComponent(MeshComponent);
	MeshComponent->RegisterComponent();

	if (MeshComponent->DoesSocketExist(BottomSocketName))
	{
		const FTransform BottomSocketTransform = MeshComponent->GetSocketTransform(BottomSocketName, RTS_Component);
		MeshComponent->SetRelativeTransform(BottomSocketTransform.Inverse());
	}

	ApplyVisualColor(MeshComponent, Color);

	return MeshComponent;
}

FTransform UWizardStaffComponent::GetFallbackSegmentAnchorTransform() const
{
	const float PreviousPieceHeight = SegmentAnchors.Num() > 0 ? VisualTuning.SegmentFallbackHeight : VisualTuning.BaseStaffFallbackHeight;
	return FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, PreviousPieceHeight + VisualTuning.SegmentSpacing), FVector::OneVector);
}

void UWizardStaffComponent::ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const
{
	if (!MeshComponent)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynamicMaterial)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

void UWizardStaffComponent::CreateStaffCollision()
{
	if (StaffCollisionBox || !BaseAnchor)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	StaffCollisionBox = NewObject<UBoxComponent>(Owner, TEXT("StaffCollisionBox"));
	if (!StaffCollisionBox)
	{
		return;
	}

	StaffCollisionBox->SetMobility(EComponentMobility::Movable);
	StaffCollisionBox->AttachToComponent(BaseAnchor, FAttachmentTransformRules::KeepRelativeTransform);
	StaffCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	StaffCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaffCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	StaffCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	StaffCollisionBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	StaffCollisionBox->SetGenerateOverlapEvents(false);
	StaffCollisionBox->SetHiddenInGame(true);
	StaffCollisionBox->SetCanEverAffectNavigation(false);
	Owner->AddInstanceComponent(StaffCollisionBox);
	StaffCollisionBox->RegisterComponent();
}

void UWizardStaffComponent::UpdateStaffCollision()
{
	if (!StaffCollisionBox)
	{
		return;
	}

	const float CollisionLength = FMath::Max(GetStaffCollisionLength(), 1.0f);
	const float HalfLength = CollisionLength * 0.5f;
	const float HalfThickness = FMath::Max(CollisionTuning.CollisionThickness, 1.0f);

	StaffCollisionBox->SetBoxExtent(FVector(HalfThickness, HalfThickness, HalfLength), true);
	StaffCollisionBox->SetRelativeLocation(CollisionTuning.CollisionLocalOffset + FVector(0.0f, 0.0f, HalfLength));
	const AActor* Owner = GetOwner();
	const bool bOwnerHasAuthority = !Owner || Owner->HasAuthority();
	const bool bUseStaffCollision = bOwnerHasAuthority && CollisionTuning.bStaffCollisionEnabled && StaffCollisionReliefRemaining <= 0.0f;
	StaffCollisionBox->SetCollisionEnabled(bUseStaffCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void UWizardStaffComponent::RefreshStaffObstruction(float DeltaTime)
{
	if (!CollisionTuning.bStaffCollisionEnabled || !StaffCollisionBox || StaffCollisionReliefRemaining > 0.0f)
	{
		bIsStaffObstructed = false;
		bWasStaffObstructed = false;
		StaffObstructionAlpha = FMath::FInterpTo(StaffObstructionAlpha, 0.0f, DeltaTime, CollisionTuning.ObstructionRecoverySpeed);
		ResetStuckState();
		return;
	}

	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner)
	{
		return;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WizardStaffCollision), false, Owner);
	QueryParams.AddIgnoredActor(Owner);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(
		Overlaps,
		StaffCollisionBox->GetComponentLocation(),
		StaffCollisionBox->GetComponentQuat(),
		ObjectQueryParams,
		FCollisionShape::MakeBox(StaffCollisionBox->GetScaledBoxExtent()),
		QueryParams);

	bool bHasBlockingObstacle = false;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		const UPrimitiveComponent* OverlapComponent = Overlap.GetComponent();
		if (OverlapComponent && OverlapComponent->GetCollisionResponseToChannel(ECC_WorldDynamic) == ECR_Block)
		{
			bHasBlockingObstacle = true;
			break;
		}
	}

	bIsStaffObstructed = bHasBlockingObstacle;

	const FVector CurrentOwnerLocation = Owner->GetActorLocation();
	float OwnerMoveSpeed = 0.0f;
	if (bHasLastOwnerLocation && DeltaTime > SMALL_NUMBER)
	{
		OwnerMoveSpeed = FVector::Dist2D(CurrentOwnerLocation, LastOwnerLocation) / DeltaTime;
	}
	LastOwnerLocation = CurrentOwnerLocation;
	bHasLastOwnerLocation = true;

	const FVector CurrentCollisionLocation = StaffCollisionBox->GetComponentLocation();
	float CollisionSpeed = 0.0f;
	if (bHasLastCollisionLocation && DeltaTime > SMALL_NUMBER)
	{
		CollisionSpeed = FVector::Dist(CurrentCollisionLocation, LastCollisionLocation) / DeltaTime;
	}
	LastCollisionLocation = CurrentCollisionLocation;
	bHasLastCollisionLocation = true;

	const float ImpactAlpha = FMath::Clamp(CollisionSpeed / FMath::Max(StressTuning.WallImpactSpeedForFullStress, 1.0f), 0.0f, 1.0f);
	if (bHasBlockingObstacle && !bWasStaffObstructed)
	{
		AddStaffStress(StressTuning.StressGainedPerWallImpact * FMath::Max(ImpactAlpha, 0.25f), TEXT("WallImpact"));
	}
	else if (bHasBlockingObstacle && StressTuning.CaughtStressPerSecond > 0.0f)
	{
		AddStaffStress(StressTuning.CaughtStressPerSecond * FMath::Max(ImpactAlpha, 0.15f) * DeltaTime, TEXT("CaughtOnProp"));
	}

	bWasStaffObstructed = bHasBlockingObstacle;

	const float TargetObstruction = bHasBlockingObstacle ? 1.0f : 0.0f;
	StaffObstructionAlpha = FMath::FInterpTo(StaffObstructionAlpha, TargetObstruction, DeltaTime, CollisionTuning.ObstructionRecoverySpeed);

	UpdateStuckFailsafe(DeltaTime, bHasBlockingObstacle, OwnerMoveSpeed);
}

void UWizardStaffComponent::UpdateStuckTimers(float DeltaTime)
{
	RecentOwnerMoveInputTime = FMath::Max(0.0f, RecentOwnerMoveInputTime - DeltaTime);
	StaffCollisionReliefRemaining = FMath::Max(0.0f, StaffCollisionReliefRemaining - DeltaTime);
	StuckReliefCooldownRemaining = FMath::Max(0.0f, StuckReliefCooldownRemaining - DeltaTime);
}

void UWizardStaffComponent::UpdateStuckFailsafe(float DeltaTime, bool bHasBlockingObstacle, float OwnerMoveSpeed)
{
	if (!StuckTuning.bEnableStuckFailsafe || SegmentCount <= 0)
	{
		ResetStuckState();
		return;
	}

	const bool bOwnerTryingToMove = RecentOwnerMoveInputTime > 0.0f;
	const bool bOwnerBarelyMoving = OwnerMoveSpeed <= StuckTuning.OwnerMoveSpeedThreshold;
	if (bHasBlockingObstacle && bOwnerTryingToMove && bOwnerBarelyMoving)
	{
		StaffStuckTime += DeltaTime;
	}
	else
	{
		StaffStuckTime = 0.0f;
		bIsStaffStuck = false;
		return;
	}

	bIsStaffStuck = StaffStuckTime >= StuckTuning.StuckTimeBeforeStressBoost;
	if (!bIsStaffStuck)
	{
		return;
	}

	const bool bDidSnap = AddStaffStress(StuckTuning.StuckStressPerSecond * DeltaTime, TEXT("StuckFailsafe"));
	if (bDidSnap || SegmentCount <= 0)
	{
		ResetStuckState();
		return;
	}

	if (StaffStuckTime >= StuckTuning.StuckTimeBeforeCollisionRelief && StuckReliefCooldownRemaining <= 0.0f)
	{
		TriggerCollisionRelief();
	}
}

void UWizardStaffComponent::ResetStuckState()
{
	bIsStaffStuck = false;
	StaffStuckTime = 0.0f;
}

void UWizardStaffComponent::TriggerCollisionRelief()
{
	StaffCollisionReliefRemaining = FMath::Max(StaffCollisionReliefRemaining, StuckTuning.CollisionReliefDuration);
	StuckReliefCooldownRemaining = StuckTuning.CollisionReliefCooldown;
	ResetStuckState();
	ApplyGentleOwnerNudge();
	UpdateStaffCollision();

	if (GEngine && StuckTuning.bShowStuckDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()) + 3200ULL, 0.9f, FColor::Orange, TEXT("Staff wedged: easing collision briefly."));
	}
}

void UWizardStaffComponent::ApplyGentleOwnerNudge()
{
	AActor* Owner = GetOwner();
	if (!Owner || !StaffCollisionBox || StuckTuning.GentleNudgeDistance <= 0.0f)
	{
		return;
	}

	FVector NudgeDirection = Owner->GetActorLocation() - StaffCollisionBox->GetComponentLocation();
	NudgeDirection.Z = 0.0f;
	if (!NudgeDirection.Normalize())
	{
		NudgeDirection = -Owner->GetActorForwardVector();
		NudgeDirection.Z = 0.0f;
		NudgeDirection.Normalize();
	}

	Owner->AddActorWorldOffset(NudgeDirection * StuckTuning.GentleNudgeDistance, true);
}

void UWizardStaffComponent::UpdateStressDecay(float DeltaTime)
{
	if (StaffStress <= 0.0f || StressTuning.StressDecayRate <= 0.0f)
	{
		return;
	}

	const float PreviousStress = StaffStress;
	StaffStress = FMath::Max(0.0f, StaffStress - (StressTuning.StressDecayRate * DeltaTime));
	if (!FMath::IsNearlyEqual(PreviousStress, StaffStress, KINDA_SMALL_NUMBER))
	{
		NotifyOwnerStaffStressChanged();
	}
}

void UWizardStaffComponent::UpdateStressVisuals(float DeltaTime)
{
	if (!BaseAnchor)
	{
		return;
	}

	StressRattlePhase += DeltaTime * 24.0f;
	const float StressAlpha = GetStaffStressAlpha();
	const float RattleAmount = FMath::Sin(StressRattlePhase) * StressAlpha * 2.5f;
	const float PitchAmount = FMath::Cos(StressRattlePhase * 0.67f) * StressAlpha * 1.5f;
	BaseAnchor->SetRelativeRotation(FRotator(PitchAmount, 0.0f, RattleAmount));
}

void UWizardStaffComponent::SpawnSnappedPhysicsSegment(const FTransform& SegmentTransform)
{
	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner || !SegmentMeshAsset)
	{
		return;
	}

	AStaticMeshActor* SnappedSegment = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SegmentTransform);
	if (!SnappedSegment)
	{
		return;
	}
	SnappedSegment->Tags.AddUnique(AWizardStaffGameMode::GetLooseSnappedSegmentTag());

	UStaticMeshComponent* MeshComponent = SnappedSegment->GetStaticMeshComponent();
	if (!MeshComponent)
	{
		SnappedSegment->Destroy();
		return;
	}

	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(SegmentMeshAsset);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetMassOverrideInKg(NAME_None, 2.5f, true);
	MeshComponent->SetLinearDamping(FMath::Max(StressTuning.SnappedSegmentLinearDamping, 0.0f));
	MeshComponent->SetAngularDamping(FMath::Max(StressTuning.SnappedSegmentAngularDamping, 0.0f));

	if (StaffMaterial)
	{
		MeshComponent->SetMaterial(0, StaffMaterial);
	}
	ApplyVisualColor(MeshComponent, VisualTuning.SegmentColor);

	FVector ImpulseDirection = Owner->GetActorForwardVector() + (Owner->GetActorRightVector() * 0.35f);
	ImpulseDirection.Z = 0.35f;
	ImpulseDirection.Normalize();

	const FVector Impulse = (ImpulseDirection * StressTuning.SnapImpulseForce) + FVector(0.0f, 0.0f, StressTuning.SnapUpwardImpulse);
	MeshComponent->AddImpulse(Impulse, NAME_None, StressTuning.bSnapImpulseIgnoresSegmentMass);

	if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
	{
		GameMode->RegisterLooseSnappedSegmentFromWizard(SnappedSegment, Cast<AWizardStaffWizardCharacter>(Owner));
	}

	if (StressTuning.bShowStressDebug)
	{
		DrawDebugSphere(World, SegmentTransform.GetLocation(), 55.0f, 16, FColor::Red, false, 1.0f, 0, 3.0f);
	}
}

void UWizardStaffComponent::RemoveTopSegmentComponents()
{
	UStaticMeshComponent* TopMesh = SegmentMeshes.Num() > 0 ? SegmentMeshes.Last().Get() : nullptr;
	USceneComponent* TopAnchor = SegmentAnchors.Num() > 0 ? SegmentAnchors.Last().Get() : nullptr;

	if (SegmentMeshes.Num() > 0)
	{
		SegmentMeshes.Pop(EAllowShrinking::No);
	}
	if (SegmentAnchors.Num() > 0)
	{
		SegmentAnchors.Pop(EAllowShrinking::No);
	}

	if (TopMesh)
	{
		TopMesh->DestroyComponent();
	}
	if (TopAnchor)
	{
		TopAnchor->DestroyComponent();
	}
}

void UWizardStaffComponent::NotifyOwnerSegmentCountChanged() const
{
	if (AWizardStaffWizardCharacter* WizardOwner = Cast<AWizardStaffWizardCharacter>(GetOwner()))
	{
		WizardOwner->SyncReplicatedStaffSegmentCountFromAuthority();
	}
}

void UWizardStaffComponent::NotifyOwnerStaffStressChanged(bool bForce) const
{
	if (AWizardStaffWizardCharacter* WizardOwner = Cast<AWizardStaffWizardCharacter>(GetOwner()))
	{
		WizardOwner->SyncReplicatedStaffStressFromAuthority(bForce);
	}
}

void UWizardStaffComponent::DrawStressDebug() const
{
	if ((!StressTuning.bShowStressDebug && !StuckTuning.bShowStuckDebug) || !GEngine || SegmentCount <= 0 || !AWizardStaffHUD::IsFullDebugMode(this))
	{
		return;
	}

	FString DebugText = FString::Printf(
		TEXT("Staff Stress %.0f/%.0f | Segments %d"),
		StaffStress,
		StressTuning.MaxStaffStress,
		SegmentCount);

	if (StuckTuning.bShowStuckDebug && (bIsStaffStuck || StaffStuckTime > 0.0f || StaffCollisionReliefRemaining > 0.0f))
	{
		DebugText += FString::Printf(
			TEXT(" | Stuck %.1fs | Relief %.1fs"),
			StaffStuckTime,
			StaffCollisionReliefRemaining);
	}

	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()) + 3000ULL, 0.0f, FColor::Red, DebugText);
}
