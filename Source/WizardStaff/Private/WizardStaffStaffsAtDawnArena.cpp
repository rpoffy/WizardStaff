#include "WizardStaffStaffsAtDawnArena.h"

#include "Algo/Sort.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr float StaffsAtDawnArenaLayoutScale = 2.0f;
constexpr float StaffsAtDawnArenaDeckZ = 420.0f;

FVector StaffsAtDawnLocation(float X, float Y, float Z)
{
	return FVector(X * StaffsAtDawnArenaLayoutScale, Y * StaffsAtDawnArenaLayoutScale, Z + StaffsAtDawnArenaDeckZ);
}

FVector StaffsAtDawnScale(float X, float Y, float Z)
{
	return FVector(X * StaffsAtDawnArenaLayoutScale, Y * StaffsAtDawnArenaLayoutScale, Z);
}
}

AWizardStaffStaffsAtDawnArena::AWizardStaffStaffsAtDawnArena()
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

	// Open platform layout: easier fighting than Mug Run, with exposed edges for ring-outs.
	CreateBlockComponent(TEXT("CentralCombatPlatform"), StaffsAtDawnLocation(0.0f, 0.0f, -6.0f), StaffsAtDawnScale(9.0f, 9.0f, 0.12f));
	CreateBlockComponent(TEXT("NorthRiskBridge"), StaffsAtDawnLocation(0.0f, 665.0f, -6.0f), StaffsAtDawnScale(2.15f, 4.9f, 0.12f));
	CreateBlockComponent(TEXT("SouthRiskBridge"), StaffsAtDawnLocation(0.0f, -665.0f, -6.0f), StaffsAtDawnScale(2.15f, 4.9f, 0.12f));
	CreateBlockComponent(TEXT("EastRiskBridge"), StaffsAtDawnLocation(665.0f, 0.0f, -6.0f), StaffsAtDawnScale(4.9f, 2.15f, 0.12f));
	CreateBlockComponent(TEXT("WestRiskBridge"), StaffsAtDawnLocation(-665.0f, 0.0f, -6.0f), StaffsAtDawnScale(4.9f, 2.15f, 0.12f));

	CreateBlockComponent(TEXT("NorthDuelPad"), StaffsAtDawnLocation(0.0f, 985.0f, -6.0f), StaffsAtDawnScale(5.8f, 2.9f, 0.12f));
	CreateBlockComponent(TEXT("SouthDuelPad"), StaffsAtDawnLocation(0.0f, -985.0f, -6.0f), StaffsAtDawnScale(5.8f, 2.9f, 0.12f));
	CreateBlockComponent(TEXT("EastDuelPad"), StaffsAtDawnLocation(985.0f, 0.0f, -6.0f), StaffsAtDawnScale(2.9f, 5.8f, 0.12f));
	CreateBlockComponent(TEXT("WestDuelPad"), StaffsAtDawnLocation(-985.0f, 0.0f, -6.0f), StaffsAtDawnScale(2.9f, 5.8f, 0.12f));

	CreateBlockComponent(TEXT("NorthLowWall"), StaffsAtDawnLocation(0.0f, 425.0f, 28.0f), StaffsAtDawnScale(2.2f, 0.24f, 0.62f));
	CreateBlockComponent(TEXT("SouthLowWall"), StaffsAtDawnLocation(0.0f, -425.0f, 28.0f), StaffsAtDawnScale(2.2f, 0.24f, 0.62f));
	CreateBlockComponent(TEXT("EastLowWall"), StaffsAtDawnLocation(425.0f, 0.0f, 28.0f), StaffsAtDawnScale(0.24f, 2.2f, 0.62f));
	CreateBlockComponent(TEXT("WestLowWall"), StaffsAtDawnLocation(-425.0f, 0.0f, 28.0f), StaffsAtDawnScale(0.24f, 2.2f, 0.62f));

	CreateBlockComponent(TEXT("Pillar_NE"), StaffsAtDawnLocation(380.0f, 380.0f, 68.0f), StaffsAtDawnScale(0.46f, 0.46f, 1.35f));
	CreateBlockComponent(TEXT("Pillar_NW"), StaffsAtDawnLocation(-380.0f, 380.0f, 68.0f), StaffsAtDawnScale(0.46f, 0.46f, 1.35f));
	CreateBlockComponent(TEXT("Pillar_SE"), StaffsAtDawnLocation(380.0f, -380.0f, 68.0f), StaffsAtDawnScale(0.46f, 0.46f, 1.35f));
	CreateBlockComponent(TEXT("Pillar_SW"), StaffsAtDawnLocation(-380.0f, -380.0f, 68.0f), StaffsAtDawnScale(0.46f, 0.46f, 1.35f));

	CreateBlockComponent(TEXT("StaffCatcherBlockA"), StaffsAtDawnLocation(-520.0f, 100.0f, 42.0f), StaffsAtDawnScale(0.72f, 1.05f, 0.78f), FRotator(0.0f, 14.0f, 0.0f));
	CreateBlockComponent(TEXT("StaffCatcherBlockB"), StaffsAtDawnLocation(520.0f, -100.0f, 42.0f), StaffsAtDawnScale(0.72f, 1.05f, 0.78f), FRotator(0.0f, -14.0f, 0.0f));

	CreateSpawnMarker(TEXT("PlayerSpawn_01"), StaffsAtDawnLocation(0.0f, -310.0f, 120.0f), FRotator(0.0f, 90.0f, 0.0f), FColor::Red, 1.7f);
	CreateSpawnMarker(TEXT("PlayerSpawn_02"), StaffsAtDawnLocation(0.0f, 310.0f, 120.0f), FRotator(0.0f, -90.0f, 0.0f), FColor::Blue, 1.7f);
	CreateSpawnMarker(TEXT("PlayerSpawn_03"), StaffsAtDawnLocation(-310.0f, 0.0f, 120.0f), FRotator(0.0f, 0.0f, 0.0f), FColor::Green, 1.7f);
	CreateSpawnMarker(TEXT("PlayerSpawn_04"), StaffsAtDawnLocation(310.0f, 0.0f, 120.0f), FRotator(0.0f, 180.0f, 0.0f), FColor::Yellow, 1.7f);

	CreateSpawnMarker(TEXT("FuturePowerupSpawn_01"), StaffsAtDawnLocation(0.0f, 0.0f, 18.0f), FRotator::ZeroRotator, FColor::Purple, 0.9f);
	CreateSpawnMarker(TEXT("FuturePowerupSpawn_02"), StaffsAtDawnLocation(0.0f, 985.0f, 18.0f), FRotator::ZeroRotator, FColor::Purple, 0.9f);
	CreateSpawnMarker(TEXT("FuturePowerupSpawn_03"), StaffsAtDawnLocation(985.0f, 0.0f, 18.0f), FRotator::ZeroRotator, FColor::Purple, 0.9f);
}

bool AWizardStaffStaffsAtDawnArena::GetPlayerSpawnTransform(int32 PlayerIndex, int32 PlayerCount, FTransform& OutTransform) const
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

TArray<FTransform> AWizardStaffStaffsAtDawnArena::GetPlayerSpawnTransforms() const
{
	TArray<FTransform> SpawnTransforms;
	TArray<UArrowComponent*> SpawnMarkers;
	GatherSpawnMarkers(TEXT("PlayerSpawn"), SpawnMarkers);
	for (const UArrowComponent* SpawnMarker : SpawnMarkers)
	{
		if (SpawnMarker)
		{
			SpawnTransforms.Add(SpawnMarker->GetComponentTransform());
		}
	}
	return SpawnTransforms;
}

int32 AWizardStaffStaffsAtDawnArena::GetPlayerSpawnCount() const
{
	TArray<UArrowComponent*> SpawnMarkers;
	GatherSpawnMarkers(TEXT("PlayerSpawn"), SpawnMarkers);
	return SpawnMarkers.Num();
}

TArray<FVector> AWizardStaffStaffsAtDawnArena::GetFuturePowerupSpawnLocations() const
{
	TArray<FVector> SpawnLocations;
	TArray<UArrowComponent*> SpawnMarkers;
	GatherSpawnMarkers(TEXT("FuturePowerupSpawn"), SpawnMarkers);
	for (const UArrowComponent* SpawnMarker : SpawnMarkers)
	{
		if (SpawnMarker)
		{
			SpawnLocations.Add(SpawnMarker->GetComponentLocation());
		}
	}
	return SpawnLocations;
}

int32 AWizardStaffStaffsAtDawnArena::GetFuturePowerupSpawnCount() const
{
	TArray<UArrowComponent*> SpawnMarkers;
	GatherSpawnMarkers(TEXT("FuturePowerupSpawn"), SpawnMarkers);
	return SpawnMarkers.Num();
}

FVector AWizardStaffStaffsAtDawnArena::GetArenaBoundsCenter() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, StaffsAtDawnArenaDeckZ);
}

float AWizardStaffStaffsAtDawnArena::GetRingOutFallZ() const
{
	return GetArenaBoundsCenter().Z - FMath::Max(RingOutFallDistanceBelowArena, 0.0f);
}

UStaticMeshComponent* AWizardStaffStaffsAtDawnArena::CreateBlockComponent(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale, const FRotator& RelativeRotation)
{
	UStaticMeshComponent* BlockComponent = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	if (!BlockComponent)
	{
		return nullptr;
	}

	BlockComponent->SetupAttachment(SceneRoot);
	BlockComponent->SetMobility(EComponentMobility::Static);
	BlockComponent->SetRelativeLocation(RelativeLocation);
	BlockComponent->SetRelativeRotation(RelativeRotation);
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

UArrowComponent* AWizardStaffStaffsAtDawnArena::CreateSpawnMarker(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FColor& Color, float ArrowSize)
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
		FuturePowerupSpawnMarkers.Add(SpawnMarker);
	}
	return SpawnMarker;
}

void AWizardStaffStaffsAtDawnArena::GatherSpawnMarkers(const FString& NamePrefix, TArray<UArrowComponent*>& OutMarkers) const
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
