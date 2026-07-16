#include "WizardStaffCauldronArena.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AWizardStaffCauldronArena::AWizardStaffCauldronArena()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	auto ConfigureMesh = [&](UStaticMeshComponent* Mesh, const FVector& Location, const FVector& Scale)
	{
		Mesh->SetupAttachment(SceneRoot);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetRelativeLocation(Location);
		Mesh->SetRelativeScale3D(Scale);
		if (CylinderMesh.Succeeded()) Mesh->SetStaticMesh(CylinderMesh.Object);
		if (BasicMaterial.Succeeded()) Mesh->SetMaterial(0, BasicMaterial.Object);
	};

	CauldronBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CauldronBody"));
	ConfigureMesh(CauldronBody, FVector(0.0f, 0.0f, 57.0f), FVector(3.6f, 3.6f, 1.16f));
	CauldronRim = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CauldronRim"));
	ConfigureMesh(CauldronRim, FVector(0.0f, 0.0f, 119.0f), FVector(4.01f, 4.01f, 0.14f));
	BrewSurface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BrewSurface"));
	ConfigureMesh(BrewSurface, FVector(0.0f, 0.0f, 127.0f), FVector(3.41f, 3.41f, 0.06f));

	for (int32 BubbleIndex = 0; BubbleIndex < 5; ++BubbleIndex)
	{
		UStaticMeshComponent* BubbleMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("CurseWarningBubble_%02d"), BubbleIndex));
		BubbleMesh->SetupAttachment(SceneRoot);
		BubbleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BubbleMesh->SetGenerateOverlapEvents(false);
		BubbleMesh->SetHiddenInGame(true);
		BubbleMesh->SetVisibility(false);
		if (SphereMesh.Succeeded()) BubbleMesh->SetStaticMesh(SphereMesh.Object);
		if (BasicMaterial.Succeeded()) BubbleMesh->SetMaterial(0, BasicMaterial.Object);
		BubbleMeshes.Add(BubbleMesh);
	}

	for (int32 IntakeIndex = 0; IntakeIndex < 4; ++IntakeIndex)
	{
		UStaticMeshComponent* IntakeMarker = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("CauldronIntake_%02d"), IntakeIndex));
		IntakeMarker->SetupAttachment(SceneRoot);
		IntakeMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IntakeMarker->SetGenerateOverlapEvents(false);
		if (CubeMesh.Succeeded()) IntakeMarker->SetStaticMesh(CubeMesh.Object);
		if (BasicMaterial.Succeeded()) IntakeMarker->SetMaterial(0, BasicMaterial.Object);
		IntakeMarkerMeshes.Add(IntakeMarker);
		IntakeMarkerMaterials.Add(nullptr);
	}

	ArenaFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArenaFloor"));
	ArenaFloor->SetupAttachment(SceneRoot);
	ArenaFloor->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
	ArenaFloor->SetRelativeScale3D(FVector(32.0f, 32.0f, 0.08f));
	ArenaFloor->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	ArenaFloor->SetGenerateOverlapEvents(false);
	if (CylinderMesh.Succeeded()) ArenaFloor->SetStaticMesh(CylinderMesh.Object);
	if (BasicMaterial.Succeeded()) ArenaFloor->SetMaterial(0, BasicMaterial.Object);

	constexpr int32 EdgeCount = 8;
	constexpr float EdgeApothem = 1527.27f;
	constexpr float EdgeScaleLength = 12.65f;
	for (int32 EdgeIndex = 0; EdgeIndex < EdgeCount; ++EdgeIndex)
	{
		UStaticMeshComponent* EdgeMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("OctagonEdge_%02d"), EdgeIndex));
		EdgeMesh->SetupAttachment(SceneRoot);
		EdgeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EdgeMesh->SetGenerateOverlapEvents(false);
		const float AngleRadians = (2.0f * UE_PI * static_cast<float>(EdgeIndex)) / static_cast<float>(EdgeCount);
		EdgeMesh->SetRelativeLocation(FVector(FMath::Cos(AngleRadians) * EdgeApothem, FMath::Sin(AngleRadians) * EdgeApothem, 5.0f));
		EdgeMesh->SetRelativeRotation(FRotator(0.0f, FMath::RadiansToDegrees(AngleRadians) + 90.0f, 0.0f));
		EdgeMesh->SetRelativeScale3D(FVector(EdgeScaleLength, 0.055f, 0.045f));
		if (CubeMesh.Succeeded()) EdgeMesh->SetStaticMesh(CubeMesh.Object);
		if (BasicMaterial.Succeeded()) EdgeMesh->SetMaterial(0, BasicMaterial.Object);
		OctagonEdgeMeshes.Add(EdgeMesh);
	}
}

void AWizardStaffCauldronArena::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWizardStaffCauldronArena, ReplicatedDepositPulseSequence);
	DOREPLIFETIME(AWizardStaffCauldronArena, bReplicatedCurseWarningActive);
	DOREPLIFETIME(AWizardStaffCauldronArena, bReplicatedCurseDepositWarningActive);
	DOREPLIFETIME(AWizardStaffCauldronArena, ReplicatedActiveIntake);
	DOREPLIFETIME(AWizardStaffCauldronArena, ReplicatedPreviewIntake);
	DOREPLIFETIME(AWizardStaffCauldronArena, bReplicatedIntakeRelocating);
	DOREPLIFETIME(AWizardStaffCauldronArena, ReplicatedIntakeDistanceFromCenter);
	DOREPLIFETIME(AWizardStaffCauldronArena, ReplicatedIntakeVisualScale);
	DOREPLIFETIME(AWizardStaffCauldronArena, ReplicatedIntakePulseSpeed);
	DOREPLIFETIME(AWizardStaffCauldronArena, bReplicatedIntakeOccupied);
	DOREPLIFETIME(AWizardStaffCauldronArena, ReplicatedIntakeBankingProgressAlpha);
}

void AWizardStaffCauldronArena::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	DepositPulseTimeRemaining = FMath::Max(DepositPulseTimeRemaining - DeltaSeconds, 0.0f);
	const float PulseAlpha = DepositPulseTimeRemaining / 0.42f;
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float ViolentBoilScale = bReplicatedCurseDepositWarningActive ? 1.0f + (FMath::Sin(Time * 21.0f) * 0.12f) : 1.0f;
	const float DepositScale = (1.0f + (FMath::Sin((1.0f - PulseAlpha) * UE_PI) * 0.18f * PulseAlpha)) * ViolentBoilScale;
	CauldronBody->SetRelativeScale3D(FVector(3.6f * DepositScale, 3.6f * DepositScale, 1.16f * (1.0f - (0.10f * PulseAlpha))));
	CauldronRim->SetRelativeScale3D(FVector(4.01f * DepositScale, 4.01f * DepositScale, 0.14f));
	BrewSurface->SetRelativeScale3D(FVector(3.41f * DepositScale, 3.41f * DepositScale, 0.06f));
	ApplyIntakePresentation();

	if (!bReplicatedCurseWarningActive && !bReplicatedCurseDepositWarningActive)
	{
		return;
	}

	const float BoilRateMultiplier = bReplicatedCurseDepositWarningActive ? 2.9f : 1.0f;
	for (int32 BubbleIndex = 0; BubbleIndex < BubbleMeshes.Num(); ++BubbleIndex)
	{
		UStaticMeshComponent* Bubble = BubbleMeshes[BubbleIndex];
		if (!Bubble)
		{
			continue;
		}
		const float Phase = Time * (2.0f + (BubbleIndex * 0.35f)) * BoilRateMultiplier + (BubbleIndex * 1.71f);
		const float Radius = 55.0f + (BubbleIndex % 3) * 42.0f;
		Bubble->SetRelativeLocation(FVector(FMath::Cos(Phase) * Radius, FMath::Sin(Phase) * Radius, 138.0f + FMath::Sin(Phase * 1.8f) * 42.0f));
		const float Scale = 0.13f + (FMath::Sin(Phase * 2.1f) + 1.0f) * 0.07f;
		Bubble->SetRelativeScale3D(FVector(Scale));
	}
}

FVector AWizardStaffCauldronArena::GetIntakeWorldLocation(EWizardCauldronIntakeDirection IntakeDirection) const
{
	return GetActorLocation() + GetActorTransform().TransformVectorNoScale(GetIntakeLocalDirection(IntakeDirection) * ReplicatedIntakeDistanceFromCenter) + FVector(0.0f, 0.0f, 124.0f);
}

void AWizardStaffCauldronArena::ConfigureIntakePresentation(float NewDistanceFromCenter, float NewVisualScale, float NewPulseSpeed)
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedIntakeDistanceFromCenter = FMath::Max(NewDistanceFromCenter, 100.0f);
	ReplicatedIntakeVisualScale = FMath::Max(NewVisualScale, 0.1f);
	ReplicatedIntakePulseSpeed = FMath::Max(NewPulseSpeed, 0.1f);
	ApplyIntakePresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronArena::SetIntakeState(EWizardCauldronIntakeDirection NewActiveIntake, EWizardCauldronIntakeDirection NewPreviewIntake, bool bNewRelocating)
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedActiveIntake = NewActiveIntake;
	ReplicatedPreviewIntake = NewPreviewIntake;
	bReplicatedIntakeRelocating = bNewRelocating;
	ApplyIntakePresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronArena::SetIntakeBankingState(bool bNewOccupied, float NewProgressAlpha)
{
	if (!HasAuthority())
	{
		return;
	}

	const float QuantizedProgressAlpha = bNewOccupied
		? FMath::GridSnap(FMath::Clamp(NewProgressAlpha, 0.0f, 1.0f), 0.05f)
		: 0.0f;
	if (bReplicatedIntakeOccupied == bNewOccupied && FMath::IsNearlyEqual(ReplicatedIntakeBankingProgressAlpha, QuantizedProgressAlpha))
	{
		return;
	}

	bReplicatedIntakeOccupied = bNewOccupied;
	ReplicatedIntakeBankingProgressAlpha = QuantizedProgressAlpha;
	ApplyIntakePresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronArena::TriggerDepositPulse()
{
	if (!HasAuthority())
	{
		return;
	}
	DepositPulseTimeRemaining = 0.42f;
	++ReplicatedDepositPulseSequence;
	ForceNetUpdate();
}

void AWizardStaffCauldronArena::SetCurseWarningActive(bool bNewWarningActive)
{
	if (!HasAuthority() || bReplicatedCurseWarningActive == bNewWarningActive)
	{
		return;
	}
	bReplicatedCurseWarningActive = bNewWarningActive;
	ApplyCurseWarningPresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronArena::SetCurseDepositWarningActive(bool bNewWarningActive)
{
	if (!HasAuthority() || bReplicatedCurseDepositWarningActive == bNewWarningActive)
	{
		return;
	}
	bReplicatedCurseDepositWarningActive = bNewWarningActive;
	ApplyCurseWarningPresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronArena::OnRep_DepositPulseSequence()
{
	DepositPulseTimeRemaining = 0.42f;
}

void AWizardStaffCauldronArena::OnRep_CurseWarningActive()
{
	ApplyCurseWarningPresentation();
}

void AWizardStaffCauldronArena::OnRep_CurseDepositWarningActive()
{
	ApplyCurseWarningPresentation();
}

void AWizardStaffCauldronArena::OnRep_IntakeState()
{
	ApplyIntakePresentation();
}

void AWizardStaffCauldronArena::ApplyCurseWarningPresentation()
{
	const bool bShowWarning = bReplicatedCurseWarningActive || bReplicatedCurseDepositWarningActive;
	for (UStaticMeshComponent* Bubble : BubbleMeshes)
	{
		if (Bubble)
		{
			Bubble->SetHiddenInGame(!bShowWarning);
			Bubble->SetVisibility(bShowWarning, true);
			if (UMaterialInstanceDynamic* Material = Bubble->CreateAndSetMaterialInstanceDynamic(0))
			{
				Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.88f, 0.10f, 0.04f));
				Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.88f, 0.10f, 0.04f));
			}
		}
	}
}

void AWizardStaffCauldronArena::ApplyIntakePresentation()
{
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (int32 IntakeIndex = 0; IntakeIndex < IntakeMarkerMeshes.Num(); ++IntakeIndex)
	{
		UStaticMeshComponent* IntakeMarker = IntakeMarkerMeshes[IntakeIndex];
		if (!IntakeMarker)
		{
			continue;
		}

		const EWizardCauldronIntakeDirection Direction = static_cast<EWizardCauldronIntakeDirection>(IntakeIndex + 1);
		const bool bIsActive = Direction == ReplicatedActiveIntake && !bReplicatedIntakeRelocating;
		const bool bIsPreview = Direction == ReplicatedPreviewIntake && bReplicatedIntakeRelocating;
		const float Pulse = bIsActive
			? 1.0f + (FMath::Sin(Time * ReplicatedIntakePulseSpeed * (bReplicatedIntakeOccupied ? 1.8f : 1.0f)) * (bReplicatedIntakeOccupied ? 0.25f : 0.16f))
			: (bIsPreview ? 0.88f + (FMath::Sin(Time * ReplicatedIntakePulseSpeed * 1.6f) * 0.20f) : 0.72f);
		const float Scale = ReplicatedIntakeVisualScale * Pulse;
		const FVector DirectionVector = GetIntakeLocalDirection(Direction);
		IntakeMarker->SetRelativeLocation((DirectionVector * ReplicatedIntakeDistanceFromCenter) + FVector(0.0f, 0.0f, 124.0f));
		IntakeMarker->SetRelativeRotation(FRotator(0.0f, GetIntakeYaw(Direction), 0.0f));
		IntakeMarker->SetRelativeScale3D(FVector(1.25f * Scale, 0.24f * Scale, 0.16f * Scale));

		const FLinearColor Color = bIsActive
			? (bReplicatedIntakeOccupied ? FLinearColor(1.0f, 0.96f, 0.62f) : FLinearColor(1.0f, 0.82f, 0.06f))
			: (bIsPreview ? FLinearColor(1.0f, 0.50f, 0.08f) : FLinearColor(0.12f, 0.10f, 0.18f));
		UMaterialInstanceDynamic* Material = IntakeMarkerMaterials.IsValidIndex(IntakeIndex) ? IntakeMarkerMaterials[IntakeIndex] : nullptr;
		if (!Material)
		{
			Material = IntakeMarker->CreateAndSetMaterialInstanceDynamic(0);
			if (IntakeMarkerMaterials.IsValidIndex(IntakeIndex))
			{
				IntakeMarkerMaterials[IntakeIndex] = Material;
			}
		}
		if (Material)
		{
			Material->SetVectorParameterValue(TEXT("Color"), Color);
			Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
		}
	}
}

FVector AWizardStaffCauldronArena::GetIntakeLocalDirection(EWizardCauldronIntakeDirection IntakeDirection) const
{
	switch (IntakeDirection)
	{
	case EWizardCauldronIntakeDirection::North:
		return FVector::ForwardVector;
	case EWizardCauldronIntakeDirection::East:
		return FVector::RightVector;
	case EWizardCauldronIntakeDirection::South:
		return -FVector::ForwardVector;
	case EWizardCauldronIntakeDirection::West:
		return -FVector::RightVector;
	default:
		return FVector::ZeroVector;
	}
}

float AWizardStaffCauldronArena::GetIntakeYaw(EWizardCauldronIntakeDirection IntakeDirection) const
{
	const FVector Direction = GetIntakeLocalDirection(IntakeDirection);
	return Direction.IsNearlyZero() ? 0.0f : Direction.Rotation().Yaw;
}

void AWizardStaffCauldronArena::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInstanceDynamic* BodyMaterial = CauldronBody->CreateAndSetMaterialInstanceDynamic(0))
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.08f, 0.06f, 0.11f));
	}
	if (UMaterialInstanceDynamic* RimMaterial = CauldronRim->CreateAndSetMaterialInstanceDynamic(0))
	{
		RimMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.24f, 0.16f, 0.30f));
	}
	if (UMaterialInstanceDynamic* BrewMaterial = BrewSurface->CreateAndSetMaterialInstanceDynamic(0))
	{
		BrewMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.28f, 1.0f, 0.10f));
	}
	if (UMaterialInstanceDynamic* FloorMaterial = ArenaFloor->CreateAndSetMaterialInstanceDynamic(0))
	{
		FloorMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.06f, 0.09f, 0.12f));
	}
	for (UStaticMeshComponent* EdgeMesh : OctagonEdgeMeshes)
	{
		if (EdgeMesh)
		{
			if (UMaterialInstanceDynamic* EdgeMaterial = EdgeMesh->CreateAndSetMaterialInstanceDynamic(0))
			{
				EdgeMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.76f, 0.32f, 0.08f));
			}
		}
	}
	ApplyCurseWarningPresentation();
	ApplyIntakePresentation();
}

FVector AWizardStaffCauldronArena::GetAcceptanceCenter() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, 124.0f);
}

float AWizardStaffCauldronArena::GetFloorSurfaceZ() const
{
	return ArenaFloor ? ArenaFloor->Bounds.Origin.Z + ArenaFloor->Bounds.BoxExtent.Z : GetActorLocation().Z;
}
