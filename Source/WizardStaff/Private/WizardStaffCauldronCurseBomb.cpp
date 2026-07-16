#include "WizardStaffCauldronCurseBomb.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardStaffWizardCharacter.h"

AWizardStaffCauldronCurseBomb::AWizardStaffCauldronCurseBomb()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(SceneRoot);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetGenerateOverlapEvents(false);
	if (SphereMesh.Succeeded()) ProjectileMesh->SetStaticMesh(SphereMesh.Object);
	if (BasicMaterial.Succeeded()) ProjectileMesh->SetMaterial(0, BasicMaterial.Object);

	ExplosionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosionMesh"));
	ExplosionMesh->SetupAttachment(SceneRoot);
	ExplosionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ExplosionMesh->SetGenerateOverlapEvents(false);
	ExplosionMesh->SetHiddenInGame(true);
	ExplosionMesh->SetVisibility(false);
	if (SphereMesh.Succeeded()) ExplosionMesh->SetStaticMesh(SphereMesh.Object);
	if (BasicMaterial.Succeeded()) ExplosionMesh->SetMaterial(0, BasicMaterial.Object);

	constexpr int32 MarkerSegmentCount = 16;
	for (int32 SegmentIndex = 0; SegmentIndex < MarkerSegmentCount; ++SegmentIndex)
	{
		UStaticMeshComponent* MarkerSegment = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("BlastMarker_%02d"), SegmentIndex));
		MarkerSegment->SetupAttachment(SceneRoot);
		MarkerSegment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MarkerSegment->SetGenerateOverlapEvents(false);
		if (CubeMesh.Succeeded()) MarkerSegment->SetStaticMesh(CubeMesh.Object);
		if (BasicMaterial.Succeeded()) MarkerSegment->SetMaterial(0, BasicMaterial.Object);
		TargetMarkerSegments.Add(MarkerSegment);
	}
}

void AWizardStaffCauldronCurseBomb::BeginPlay()
{
	Super::BeginPlay();
	ApplyPresentation();
}

void AWizardStaffCauldronCurseBomb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ElapsedTime += FMath::Max(DeltaSeconds, 0.0f);

	const float SafeLaunchDelay = FMath::Max(LaunchDelay, 0.0f);
	const float SafeFlightDuration = FMath::Max(FlightDuration, 0.05f);
	const bool bLaunched = ElapsedTime >= SafeLaunchDelay;
	const float FlightAlpha = bLaunched ? FMath::Clamp((ElapsedTime - SafeLaunchDelay) / SafeFlightDuration, 0.0f, 1.0f) : 0.0f;
	const bool bHasExploded = FlightAlpha >= 1.0f;

	for (UStaticMeshComponent* MarkerSegment : TargetMarkerSegments)
	{
		if (MarkerSegment)
		{
			MarkerSegment->SetHiddenInGame(bHasExploded);
			MarkerSegment->SetVisibility(!bHasExploded, true);
		}
	}

	if (!bLaunched)
	{
		ProjectileMesh->SetHiddenInGame(true);
		ProjectileMesh->SetVisibility(false);
	}
	else if (!bHasExploded)
	{
		const FVector ArcLocation = FMath::Lerp(Origin, TargetLocation, FlightAlpha) + FVector(0.0f, 0.0f, FMath::Sin(FlightAlpha * UE_PI) * 260.0f);
		ProjectileMesh->SetWorldLocation(ArcLocation);
		ProjectileMesh->SetRelativeScale3D(FVector(0.24f + (FlightAlpha * 0.12f)));
		ProjectileMesh->SetHiddenInGame(false);
		ProjectileMesh->SetVisibility(true);
	}
	else
	{
		ProjectileMesh->SetHiddenInGame(true);
		ProjectileMesh->SetVisibility(false);
		if (!bExplosionApplied)
		{
			ApplyExplosion();
		}
		ExplosionVisualTimeRemaining = FMath::Max(ExplosionVisualTimeRemaining - DeltaSeconds, 0.0f);
		const float VisualAlpha = FMath::Clamp(ExplosionVisualTimeRemaining / 0.24f, 0.0f, 1.0f);
		ExplosionMesh->SetRelativeScale3D(FVector((BlastRadius / 50.0f) * (1.0f - VisualAlpha + 0.20f)));
		ExplosionMesh->SetHiddenInGame(ExplosionVisualTimeRemaining <= 0.0f);
		ExplosionMesh->SetVisibility(ExplosionVisualTimeRemaining > 0.0f, true);
		if (HasAuthority() && ExplosionVisualTimeRemaining <= 0.0f)
		{
			Destroy();
		}
	}
}

void AWizardStaffCauldronCurseBomb::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (UStaticMeshComponent* MarkerSegment : TargetMarkerSegments)
	{
		if (MarkerSegment)
		{
			MarkerSegment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AWizardStaffCauldronCurseBomb::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWizardStaffCauldronCurseBomb, Origin);
	DOREPLIFETIME(AWizardStaffCauldronCurseBomb, TargetLocation);
	DOREPLIFETIME(AWizardStaffCauldronCurseBomb, BlastRadius);
	DOREPLIFETIME(AWizardStaffCauldronCurseBomb, LaunchDelay);
	DOREPLIFETIME(AWizardStaffCauldronCurseBomb, FlightDuration);
	DOREPLIFETIME(AWizardStaffCauldronCurseBomb, HorizontalKnockback);
	DOREPLIFETIME(AWizardStaffCauldronCurseBomb, VerticalKnockback);
}

void AWizardStaffCauldronCurseBomb::InitializeBomb(const FVector& NewOrigin, const FVector& NewTargetLocation, float NewBlastRadius, float NewLaunchDelay, float NewFlightDuration, float NewHorizontalKnockback, float NewVerticalKnockback)
{
	if (!HasAuthority())
	{
		return;
	}

	Origin = NewOrigin;
	TargetLocation = NewTargetLocation;
	BlastRadius = FMath::Max(NewBlastRadius, 25.0f);
	LaunchDelay = FMath::Max(NewLaunchDelay, 0.0f);
	FlightDuration = FMath::Max(NewFlightDuration, 0.05f);
	HorizontalKnockback = FMath::Max(NewHorizontalKnockback, 0.0f);
	VerticalKnockback = FMath::Max(NewVerticalKnockback, 0.0f);
	SetActorLocation(TargetLocation);
	ApplyPresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronCurseBomb::OnRep_BombPresentation()
{
	ApplyPresentation();
}

void AWizardStaffCauldronCurseBomb::ApplyPresentation()
{
	if (!ProjectileMesh || !ExplosionMesh)
	{
		return;
	}

	const FLinearColor Red = FLinearColor(1.0f, 0.06f, 0.03f);
	for (UStaticMeshComponent* Mesh : { ProjectileMesh.Get(), ExplosionMesh.Get() })
	{
		if (Mesh)
		{
			if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
			{
				Material->SetVectorParameterValue(TEXT("Color"), Red);
				Material->SetVectorParameterValue(TEXT("BaseColor"), Red);
			}
		}
	}

	constexpr int32 MarkerSegmentCount = 16;
	const float Radius = FMath::Max(BlastRadius, 25.0f);
	const float ChordLength = 2.0f * Radius * FMath::Sin(UE_PI / static_cast<float>(MarkerSegmentCount));
	for (int32 SegmentIndex = 0; SegmentIndex < TargetMarkerSegments.Num(); ++SegmentIndex)
	{
		UStaticMeshComponent* MarkerSegment = TargetMarkerSegments[SegmentIndex];
		if (!MarkerSegment)
		{
			continue;
		}
		const float Angle = (2.0f * UE_PI * static_cast<float>(SegmentIndex)) / static_cast<float>(MarkerSegmentCount);
		MarkerSegment->SetRelativeLocation(FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 8.0f));
		MarkerSegment->SetRelativeRotation(FRotator(0.0f, FMath::RadiansToDegrees(Angle) + 90.0f, 0.0f));
		MarkerSegment->SetRelativeScale3D(FVector(ChordLength / 100.0f, 0.030f, 0.022f));
		if (UMaterialInstanceDynamic* Material = MarkerSegment->CreateAndSetMaterialInstanceDynamic(0))
		{
			Material->SetVectorParameterValue(TEXT("Color"), Red);
			Material->SetVectorParameterValue(TEXT("BaseColor"), Red);
		}
	}
}

void AWizardStaffCauldronCurseBomb::ApplyExplosion()
{
	bExplosionApplied = true;
	ExplosionVisualTimeRemaining = 0.24f;
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	World->OverlapMultiByObjectType(Overlaps, TargetLocation, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(BlastRadius));
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(Overlap.GetActor());
		if (!IsValid(Wizard) || Wizard->IsReadableOutOfArenaRespawning())
		{
			continue;
		}
		FVector KnockbackDirection = Wizard->GetActorLocation() - TargetLocation;
		KnockbackDirection.Z = 0.0f;
		if (!KnockbackDirection.Normalize())
		{
			KnockbackDirection = FVector::ForwardVector;
		}
		Wizard->ApplyBonkReaction(KnockbackDirection, HorizontalKnockback, VerticalKnockback, 0);
	}
}