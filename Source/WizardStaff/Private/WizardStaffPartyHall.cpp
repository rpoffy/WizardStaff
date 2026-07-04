#include "WizardStaffPartyHall.h"

#include "Algo/Sort.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AWizardStaffPartyHall::AWizardStaffPartyHall()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Static);
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CubeMesh.Succeeded())
	{
		BlockMeshAsset = CubeMesh.Object;
	}
	if (CylinderMesh.Succeeded())
	{
		CylinderMeshAsset = CylinderMesh.Object;
	}

	const float FloorScale = 12.8f;
	const float WallThickness = 0.32f;
	const float WallHeight = 1.35f;
	const float WallOffset = 656.0f;

	CreateBlockComponent(TEXT("PartyHallFloor"), FVector(0.0f, 0.0f, -6.0f), FVector(FloorScale, FloorScale, 0.12f));
	CreateBlockComponent(TEXT("NorthWall"), FVector(0.0f, WallOffset, 68.0f), FVector(FloorScale, WallThickness, WallHeight));
	CreateBlockComponent(TEXT("SouthWall"), FVector(0.0f, -WallOffset, 68.0f), FVector(FloorScale, WallThickness, WallHeight));
	CreateBlockComponent(TEXT("EastWall"), FVector(WallOffset, 0.0f, 68.0f), FVector(WallThickness, FloorScale, WallHeight));
	CreateBlockComponent(TEXT("WestWall"), FVector(-WallOffset, 0.0f, 68.0f), FVector(WallThickness, FloorScale, WallHeight));

	CreateBlockComponent(TEXT("TavernBar"), FVector(-420.0f, 170.0f, 42.0f), FVector(0.5f, 4.4f, 0.42f));
	CreateBlockComponent(TEXT("LongTable"), FVector(110.0f, -110.0f, 38.0f), FVector(2.6f, 0.95f, 0.28f));
	CreateBlockComponent(TEXT("CornerTable"), FVector(380.0f, 315.0f, 36.0f), FVector(1.25f, 1.05f, 0.26f));
	CreateBlockComponent(TEXT("BlockStoolA"), FVector(-45.0f, -245.0f, 32.0f), FVector(0.45f, 0.45f, 0.62f));
	CreateBlockComponent(TEXT("BlockStoolB"), FVector(250.0f, -245.0f, 32.0f), FVector(0.45f, 0.45f, 0.62f));
	CreateBlockComponent(TEXT("BlockStoolC"), FVector(260.0f, 80.0f, 32.0f), FVector(0.45f, 0.45f, 0.62f));
	CreateBlockComponent(TEXT("CrateStack"), FVector(-285.0f, -395.0f, 52.0f), FVector(0.8f, 0.65f, 1.05f));
	const FRotator CameraTopWallBoardTilt(-28.0f, 0.0f, 0.0f);
	const FRotator CameraTopWallTextTilt(42.0f, 180.0f, 0.0f);
	CreateBlockComponent(TEXT("StandingsSignBoard"), FVector(606.0f, 0.0f, 190.0f), FVector(0.10f, 5.45f, 1.55f), CameraTopWallBoardTilt);
	CreateBlockComponent(TEXT("CountdownSignBoard"), FVector(606.0f, -430.0f, 138.0f), FVector(0.10f, 1.95f, 0.56f), CameraTopWallBoardTilt);
	CreateBlockComponent(TEXT("LeaderSignBoard"), FVector(606.0f, 430.0f, 104.0f), FVector(0.10f, 1.95f, 0.46f), CameraTopWallBoardTilt);

	StandingsSignText = CreateSignTextComponent(TEXT("StandingsSignText"), FVector(555.0f, 0.0f, 220.0f), CameraTopWallTextTilt, TEXT("STANDINGS"), FColor::Yellow, 25.0f);
	CountdownSignText = CreateSignTextComponent(TEXT("CountdownSignText"), FVector(568.0f, -430.0f, 154.0f), CameraTopWallTextTilt, TEXT("NEXT TRIAL"), FColor::Cyan, 28.0f);
	PresetSignText = CreateSignTextComponent(TEXT("PresetSignText"), FVector(-355.0f, 330.0f, 8.0f), FRotator(90.0f, 180.0f, 0.0f), TEXT("PRESET"), FColor::Magenta, 26.0f);
	LeaderSignText = CreateSignTextComponent(TEXT("LeaderSignText"), FVector(570.0f, 430.0f, 116.0f), CameraTopWallTextTilt, TEXT("LEADER"), FColor::Yellow, 23.0f);

	ReadyBellBaseMesh = CreateBlockComponent(TEXT("ReadyBellBase"), FVector(0.0f, 420.0f, 28.0f), FVector(0.56f, 0.56f, 0.32f));
	CreateBlockComponent(TEXT("ReadyBellTextPlate"), FVector(0.0f, 330.0f, 2.0f), FVector(1.42f, 0.58f, 0.025f), FRotator::ZeroRotator, false);
	CreateBlockComponent(TEXT("PresetTextPlate"), FVector(-355.0f, 330.0f, 2.0f), FVector(1.82f, 0.62f, 0.025f), FRotator::ZeroRotator, false);

	ReadyBellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReadyBell"));
	if (ReadyBellMesh)
	{
		ReadyBellMesh->SetupAttachment(SceneRoot);
		ReadyBellMesh->SetMobility(EComponentMobility::Movable);
		ReadyBellMesh->SetRelativeLocation(FVector(0.0f, 420.0f, 82.0f));
		ReadyBellMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
		ReadyBellMesh->SetRelativeScale3D(FVector(0.42f, 0.42f, 0.24f));
		ReadyBellMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		ReadyBellMesh->SetCollisionObjectType(ECC_WorldDynamic);
		ReadyBellMesh->SetGenerateOverlapEvents(true);
		ReadyBellMesh->SetCanEverAffectNavigation(false);
		ReadyBellMesh->ComponentTags.AddUnique(GetReadyBellComponentTag());
		if (CylinderMeshAsset)
		{
			ReadyBellMesh->SetStaticMesh(CylinderMeshAsset);
		}
		ReadyBellDefaultScale = ReadyBellMesh->GetRelativeScale3D();
	}

	ReadyBellText = CreateSignTextComponent(TEXT("ReadyBellText"), FVector(0.0f, 330.0f, 8.0f), FRotator(90.0f, 180.0f, 0.0f), TEXT("READY\nBELL"), FColor::Green, 30.0f);

	CreatePhysicsPropComponent(TEXT("LooseChairA"), FVector(15.0f, -292.0f, 38.0f), FRotator(0.0f, 18.0f, 0.0f), FVector(0.36f, 0.36f, 0.68f));
	CreatePhysicsPropComponent(TEXT("LooseChairB"), FVector(315.0f, -184.0f, 38.0f), FRotator(0.0f, -26.0f, 0.0f), FVector(0.36f, 0.36f, 0.68f));
	CreatePhysicsPropComponent(TEXT("LooseBarrelA"), FVector(-496.0f, -270.0f, 46.0f), FRotator::ZeroRotator, FVector(0.48f, 0.48f, 0.70f), true);
	CreatePhysicsPropComponent(TEXT("LooseBarrelB"), FVector(-510.0f, -350.0f, 46.0f), FRotator(0.0f, 9.0f, 0.0f), FVector(0.48f, 0.48f, 0.70f), true);
	CreatePhysicsPropComponent(TEXT("LooseMugA"), FVector(48.0f, -112.0f, 74.0f), FRotator(0.0f, 38.0f, 0.0f), FVector(0.16f, 0.16f, 0.24f), true);
	CreatePhysicsPropComponent(TEXT("LooseMugB"), FVector(166.0f, -88.0f, 74.0f), FRotator(0.0f, -18.0f, 0.0f), FVector(0.16f, 0.16f, 0.24f), true);
	CreatePhysicsPropComponent(TEXT("LooseMugC"), FVector(-420.0f, 82.0f, 88.0f), FRotator(0.0f, 5.0f, 0.0f), FVector(0.16f, 0.16f, 0.24f), true);

	CreateSpawnMarker(TEXT("PlayerSpawn_01"), FVector(235.0f, 250.0f, 120.0f), FRotator(0.0f, -135.0f, 0.0f), FColor::Red, 1.6f);
	CreateSpawnMarker(TEXT("PlayerSpawn_02"), FVector(-235.0f, 250.0f, 120.0f), FRotator(0.0f, -45.0f, 0.0f), FColor::Blue, 1.6f);
	CreateSpawnMarker(TEXT("PlayerSpawn_03"), FVector(235.0f, -250.0f, 120.0f), FRotator(0.0f, 135.0f, 0.0f), FColor::Green, 1.6f);
	CreateSpawnMarker(TEXT("PlayerSpawn_04"), FVector(-235.0f, -250.0f, 120.0f), FRotator(0.0f, 45.0f, 0.0f), FColor::Yellow, 1.6f);
}

void AWizardStaffPartyHall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ReadyBellMesh)
	{
		return;
	}

	if (ReadyBellPulseRemaining <= 0.0f || ReadyBellPulseDuration <= 0.0f)
	{
		ReadyBellMesh->SetRelativeScale3D(ReadyBellDefaultScale);
		return;
	}

	ReadyBellPulseRemaining = FMath::Max(0.0f, ReadyBellPulseRemaining - DeltaSeconds);
	const float PulseAlpha = FMath::Clamp(ReadyBellPulseRemaining / ReadyBellPulseDuration, 0.0f, 1.0f);
	const FVector TargetScale = ReadyBellDefaultScale * FMath::Max(ReadyBellPulseScale, 1.0f);
	ReadyBellMesh->SetRelativeScale3D(FMath::Lerp(ReadyBellDefaultScale, TargetScale, PulseAlpha));
}

FName AWizardStaffPartyHall::GetReadyBellComponentTag()
{
	static const FName ReadyBellTag(TEXT("WizardStaffReadyBell"));
	return ReadyBellTag;
}

void AWizardStaffPartyHall::BeginPlay()
{
	Super::BeginPlay();

	if (ReadyBellMesh)
	{
		ReadyBellDefaultScale = ReadyBellMesh->GetRelativeScale3D();
	}

	for (UStaticMeshComponent* PropMesh : PhysicsPropMeshes)
	{
		if (PropMesh)
		{
			PropMesh->SetSimulatePhysics(true);
			PropMesh->SetLinearDamping(0.6f);
			PropMesh->SetAngularDamping(0.9f);
		}
	}
}

void AWizardStaffPartyHall::UpdateIntermissionSigns(const FString& StandingsText, const FString& CountdownText, const FString& PresetText, const FString& LeaderText)
{
	if (StandingsSignText)
	{
		StandingsSignText->SetText(FText::FromString(StandingsText));
	}
	if (CountdownSignText)
	{
		CountdownSignText->SetText(FText::FromString(CountdownText));
	}
	if (PresetSignText)
	{
		PresetSignText->SetText(FText::FromString(PresetText));
	}
	if (LeaderSignText)
	{
		LeaderSignText->SetText(FText::FromString(LeaderText));
	}
}

void AWizardStaffPartyHall::PlayReadyBellFeedback(int32 PlayerIndex)
{
	if (ReadyBellMesh)
	{
		ReadyBellPulseRemaining = FMath::Max(ReadyBellPulseDuration, 0.0f);
		ReadyBellMesh->SetRelativeScale3D(ReadyBellDefaultScale * FMath::Max(ReadyBellPulseScale, 1.0f));

		if (ReadyBellSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ReadyBellSound, ReadyBellMesh->GetComponentLocation());
		}
	}

	if (ReadyBellText)
	{
		ReadyBellText->SetText(FText::FromString(FString::Printf(TEXT("P%d\nREADY"), FMath::Max(PlayerIndex, 0) + 1)));
	}
}

void AWizardStaffPartyHall::ResetReadyBellFeedback()
{
	ReadyBellPulseRemaining = 0.0f;
	if (ReadyBellMesh)
	{
		ReadyBellMesh->SetRelativeScale3D(ReadyBellDefaultScale);
	}
	if (ReadyBellText)
	{
		ReadyBellText->SetText(FText::FromString(TEXT("READY\nBELL")));
	}
}

bool AWizardStaffPartyHall::GetReadyBellWorldLocation(FVector& OutLocation) const
{
	if (!ReadyBellMesh)
	{
		return false;
	}

	OutLocation = ReadyBellMesh->GetComponentLocation();
	return true;
}

bool AWizardStaffPartyHall::GetPlayerSpawnTransform(int32 PlayerIndex, int32 PlayerCount, FTransform& OutTransform) const
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

FVector AWizardStaffPartyHall::GetHallBoundsCenter() const
{
	return GetActorLocation();
}

UStaticMeshComponent* AWizardStaffPartyHall::CreateBlockComponent(FName Name, const FVector& RelativeLocation, const FVector& RelativeScale, const FRotator& RelativeRotation, bool bEnableCollision)
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
	BlockComponent->SetCollisionProfileName(bEnableCollision ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
	BlockComponent->SetCanEverAffectNavigation(false);
	if (BlockMeshAsset)
	{
		BlockComponent->SetStaticMesh(BlockMeshAsset);
	}
	HallBlockMeshes.Add(BlockComponent);
	return BlockComponent;
}

UStaticMeshComponent* AWizardStaffPartyHall::CreatePhysicsPropComponent(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FVector& RelativeScale, bool bUseCylinderMesh)
{
	UStaticMeshComponent* PropComponent = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	if (!PropComponent)
	{
		return nullptr;
	}

	PropComponent->SetupAttachment(SceneRoot);
	PropComponent->SetMobility(EComponentMobility::Movable);
	PropComponent->SetRelativeLocation(RelativeLocation);
	PropComponent->SetRelativeRotation(RelativeRotation);
	PropComponent->SetRelativeScale3D(RelativeScale);
	PropComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	PropComponent->SetCanEverAffectNavigation(false);
	PropComponent->SetMassOverrideInKg(NAME_None, bUseCylinderMesh ? 14.0f : 9.0f, true);
	PropComponent->SetStaticMesh((bUseCylinderMesh && CylinderMeshAsset) ? CylinderMeshAsset : BlockMeshAsset);
	PhysicsPropMeshes.Add(PropComponent);
	return PropComponent;
}

UTextRenderComponent* AWizardStaffPartyHall::CreateSignTextComponent(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FString& InitialText, const FColor& Color, float TextSize)
{
	UTextRenderComponent* TextComponent = CreateDefaultSubobject<UTextRenderComponent>(Name);
	if (!TextComponent)
	{
		return nullptr;
	}

	TextComponent->SetupAttachment(SceneRoot);
	TextComponent->SetMobility(EComponentMobility::Movable);
	TextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TextComponent->SetRelativeLocation(RelativeLocation);
	TextComponent->SetRelativeRotation(RelativeRotation);
	TextComponent->SetHorizontalAlignment(EHTA_Center);
	TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	TextComponent->SetTextRenderColor(Color);
	TextComponent->SetWorldSize(TextSize);
	TextComponent->SetText(FText::FromString(InitialText));
	TextComponent->SetCanEverAffectNavigation(false);
	HallSignTexts.Add(TextComponent);
	return TextComponent;
}

UArrowComponent* AWizardStaffPartyHall::CreateSpawnMarker(FName Name, const FVector& RelativeLocation, const FRotator& RelativeRotation, const FColor& Color, float ArrowSize)
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
	PlayerSpawnMarkers.Add(SpawnMarker);
	return SpawnMarker;
}

void AWizardStaffPartyHall::GatherSpawnMarkers(const FString& NamePrefix, TArray<UArrowComponent*>& OutMarkers) const
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
