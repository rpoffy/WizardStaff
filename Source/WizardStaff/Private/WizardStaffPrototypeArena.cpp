#include "WizardStaffPrototypeArena.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Algo/Sort.h"

AWizardStaffPrototypeArena::AWizardStaffPrototypeArena()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BlockMeshAsset = CubeMesh.Object;
	}

	const float WallThickness = 0.35f;
	const float WallHeight = 1.5f;
	const float FloorScale = 19.0f;
	const float WallOffset = 967.5f;

	CreateBlockComponent(TEXT("ArenaFloor"), FVector(0.0f, 0.0f, -6.0f), FVector(FloorScale, FloorScale, 0.12f));
	CreateBlockComponent(TEXT("NorthWall"), FVector(0.0f, WallOffset, 70.0f), FVector(FloorScale, WallThickness, WallHeight));
	CreateBlockComponent(TEXT("SouthWall"), FVector(0.0f, -WallOffset, 70.0f), FVector(FloorScale, WallThickness, WallHeight));
	CreateBlockComponent(TEXT("EastWall"), FVector(WallOffset, 0.0f, 70.0f), FVector(WallThickness, FloorScale, WallHeight));
	CreateBlockComponent(TEXT("WestWall"), FVector(-WallOffset, 0.0f, 70.0f), FVector(WallThickness, FloorScale, WallHeight));

	CreateBlockComponent(TEXT("DoorwayLeftWall"), FVector(-500.0f, 180.0f, 70.0f), FVector(4.8f, 0.35f, 1.4f));
	CreateBlockComponent(TEXT("DoorwayRightWall"), FVector(500.0f, 180.0f, 70.0f), FVector(4.8f, 0.35f, 1.4f));
	CreateBlockComponent(TEXT("DoorwayLintel"), FVector(0.0f, 180.0f, 240.0f), FVector(2.3f, 0.35f, 0.35f));

	CreateBlockComponent(TEXT("TableA"), FVector(-360.0f, -320.0f, 38.0f), FVector(1.5f, 0.9f, 0.28f));
	CreateBlockComponent(TEXT("TableB"), FVector(390.0f, -230.0f, 38.0f), FVector(1.3f, 1.0f, 0.28f));
	CreateBlockComponent(TEXT("BlockPropA"), FVector(-90.0f, -540.0f, 55.0f), FVector(0.8f, 0.8f, 1.1f));
	CreateBlockComponent(TEXT("BlockPropB"), FVector(210.0f, 500.0f, 55.0f), FVector(0.9f, 0.65f, 1.1f));

	CreateSpawnMarker(TEXT("PlayerSpawn_01"), FVector(260.0f, 0.0f, 120.0f), FRotator(0.0f, 180.0f, 0.0f), FColor::Red, 1.6f);
	CreateSpawnMarker(TEXT("PlayerSpawn_02"), FVector(-260.0f, 0.0f, 120.0f), FRotator(0.0f, 0.0f, 0.0f), FColor::Blue, 1.6f);
	CreateSpawnMarker(TEXT("PlayerSpawn_03"), FVector(0.0f, 260.0f, 120.0f), FRotator(0.0f, -90.0f, 0.0f), FColor::Green, 1.6f);
	CreateSpawnMarker(TEXT("PlayerSpawn_04"), FVector(0.0f, -260.0f, 120.0f), FRotator(0.0f, 90.0f, 0.0f), FColor::Yellow, 1.6f);

	CreateSpawnMarker(TEXT("MugSpawn_01"), FVector(-700.0f, -650.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_02"), FVector(700.0f, -650.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_03"), FVector(-700.0f, 650.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_04"), FVector(700.0f, 650.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_05"), FVector(-220.0f, 0.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_06"), FVector(220.0f, 0.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_07"), FVector(-620.0f, 120.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_08"), FVector(620.0f, 120.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_09"), FVector(-320.0f, -520.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_10"), FVector(360.0f, 470.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_11"), FVector(0.0f, -760.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
	CreateSpawnMarker(TEXT("MugSpawn_12"), FVector(0.0f, 760.0f, 0.0f), FRotator::ZeroRotator, FColor::Cyan, 0.8f);
}

void AWizardStaffPrototypeArena::BeginPlay()
{
	Super::BeginPlay();
	ApplyPhasePresentationState();
}

void AWizardStaffPrototypeArena::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffPrototypeArena, bReplicatedPhasePresentationActive);
}

void AWizardStaffPrototypeArena::SetPhasePresentationActive(bool bActive)
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bStateChanged = bReplicatedPhasePresentationActive != bActive;
	bReplicatedPhasePresentationActive = bActive;
	ApplyPhasePresentationState();
	if (bStateChanged)
	{
		ForceNetUpdate();
	}
}

void AWizardStaffPrototypeArena::OnRep_PhasePresentationActive()
{
	ApplyPhasePresentationState();
}

void AWizardStaffPrototypeArena::ApplyPhasePresentationState()
{
	SetActorHiddenInGame(!bReplicatedPhasePresentationActive);

	for (UStaticMeshComponent* BlockMesh : ArenaBlockMeshes)
	{
		if (!BlockMesh)
		{
			continue;
		}

		BlockMesh->SetHiddenInGame(!bReplicatedPhasePresentationActive);
		BlockMesh->SetVisibility(bReplicatedPhasePresentationActive);
		BlockMesh->SetCollisionEnabled(bReplicatedPhasePresentationActive
			? ECollisionEnabled::QueryAndPhysics
			: ECollisionEnabled::NoCollision);
	}
}

bool AWizardStaffPrototypeArena::GetPlayerSpawnTransform(int32 PlayerIndex, int32 PlayerCount, FTransform& OutTransform) const
{
	TArray<UArrowComponent*> SpawnMarkers;
	GatherSpawnMarkers(TEXT("PlayerSpawn"), SpawnMarkers);
	if (SpawnMarkers.Num() <= 0)
	{
		return false;
	}

	const int32 WrappedPlayerIndex = FMath::Abs(PlayerIndex) % FMath::Max(PlayerCount, 1);
	const int32 SafeIndex = WrappedPlayerIndex % SpawnMarkers.Num();
	const UArrowComponent* SpawnMarker = SpawnMarkers.IsValidIndex(SafeIndex) ? SpawnMarkers[SafeIndex] : nullptr;
	if (!SpawnMarker)
	{
		return false;
	}

	OutTransform = SpawnMarker->GetComponentTransform();
	return true;
}

TArray<FVector> AWizardStaffPrototypeArena::GetMugSpawnLocations() const
{
	TArray<FVector> SpawnLocations;
	TArray<UArrowComponent*> SpawnMarkers;
	GatherSpawnMarkers(TEXT("MugSpawn"), SpawnMarkers);
	for (const UArrowComponent* SpawnMarker : SpawnMarkers)
	{
		if (SpawnMarker)
		{
			SpawnLocations.Add(SpawnMarker->GetComponentLocation());
		}
	}
	return SpawnLocations;
}

FVector AWizardStaffPrototypeArena::GetArenaBoundsCenter() const
{
	return GetActorLocation();
}

UStaticMeshComponent* AWizardStaffPrototypeArena::CreateBlockComponent(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale)
{
	UStaticMeshComponent* BlockComponent = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	if (!BlockComponent)
	{
		return nullptr;
	}

	BlockComponent->SetupAttachment(SceneRoot);
	BlockComponent->SetMobility(EComponentMobility::Static);
	BlockComponent->SetRelativeLocation(RelativeLocation);
	BlockComponent->SetRelativeScale3D(RelativeScale);
	BlockComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	BlockComponent->SetCanEverAffectNavigation(false);
	if (BlockMeshAsset)
	{
		BlockComponent->SetStaticMesh(BlockMeshAsset);
	}
	ArenaBlockMeshes.Add(BlockComponent);
	return BlockComponent;
}

UArrowComponent* AWizardStaffPrototypeArena::CreateSpawnMarker(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FColor& Color, float ArrowSize)
{
	UArrowComponent* SpawnMarker = CreateDefaultSubobject<UArrowComponent>(Name);
	if (!SpawnMarker)
	{
		return nullptr;
	}

	SpawnMarker->SetupAttachment(SceneRoot);
	SpawnMarker->SetRelativeLocation(RelativeLocation);
	SpawnMarker->SetRelativeRotation(RelativeRotation);
	SpawnMarker->ArrowColor = Color;
	SpawnMarker->ArrowSize = ArrowSize;
	SpawnMarker->bHiddenInGame = true;
	SpawnMarker->SetCanEverAffectNavigation(false);

	const FString MarkerName = Name.ToString();
	if (MarkerName.StartsWith(TEXT("PlayerSpawn")))
	{
		PlayerSpawnMarkers.Add(SpawnMarker);
	}
	else
	{
		MugSpawnMarkers.Add(SpawnMarker);
	}
	return SpawnMarker;
}

void AWizardStaffPrototypeArena::GatherSpawnMarkers(const FString& NamePrefix, TArray<UArrowComponent*>& OutMarkers) const
{
	OutMarkers.Reset();

	TArray<UArrowComponent*> ArrowComponents;
	GetComponents<UArrowComponent>(ArrowComponents);
	for (UArrowComponent* ArrowComponent : ArrowComponents)
	{
		if (ArrowComponent && ArrowComponent->GetName().StartsWith(NamePrefix))
		{
			OutMarkers.Add(ArrowComponent);
		}
	}

	Algo::Sort(OutMarkers, [](const UArrowComponent* Left, const UArrowComponent* Right)
	{
		if (!Left || !Right)
		{
			return Left != nullptr;
		}
		return Left->GetName() < Right->GetName();
	});
}
