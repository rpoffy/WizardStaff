#include "WizardStaffWizardCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "ProceduralMeshComponent.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardStaffArcanePinballProjectile.h"
#include "WizardStaffCauldronIngredient.h"
#include "WizardStaffComponent.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffGameState.h"
#include "WizardStaffHUD.h"
#include "WizardStaffPlayerState.h"
#include "WizardStaffPartyHall.h"
#include "WizardStaffPlaytestBotComponent.h"

namespace
{
constexpr float StaffSnapCueDurationSeconds = 0.85f;
constexpr float StaffSnapCuePitchDegrees = 16.0f;
constexpr float StaffSnapCueRollDegrees = 14.0f;
constexpr float ServerFacingYawMaxBurstDegrees = 30.0f;
constexpr float ServerFacingYawMaxInputScale = 2.0f;
constexpr float NetworkStaffClashMashMinIntervalSeconds = 0.04f;
constexpr int32 NetworkStaffClashMashCountLimit = 1000;
constexpr float CauldronStickyTetherSlackDistance = 75.0f;
constexpr float CauldronStickyTetherBreakDistance = 340.0f;
}

AWizardStaffWizardCharacter::AWizardStaffWizardCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(30.0f);
	SetMinNetUpdateFrequency(10.0f);

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 88.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->MaxAcceleration = MaxAcceleration;
	MoveComp->JumpZVelocity = HopVelocity;
	MoveComp->AirControl = 0.45f;
	MoveComp->BrakingDecelerationWalking = BrakingDecelerationWalking;
	MoveComp->bOrientRotationToMovement = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	RobeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RobeMesh"));
	RobeMesh->SetupAttachment(GetCapsuleComponent());
	RobeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RobeMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -18.0f));
	RobeMesh->SetRelativeScale3D(FVector(0.58f, 0.58f, 0.92f));

	HatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HatMesh"));
	HatMesh->SetupAttachment(GetCapsuleComponent());
	HatMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HatMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	HatMesh->SetRelativeScale3D(FVector(0.52f, 0.52f, 0.68f));

	FaceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FaceMesh"));
	FaceMesh->SetupAttachment(GetCapsuleComponent());
	FaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FaceMesh->SetRelativeLocation(FVector(38.0f, 0.0f, 18.0f));
	FaceMesh->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.16f));

	PlayerMarkerMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PlayerMarkerMesh"));
	PlayerMarkerMesh->SetupAttachment(GetCapsuleComponent());
	PlayerMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlayerMarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, PlayerMarkerZOffset));
	PlayerMarkerMesh->SetRelativeScale3D(PlayerMarkerScale);
	PlayerMarkerMesh->SetCastShadow(false);

	LeaderMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeaderMarkerMesh"));
	LeaderMarkerMesh->SetupAttachment(GetCapsuleComponent());
	LeaderMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeaderMarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, LeaderMarkerHeight));
	LeaderMarkerMesh->SetRelativeScale3D(LeaderMarkerScale);
	LeaderMarkerMesh->SetHiddenInGame(true);
	LeaderMarkerMesh->SetVisibility(false);

	SpellbookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpellbookMesh"));
	SpellbookMesh->SetupAttachment(GetCapsuleComponent());
	SpellbookMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpellbookMesh->SetRelativeLocation(SpellbookLocalLocation);
	SpellbookMesh->SetRelativeRotation(SpellbookLocalRotation);
	SpellbookMesh->SetRelativeScale3D(SpellbookScale);

	SpellbookGlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpellbookGlowMesh"));
	SpellbookGlowMesh->SetupAttachment(GetCapsuleComponent());
	SpellbookGlowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpellbookGlowMesh->SetRelativeLocation(SpellbookLocalLocation);
	SpellbookGlowMesh->SetRelativeRotation(SpellbookLocalRotation);
	SpellbookGlowMesh->SetRelativeScale3D(SpellbookGlowScale);
	SpellbookGlowMesh->SetHiddenInGame(true);
	SpellbookGlowMesh->SetVisibility(false);

	BroomHandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BroomHandleMesh"));
	BroomHandleMesh->SetupAttachment(GetCapsuleComponent());
	BroomHandleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BroomHandleMesh->SetRelativeLocation(BroomBoostTuning.BroomLocalLocation);
	BroomHandleMesh->SetRelativeScale3D(BroomBoostTuning.HandleScale);
	BroomHandleMesh->SetHiddenInGame(true);
	BroomHandleMesh->SetVisibility(false);

	BroomBristleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BroomBristleMesh"));
	BroomBristleMesh->SetupAttachment(BroomHandleMesh);
	BroomBristleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BroomBristleMesh->SetRelativeLocation(BroomBoostTuning.BristleLocalLocation);
	BroomBristleMesh->SetRelativeScale3D(BroomBoostTuning.BristleScale);
	BroomBristleMesh->SetHiddenInGame(true);
	BroomBristleMesh->SetVisibility(false);

	StaffRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StaffRoot"));
	StaffRoot->SetupAttachment(GetCapsuleComponent());
	StaffRoot->SetRelativeLocation(FVector(12.0f, 42.0f, 24.0f));

	CauldronCurseOrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CauldronCurseOrbMesh"));
	CauldronCurseOrbMesh->SetupAttachment(StaffRoot);
	CauldronCurseOrbMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CauldronCurseOrbMesh->SetGenerateOverlapEvents(false);
	CauldronCurseOrbMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
	CauldronCurseOrbMesh->SetRelativeScale3D(FVector(0.58f));
	CauldronCurseOrbMesh->SetHiddenInGame(true);
	CauldronCurseOrbMesh->SetVisibility(false);
	for (int32 AuraIndex = 0; AuraIndex < 6; ++AuraIndex)
	{
		UStaticMeshComponent* AuraMesh = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("CauldronCurseAura_%02d"), AuraIndex));
		AuraMesh->SetupAttachment(GetCapsuleComponent());
		AuraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AuraMesh->SetGenerateOverlapEvents(false);
		AuraMesh->SetHiddenInGame(true);
		AuraMesh->SetVisibility(false);
		CauldronCurseAuraMeshes.Add(AuraMesh);
	}

	StaffComponent = CreateDefaultSubobject<UWizardStaffComponent>(TEXT("StaffComponent"));
	PlaytestBotComponent = CreateDefaultSubobject<UWizardStaffPlaytestBotComponent>(TEXT("PlaytestBotComponent"));
	ArcanePinballProjectileClass = AWizardStaffArcanePinballProjectile::StaticClass();

	if (CylinderMesh.Succeeded())
	{
		RobeMesh->SetStaticMesh(CylinderMesh.Object);
	}

	if (ConeMesh.Succeeded())
	{
		HatMesh->SetStaticMesh(ConeMesh.Object);
	}

	if (SphereMesh.Succeeded())
	{
		FaceMesh->SetStaticMesh(SphereMesh.Object);
		CauldronCurseOrbMesh->SetStaticMesh(SphereMesh.Object);
		for (UStaticMeshComponent* AuraMesh : CauldronCurseAuraMeshes)
		{
			if (AuraMesh)
			{
				AuraMesh->SetStaticMesh(SphereMesh.Object);
			}
		}
	}
	if (ConeMesh.Succeeded())
	{
		LeaderMarkerMesh->SetStaticMesh(ConeMesh.Object);
	}
	if (CubeMesh.Succeeded())
	{
		SpellbookMesh->SetStaticMesh(CubeMesh.Object);
		BroomHandleMesh->SetStaticMesh(CubeMesh.Object);
		BroomBristleMesh->SetStaticMesh(CubeMesh.Object);
	}
	if (SphereMesh.Succeeded())
	{
		SpellbookGlowMesh->SetStaticMesh(SphereMesh.Object);
	}

	if (BasicMaterial.Succeeded())
	{
		CauldronCurseOrbMesh->SetMaterial(0, BasicMaterial.Object);
		for (UStaticMeshComponent* AuraMesh : CauldronCurseAuraMeshes)
		{
			if (AuraMesh)
			{
				AuraMesh->SetMaterial(0, BasicMaterial.Object);
			}
		}
		RobeMesh->SetMaterial(0, BasicMaterial.Object);
		HatMesh->SetMaterial(0, BasicMaterial.Object);
		FaceMesh->SetMaterial(0, BasicMaterial.Object);
		PlayerMarkerMesh->SetMaterial(0, BasicMaterial.Object);
		LeaderMarkerMesh->SetMaterial(0, BasicMaterial.Object);
		SpellbookMesh->SetMaterial(0, BasicMaterial.Object);
		SpellbookGlowMesh->SetMaterial(0, BasicMaterial.Object);
		BroomHandleMesh->SetMaterial(0, BasicMaterial.Object);
		BroomBristleMesh->SetMaterial(0, BasicMaterial.Object);
	}

	CauldronStickyTetherMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CauldronStickyTetherMesh"));
	CauldronStickyTetherMesh->SetupAttachment(GetCapsuleComponent());
	CauldronStickyTetherMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CauldronStickyTetherMesh->SetGenerateOverlapEvents(false);
	CauldronStickyTetherMesh->SetHiddenInGame(true);
	CauldronStickyTetherMesh->SetVisibility(false);
	if (CylinderMesh.Succeeded())
	{
		CauldronStickyTetherMesh->SetStaticMesh(CylinderMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		CauldronStickyTetherMesh->SetMaterial(0, BasicMaterial.Object);
	}
}

void AWizardStaffWizardCharacter::RebuildPlayerGroundMarkerMesh()
{
	if (!PlayerMarkerMesh)
	{
		return;
	}

	PlayerMarkerMesh->ClearAllMeshSections();

	const float Radius = FMath::Max(PlayerMarkerCircleRadius, 1.0f);
	const float PointLength = FMath::Max(PlayerMarkerPointLength, 0.0f);
	const int32 SegmentCount = FMath::Clamp(PlayerMarkerCircleSegments, 12, 96);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;

	const int32 CenterIndex = Vertices.Add(FVector::ZeroVector);
	TArray<int32> BoundaryIndices;

	if (PointLength <= KINDA_SMALL_NUMBER)
	{
		BoundaryIndices.Reserve(SegmentCount);
		for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
		{
			const float Angle = (2.0f * PI * static_cast<float>(SegmentIndex)) / static_cast<float>(SegmentCount);
			BoundaryIndices.Add(Vertices.Add(FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f)));
		}
	}
	else
	{
		const float BaseX = FMath::Clamp(Radius - FMath::Max(PlayerMarkerPointBaseInset, 0.0f), 1.0f, Radius - 1.0f);
		const float MaxHalfWidthAtBase = FMath::Sqrt(FMath::Max((Radius * Radius) - (BaseX * BaseX), 1.0f));
		const float HalfWidth = FMath::Clamp(PlayerMarkerPointHalfWidth, 1.0f, MaxHalfWidthAtBase);
		const float BaseAngle = FMath::Atan2(HalfWidth, BaseX);
		const float BackArcStart = BaseAngle;
		const float BackArcEnd = (2.0f * PI) - BaseAngle;
		const int32 BackArcSegments = FMath::Max(8, FMath::RoundToInt(static_cast<float>(SegmentCount) * ((BackArcEnd - BackArcStart) / (2.0f * PI))));

		BoundaryIndices.Reserve(BackArcSegments + 2);
		for (int32 ArcIndex = 0; ArcIndex <= BackArcSegments; ++ArcIndex)
		{
			const float Alpha = static_cast<float>(ArcIndex) / static_cast<float>(BackArcSegments);
			const float Angle = FMath::Lerp(BackArcStart, BackArcEnd, Alpha);
			BoundaryIndices.Add(Vertices.Add(FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f)));
		}
		BoundaryIndices.Add(Vertices.Add(FVector(Radius + PointLength, 0.0f, 0.0f)));
	}

	for (int32 BoundaryIndex = 0; BoundaryIndex < BoundaryIndices.Num(); ++BoundaryIndex)
	{
		const int32 CurrentIndex = BoundaryIndices[BoundaryIndex];
		const int32 NextIndex = BoundaryIndices[(BoundaryIndex + 1) % BoundaryIndices.Num()];
		Triangles.Add(CenterIndex);
		Triangles.Add(CurrentIndex);
		Triangles.Add(NextIndex);

		Triangles.Add(CenterIndex);
		Triangles.Add(NextIndex);
		Triangles.Add(CurrentIndex);
	}

	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	const float UVExtent = FMath::Max(Radius + PointLength, Radius);
	Normals.Reserve(Vertices.Num());
	UVs.Reserve(Vertices.Num());
	VertexColors.Reserve(Vertices.Num());
	Tangents.Reserve(Vertices.Num());

	for (const FVector& Vertex : Vertices)
	{
		Normals.Add(FVector::UpVector);
		UVs.Add(FVector2D((Vertex.X / (UVExtent * 2.0f)) + 0.5f, (Vertex.Y / (UVExtent * 2.0f)) + 0.5f));
		VertexColors.Add(FLinearColor::White);
		Tangents.Add(FProcMeshTangent(1.0f, 0.0f, 0.0f));
	}

	PlayerMarkerMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, false);
	PlayerMarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWizardStaffWizardCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxAcceleration = MaxAcceleration;
	GetCharacterMovement()->BrakingDecelerationWalking = BrakingDecelerationWalking;
	GetCharacterMovement()->JumpZVelocity = HopVelocity;
	DefaultCauldronGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultCauldronRotationRateYaw = GetCharacterMovement()->RotationRate.Yaw;
	ManaSlosh = FMath::Clamp(ManaSlosh, 0.0f, ManaTuning.MaxSlosh);
	if (StaffComponent)
	{
		StaffComponent->InitializeStaff(StaffRoot);
		StaffComponent->OnStaffSegmentSnapped.AddDynamic(this, &AWizardStaffWizardCharacter::HandleStaffSegmentSnapped);
	}
	if (StaffRoot)
	{
		StaffRootDefaultRelativeRotation = StaffRoot->GetRelativeRotation();
	}
	if (PlayerMarkerMesh)
	{
		RebuildPlayerGroundMarkerMesh();
		PlayerMarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, PlayerMarkerZOffset));
		PlayerMarkerMesh->SetRelativeScale3D(PlayerMarkerScale);
		PlayerMarkerMesh->SetHiddenInGame(!bShowPlayerGroundMarker);
		PlayerMarkerMesh->SetVisibility(bShowPlayerGroundMarker);
	}
	if (LeaderMarkerMesh)
	{
		LeaderMarkerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, LeaderMarkerHeight));
		LeaderMarkerMesh->SetRelativeScale3D(LeaderMarkerScale);
		SetLeaderHighlight(false);
	}
	if (SpellbookMesh)
	{
		SpellbookMesh->SetRelativeLocation(SpellbookLocalLocation);
		SpellbookMesh->SetRelativeRotation(SpellbookLocalRotation);
		SpellbookMesh->SetRelativeScale3D(SpellbookScale);
	}
	if (SpellbookGlowMesh)
	{
		SpellbookGlowMesh->SetRelativeLocation(SpellbookLocalLocation);
		SpellbookGlowMesh->SetRelativeRotation(SpellbookLocalRotation);
		SpellbookGlowMesh->SetRelativeScale3D(SpellbookGlowScale);
	}
	if (BroomHandleMesh)
	{
		BroomHandleMesh->SetRelativeLocation(BroomBoostTuning.BroomLocalLocation);
		BroomHandleMesh->SetRelativeScale3D(BroomBoostTuning.HandleScale);
		SetBroomBoostVisualActive(false);
	}
	if (BroomBristleMesh)
	{
		BroomBristleMesh->SetRelativeLocation(BroomBoostTuning.BristleLocalLocation);
		BroomBristleMesh->SetRelativeScale3D(BroomBoostTuning.BristleScale);
	}
	if (!BroomHandleMaterialInstance && BroomHandleMesh)
	{
		BroomHandleMaterialInstance = BroomHandleMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (BroomHandleMaterialInstance)
	{
		const FLinearColor HandleColor(0.38f, 0.20f, 0.08f, 1.0f);
		BroomHandleMaterialInstance->SetVectorParameterValue(TEXT("Color"), HandleColor);
		BroomHandleMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), HandleColor);
	}
	if (!BroomBristleMaterialInstance && BroomBristleMesh)
	{
		BroomBristleMaterialInstance = BroomBristleMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (BroomBristleMaterialInstance)
	{
		const FLinearColor BristleColor(0.82f, 0.62f, 0.26f, 1.0f);
		BroomBristleMaterialInstance->SetVectorParameterValue(TEXT("Color"), BristleColor);
		BroomBristleMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), BristleColor);
	}
	if (CauldronCurseOrbMesh)
	{
		CauldronCurseOrbMaterialInstance = CauldronCurseOrbMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (CauldronCurseOrbMaterialInstance)
		{
			CauldronCurseOrbMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.08f, 0.01f));
			CauldronCurseOrbMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.0f, 0.08f, 0.01f));
		}
		OnRep_ReplicatedCauldronState();
	}
	for (UStaticMeshComponent* AuraMesh : CauldronCurseAuraMeshes)
	{
		if (AuraMesh)
		{
			if (UMaterialInstanceDynamic* AuraMaterial = AuraMesh->CreateAndSetMaterialInstanceDynamic(0))
			{
				AuraMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.58f, 0.05f, 0.88f));
				AuraMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.58f, 0.05f, 0.88f));
			}
		}
	}
	if (CauldronStickyTetherMesh)
	{
		CauldronStickyTetherMaterialInstance = CauldronStickyTetherMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (CauldronStickyTetherMaterialInstance)
		{
			CauldronStickyTetherMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.26f, 0.82f, 0.10f));
			CauldronStickyTetherMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.26f, 0.82f, 0.10f));
		}
		OnRep_ReplicatedCauldronStickyTether();
	}
	UpdateSpellbookVisual();
	RefreshColorFromPlayerState();
	if (HasAuthority())
	{
		SyncReplicatedStaffSegmentCountFromAuthority();
		SyncReplicatedManaSloshFromAuthority(true);
		SyncReplicatedStaffStressFromAuthority(true);
		SyncReplicatedCarriedBrewRewardFromAuthority();
		SyncReplicatedMegaStaffStateFromAuthority(true);
		SetCauldronHazardMovementMultipliers(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		SetCauldronCurseState(false, 0.0f);
	}
	else
	{
		RebuildStaffVisualsFromReplicatedSegmentCount();
	}
}

void AWizardStaffWizardCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearStaffSnapReadabilityCue(false);
	if (StaffComponent)
	{
		StaffComponent->OnStaffSegmentSnapped.RemoveDynamic(this, &AWizardStaffWizardCharacter::HandleStaffSegmentSnapped);
	}

	Super::EndPlay(EndPlayReason);
}

void AWizardStaffWizardCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateManaSlosh(DeltaSeconds);
	UpdateHitReaction(DeltaSeconds);
	UpdateBroomBoost(DeltaSeconds);
	UpdateMegaStaffBrew(DeltaSeconds);
	UpdateStaffClash(DeltaSeconds);
	UpdateSloshMovementSettings();
	UpdateCauldronCurseVisual(DeltaSeconds);
	UpdateCauldronStickyTether(DeltaSeconds);
	UpdateSloshVisuals(DeltaSeconds);
	UpdateBonkVisual(DeltaSeconds);
	UpdateBonkAttack(DeltaSeconds);
	DrawSloshDebug();
}

void AWizardStaffWizardCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	RefreshColorFromPlayerState();
}

void AWizardStaffWizardCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	RefreshColorFromPlayerState();
}

void AWizardStaffWizardCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffSegmentCount);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedManaSlosh);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedMaxManaSlosh);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffStress);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedMaxStaffStress);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedLastSnapPlayerIndex);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedLastSnapSegmentCountAfter);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedLastSnapWasMegaTemporarySegment);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffSnapSequence);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCarriedBrewReward);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedQuickBonkSequence);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedQuickBonkVisualDuration);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedQuickBonkCancelSequence);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedOutOfArenaRespawning);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedOutOfArenaRespawnRemainingTime);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedStaffClashActive);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffClashOpponent);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffClashSequence);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffClashRemainingTime);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffClashMashCount);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedStaffClashLockedLocation);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedMegaStaffActive);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedMegaStaffRemainingTime);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedMegaStaffTemporarySegmentCount);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedMegaStaffSequence);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedBroomBoostActive);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedPrototypeInputLocked);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedCauldronCursed);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronCurseRemainingTime);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedCauldronStickyTethered);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronStickyTetherAnchor);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronCurseOrbRelativeLocation);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronMovementMultipliers);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronTurningMultiplier);
	DOREPLIFETIME(AWizardStaffWizardCharacter, bReplicatedCauldronBanking);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronBankingMovementMultipliers);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedActiveCauldronVial);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronVialCount);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronVialEffectMultipliers);
	DOREPLIFETIME(AWizardStaffWizardCharacter, ReplicatedCauldronVialInstabilityMultiplier);
}

void AWizardStaffWizardCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AWizardStaffWizardCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AWizardStaffWizardCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AWizardStaffWizardCharacter::Turn);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AWizardStaffWizardCharacter::HandleJumpPressed);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AWizardStaffWizardCharacter::HandleJumpReleased);
	PlayerInputComponent->BindAction(TEXT("DrinkMug"), IE_Pressed, this, &AWizardStaffWizardCharacter::DrinkMug);
	PlayerInputComponent->BindAction(TEXT("QuickBonk"), IE_Pressed, this, &AWizardStaffWizardCharacter::QuickBonk);
	PlayerInputComponent->BindAction(TEXT("UseReward"), IE_Pressed, this, &AWizardStaffWizardCharacter::UseReward);

#if !UE_BUILD_SHIPPING
	PlayerInputComponent->BindAction(TEXT("DebugAddStaffSegment"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugAddStaffSegment);
	PlayerInputComponent->BindAction(TEXT("DebugRemoveStaffSegment"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugRemoveStaffSegment);
	PlayerInputComponent->BindAction(TEXT("DebugResetSlosh"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugResetSlosh);
	PlayerInputComponent->BindAction(TEXT("DebugMaxStaffStress"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugMaxStaffStress);
	PlayerInputComponent->BindAction(TEXT("DebugForceSnapTopSegment"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugForceSnapTopSegment);
	PlayerInputComponent->BindAction(TEXT("DebugRestartMugRunMatch"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugRestartMugRunMatch);
	PlayerInputComponent->BindAction(TEXT("DebugCycleTuningPreset"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugCyclePrototypeTuningPreset);
	PlayerInputComponent->BindAction(TEXT("CycleWizardHudMode"), IE_Pressed, this, &AWizardStaffWizardCharacter::CycleWizardHudMode);
	PlayerInputComponent->BindAction(TEXT("DebugSetLowSlosh"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugSetLowSlosh);
	PlayerInputComponent->BindAction(TEXT("DebugSetMediumSlosh"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugSetMediumSlosh);
	PlayerInputComponent->BindAction(TEXT("DebugSetHighSlosh"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugSetHighSlosh);
	PlayerInputComponent->BindAction(TEXT("DebugSetAbsurdSlosh"), IE_Pressed, this, &AWizardStaffWizardCharacter::DebugSetAbsurdSlosh);
#endif
}

void AWizardStaffWizardCharacter::ApplyPlayerColor(int32 PlayerIndex)
{
	const FLinearColor RobeColor = GetPrototypePlayerColor(PlayerIndex);
	const FLinearColor HatColor = FLinearColor::LerpUsingHSV(RobeColor, FLinearColor::Black, 0.35f);
	CurrentRobeColor = RobeColor;
	CurrentHatColor = HatColor;

	RobeMaterialInstance = RobeMesh->CreateAndSetMaterialInstanceDynamic(0);
	HatMaterialInstance = HatMesh->CreateAndSetMaterialInstanceDynamic(0);
	UMaterialInstanceDynamic* FaceMaterialInstance = FaceMesh->CreateAndSetMaterialInstanceDynamic(0);
	PlayerMarkerMaterialInstance = PlayerMarkerMesh ? PlayerMarkerMesh->CreateAndSetMaterialInstanceDynamic(0) : nullptr;
	LeaderMarkerMaterialInstance = LeaderMarkerMesh ? LeaderMarkerMesh->CreateAndSetMaterialInstanceDynamic(0) : nullptr;
	SpellbookMaterialInstance = SpellbookMesh ? SpellbookMesh->CreateAndSetMaterialInstanceDynamic(0) : nullptr;
	SpellbookGlowMaterialInstance = SpellbookGlowMesh ? SpellbookGlowMesh->CreateAndSetMaterialInstanceDynamic(0) : nullptr;

	if (RobeMaterialInstance)
	{
		RobeMaterialInstance->SetVectorParameterValue(TEXT("Color"), RobeColor);
		RobeMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), RobeColor);
	}

	if (HatMaterialInstance)
	{
		HatMaterialInstance->SetVectorParameterValue(TEXT("Color"), HatColor);
		HatMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), HatColor);
	}

	if (FaceMaterialInstance)
	{
		const FLinearColor FaceColor(0.92f, 0.76f, 0.58f, 1.0f);
		FaceMaterialInstance->SetVectorParameterValue(TEXT("Color"), FaceColor);
		FaceMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FaceColor);
	}

	if (PlayerMarkerMaterialInstance)
	{
		const FLinearColor MarkerColor = FLinearColor::LerpUsingHSV(RobeColor, FLinearColor::White, 0.18f);
		CurrentPlayerMarkerColor = MarkerColor;
		PlayerMarkerMaterialInstance->SetVectorParameterValue(TEXT("Color"), MarkerColor);
		PlayerMarkerMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), MarkerColor);
	}
	if (LeaderMarkerMaterialInstance)
	{
		LeaderMarkerMaterialInstance->SetVectorParameterValue(TEXT("Color"), LeaderMarkerColor);
		LeaderMarkerMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), LeaderMarkerColor);
	}

	UpdateSpellbookVisual();
	UpdateMegaStaffVisual(0.0f);
}

void AWizardStaffWizardCharacter::SetLeaderHighlight(bool bNewHighlighted)
{
	if (!LeaderMarkerMesh)
	{
		return;
	}

	LeaderMarkerMesh->SetHiddenInGame(!bNewHighlighted);
	LeaderMarkerMesh->SetVisibility(bNewHighlighted, true);
}

FLinearColor AWizardStaffWizardCharacter::GetPrototypePlayerColor(int32 PlayerIndex)
{
	static const FLinearColor Colors[] =
	{
		FLinearColor(0.90f, 0.18f, 0.18f, 1.0f),
		FLinearColor(0.14f, 0.38f, 0.95f, 1.0f),
		FLinearColor(0.12f, 0.78f, 0.32f, 1.0f),
		FLinearColor(0.96f, 0.82f, 0.16f, 1.0f)
	};

	const int32 SafeIndex = FMath::Abs(PlayerIndex) % UE_ARRAY_COUNT(Colors);
	return Colors[SafeIndex];
}

void AWizardStaffWizardCharacter::DrinkMug()
{
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Standalone)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Ignoring direct DrinkMug input in network play; server-owned pickups grant mug rewards."));
		return;
	}

	ApplyCollectedMugReward();
}

void AWizardStaffWizardCharacter::ApplyCollectedMugReward()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Verbose, TEXT("Ignoring mug reward grant outside authority."));
		return;
	}

	if (StaffComponent)
	{
		const int32 PreviousSegmentCount = StaffComponent->GetSegmentCount();
		const int32 NewSegmentCount = StaffComponent->AddStaffSegment();
		if (NewSegmentCount > PreviousSegmentCount)
		{
			AddManaSloshForStaffGrowth(NewSegmentCount - PreviousSegmentCount, TEXT("ManaMugStaffGrowth"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Wizard %d drank a mug. Mana Slosh %.0f/%.0f, Staff Segments %d"),
		GetDebugPlayerIndex(),
		ManaSlosh,
		ManaTuning.MaxSlosh,
		GetStaffSegmentCount());
}

void AWizardStaffWizardCharacter::DrinkManaMug()
{
	DrinkMug();
}

void AWizardStaffWizardCharacter::ResetForNewMatch(bool bResetStaffSegments, bool bResetSlosh)
{
	if (!HasAuthority())
	{
		return;
	}

	SetPrototypeInputLocked(false);
	SetManaSloshLocked(false);

	if (bResetSlosh)
	{
		ManaSlosh = 0.0f;
	}
	else
	{
		ManaSlosh = FMath::Clamp(ManaSlosh, 0.0f, FMath::Max(ManaTuning.MaxSlosh, 1.0f));
	}

	SloshTurnCarryDegreesPerSecond = 0.0f;
	StumbleCooldownRemaining = 0.0f;
	SloshStaffVisualRotation = FRotator::ZeroRotator;
	HitReactionVisualRotation = FRotator::ZeroRotator;
	HitReactionStaffVisualRotation = FRotator::ZeroRotator;
	LastHitReactionDirection = FVector::ForwardVector;
	LastHitReactionSeverity = 0.0f;
	HitStumbleTimeRemaining = 0.0f;
	HitRecoveryTimeRemaining = 0.0f;
	HitControlLossTimeRemaining = 0.0f;
	HitKnockdownTimeRemaining = 0.0f;
	BroomBoostTimeRemaining = 0.0f;
	BroomBoostLandingCooldownRemaining = 0.0f;
	bBroomBoostAvailable = true;
	bBroomBoostActive = false;
	bBroomBoostNeedsLandingCooldown = false;
	BroomBoostDirection = FVector::ForwardVector;
	bReplicatedBroomBoostActive = false;
	bReplicatedPrototypeInputLocked = false;
	SetBroomBoostVisualActive(false);

	LastQuickBonkTime = -1000.0f;
	ClearQuickBonkState(true);
	ClearStaffClashState();
	SyncReplicatedOutOfArenaRespawnStateFromAuthority(false, 0.0f, true);
	ClearCarriedBrewReward();
	ClearMegaStaffBrew(!bResetStaffSegments);
	SetCauldronHazardMovementMultipliers(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	SetCauldronCurseState(false, 0.0f);

	if (StaffComponent)
	{
		StaffComponent->ResetForNewMatch(bResetStaffSegments);
	}
	ClearStaffSnapReadabilityCue(true);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	if (StaffRoot)
	{
		StaffRoot->SetRelativeRotation(StaffRootDefaultRelativeRotation);
	}
	if (RobeMesh)
	{
		RobeMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (HatMesh)
	{
		HatMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}
	if (FaceMesh)
	{
		FaceMesh->SetRelativeRotation(FRotator::ZeroRotator);
	}

	SetLeaderHighlight(false);
	UpdateMegaStaffVisual(0.0f);
	UpdateSloshMovementSettings();
	SyncReplicatedManaSloshFromAuthority(true);
	SyncReplicatedStaffStressFromAuthority(true);
}

void AWizardStaffWizardCharacter::AddManaSlosh(float SloshAmount)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bManaSloshLocked)
	{
		ManaSlosh = LockedManaSlosh;
		SyncReplicatedManaSloshFromAuthority();
		return;
	}

	ManaSlosh = FMath::Clamp(ManaSlosh + SloshAmount, 0.0f, FMath::Max(ManaTuning.MaxSlosh, 1.0f));
	SyncReplicatedManaSloshFromAuthority();
}

void AWizardStaffWizardCharacter::AddManaSloshForStaffGrowth(int32 SegmentCount, FName GrowthSource)
{
	if (SegmentCount <= 0 || ManaTuning.SloshGainedPerStaffSegment <= 0.0f)
	{
		return;
	}

	const float SloshToAdd = ManaTuning.SloshGainedPerStaffSegment * static_cast<float>(SegmentCount);
	AddManaSlosh(SloshToAdd);

	UE_LOG(LogTemp, Log, TEXT("Wizard %d gained %.1f Mana Slosh from %d staff segment(s): %s."),
		GetDebugPlayerIndex(),
		SloshToAdd,
		SegmentCount,
		*GrowthSource.ToString());
}

float AWizardStaffWizardCharacter::GetManaSloshAlpha() const
{
	return FMath::Clamp(ManaSlosh / FMath::Max(ManaTuning.MaxSlosh, 1.0f), 0.0f, 1.0f);
}

float AWizardStaffWizardCharacter::GetReadableManaSlosh() const
{
	return HasAuthority()
		? FMath::Clamp(ManaSlosh, 0.0f, FMath::Max(ManaTuning.MaxSlosh, 1.0f))
		: FMath::Clamp(ReplicatedManaSlosh, 0.0f, FMath::Max(GetReadableMaxManaSlosh(), 1.0f));
}

float AWizardStaffWizardCharacter::GetReadableMaxManaSlosh() const
{
	return HasAuthority()
		? FMath::Max(ManaTuning.MaxSlosh, 1.0f)
		: FMath::Max(ReplicatedMaxManaSlosh, 1.0f);
}

float AWizardStaffWizardCharacter::GetReadableManaSloshAlpha() const
{
	return FMath::Clamp(GetReadableManaSlosh() / GetReadableMaxManaSlosh(), 0.0f, 1.0f);
}

void AWizardStaffWizardCharacter::SyncReplicatedManaSloshFromAuthority(bool bForce)
{
	if (!HasAuthority())
	{
		return;
	}

	const float SafeMaxManaSlosh = FMath::Max(ManaTuning.MaxSlosh, 1.0f);
	const float SafeManaSlosh = FMath::Clamp(ManaSlosh, 0.0f, SafeMaxManaSlosh);
	constexpr float SloshReplicationThreshold = 0.5f;
	const bool bShouldUpdate = bForce
		|| FMath::Abs(SafeManaSlosh - ReplicatedManaSlosh) >= SloshReplicationThreshold
		|| (SafeManaSlosh <= KINDA_SMALL_NUMBER && ReplicatedManaSlosh > KINDA_SMALL_NUMBER)
		|| !FMath::IsNearlyEqual(SafeMaxManaSlosh, ReplicatedMaxManaSlosh, KINDA_SMALL_NUMBER);

	if (!bShouldUpdate)
	{
		return;
	}

	ReplicatedManaSlosh = SafeManaSlosh;
	ReplicatedMaxManaSlosh = SafeMaxManaSlosh;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SetManaSloshLocked(bool bNewLocked)
{
	if (!HasAuthority())
	{
		return;
	}

	bManaSloshLocked = bNewLocked;
	if (bManaSloshLocked)
	{
		LockedManaSlosh = FMath::Clamp(ManaSlosh, 0.0f, FMath::Max(ManaTuning.MaxSlosh, 1.0f));
		ManaSlosh = LockedManaSlosh;
		SyncReplicatedManaSloshFromAuthority(true);
		return;
	}

	LockedManaSlosh = 0.0f;
	ManaSlosh = FMath::Clamp(ManaSlosh, 0.0f, FMath::Max(ManaTuning.MaxSlosh, 1.0f));
	SyncReplicatedManaSloshFromAuthority(true);
}

int32 AWizardStaffWizardCharacter::GetStaffSegmentCount() const
{
	return StaffComponent ? StaffComponent->GetSegmentCount() : 0;
}

float AWizardStaffWizardCharacter::GetReadableStaffStress() const
{
	return HasAuthority()
		? (StaffComponent ? FMath::Clamp(StaffComponent->StaffStress, 0.0f, FMath::Max(StaffComponent->StressTuning.MaxStaffStress, 1.0f)) : 0.0f)
		: FMath::Clamp(ReplicatedStaffStress, 0.0f, FMath::Max(GetReadableMaxStaffStress(), 1.0f));
}

float AWizardStaffWizardCharacter::GetReadableMaxStaffStress() const
{
	return HasAuthority()
		? (StaffComponent ? FMath::Max(StaffComponent->StressTuning.MaxStaffStress, 1.0f) : 1.0f)
		: FMath::Max(ReplicatedMaxStaffStress, 1.0f);
}

float AWizardStaffWizardCharacter::GetReadableStaffStressAlpha() const
{
	return FMath::Clamp(GetReadableStaffStress() / GetReadableMaxStaffStress(), 0.0f, 1.0f);
}

void AWizardStaffWizardCharacter::SyncReplicatedStaffSegmentCountFromAuthority()
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedStaffSegmentCount = StaffComponent ? StaffComponent->GetSegmentCount() : 0;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SyncReplicatedStaffStressFromAuthority(bool bForce)
{
	if (!HasAuthority())
	{
		return;
	}

	const float SafeMaxStaffStress = StaffComponent ? FMath::Max(StaffComponent->StressTuning.MaxStaffStress, 1.0f) : 1.0f;
	const float SafeStaffStress = StaffComponent ? FMath::Clamp(StaffComponent->StaffStress, 0.0f, SafeMaxStaffStress) : 0.0f;
	constexpr float StaffStressReplicationThreshold = 0.5f;
	const bool bShouldUpdate = bForce
		|| FMath::Abs(SafeStaffStress - ReplicatedStaffStress) >= StaffStressReplicationThreshold
		|| (SafeStaffStress <= KINDA_SMALL_NUMBER && ReplicatedStaffStress > KINDA_SMALL_NUMBER)
		|| !FMath::IsNearlyEqual(SafeMaxStaffStress, ReplicatedMaxStaffStress, KINDA_SMALL_NUMBER);

	if (!bShouldUpdate)
	{
		return;
	}

	ReplicatedStaffStress = SafeStaffStress;
	ReplicatedMaxStaffStress = SafeMaxStaffStress;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SyncReplicatedStaffSnapCueFromAuthority(int32 SegmentCountAfter, bool bWasMegaTemporarySegment)
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_Standalone)
	{
		return;
	}

	const int32 SafeSegmentCountAfter = FMath::Max(SegmentCountAfter, 0);
	int32 PlayerIndex = GetDebugPlayerIndex();
	if (World)
	{
		if (const AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			const int32 GameModePlayerIndex = GameMode->GetPlayerIndexForWizard(this);
			if (GameModePlayerIndex != INDEX_NONE)
			{
				PlayerIndex = GameModePlayerIndex;
			}
		}
	}

	ReplicatedLastSnapPlayerIndex = PlayerIndex;
	ReplicatedLastSnapSegmentCountAfter = SafeSegmentCountAfter;
	bReplicatedLastSnapWasMegaTemporarySegment = bWasMegaTemporarySegment;
	++ReplicatedStaffSnapSequence;
	LastProcessedStaffSnapSequence = ReplicatedStaffSnapSequence;

	StartStaffSnapReadabilityCue(SafeSegmentCountAfter, bWasMegaTemporarySegment);
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::StartStaffSnapReadabilityCue(int32 SegmentCountAfter, bool bWasMegaTemporarySegment)
{
	(void)SegmentCountAfter;
	StaffSnapCueTimeRemaining = StaffSnapCueDurationSeconds;
	bLastStaffSnapCueWasMegaTemporarySegment = bWasMegaTemporarySegment;
}

void AWizardStaffWizardCharacter::ClearStaffSnapReadabilityCue(bool bClearReplicated)
{
	StaffSnapCueTimeRemaining = 0.0f;
	bLastStaffSnapCueWasMegaTemporarySegment = false;

	if (!bClearReplicated || !HasAuthority())
	{
		return;
	}

	ReplicatedLastSnapPlayerIndex = INDEX_NONE;
	ReplicatedLastSnapSegmentCountAfter = StaffComponent ? StaffComponent->GetSegmentCount() : 0;
	bReplicatedLastSnapWasMegaTemporarySegment = false;
	++ReplicatedStaffSnapSequence;
	LastProcessedStaffSnapSequence = ReplicatedStaffSnapSequence;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SyncReplicatedCarriedBrewRewardFromAuthority()
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedCarriedBrewReward = CarriedBrewReward;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SyncReplicatedMegaStaffStateFromAuthority(bool bForce)
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bSafeActive = bMegaStaffBrewActive;
	const float SafeRemainingTime = bSafeActive ? FMath::Max(MegaStaffTimeRemaining, 0.0f) : 0.0f;
	const int32 SafeTemporarySegments = bSafeActive ? FMath::Max(MegaStaffTemporarySegmentsRemaining, 0) : 0;
	constexpr float RemainingTimeReplicationThreshold = 0.1f;
	const bool bActiveChanged = bReplicatedMegaStaffActive != bSafeActive;
	const bool bShouldUpdate = bForce
		|| bActiveChanged
		|| FMath::Abs(ReplicatedMegaStaffRemainingTime - SafeRemainingTime) >= RemainingTimeReplicationThreshold
		|| ReplicatedMegaStaffTemporarySegmentCount != SafeTemporarySegments;

	if (!bShouldUpdate)
	{
		return;
	}

	bReplicatedMegaStaffActive = bSafeActive;
	ReplicatedMegaStaffRemainingTime = SafeRemainingTime;
	ReplicatedMegaStaffTemporarySegmentCount = SafeTemporarySegments;
	if (bForce || bActiveChanged)
	{
		++ReplicatedMegaStaffSequence;
	}
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SyncReplicatedOutOfArenaRespawnStateFromAuthority(bool bNewRespawning, float RespawnTimeRemaining, bool bForce)
{
	if (!HasAuthority())
	{
		return;
	}

	const float SafeTimeRemaining = bNewRespawning ? FMath::Max(RespawnTimeRemaining, 0.0f) : 0.0f;
	constexpr float RespawnTimeReplicationThreshold = 0.1f;
	const bool bShouldUpdate = bForce
		|| bReplicatedOutOfArenaRespawning != bNewRespawning
		|| FMath::Abs(ReplicatedOutOfArenaRespawnRemainingTime - SafeTimeRemaining) >= RespawnTimeReplicationThreshold;

	if (!bShouldUpdate)
	{
		return;
	}

	bReplicatedOutOfArenaRespawning = bNewRespawning;
	ReplicatedOutOfArenaRespawnRemainingTime = SafeTimeRemaining;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SyncReplicatedStaffClashStateFromAuthority(bool bForce)
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_Standalone)
	{
		return;
	}

	AWizardStaffWizardCharacter* SafeOpponent = bStaffClashActive ? StaffClashOpponent.Get() : nullptr;
	const float SafeRemainingTime = bStaffClashActive ? FMath::Max(StaffClashTimeRemaining, 0.0f) : 0.0f;
	const int32 SafeMashCount = bStaffClashActive ? FMath::Max(StaffClashMashCount, 0) : 0;
	const FVector SafeLockedLocation = bStaffClashActive ? StaffClashLockedLocation : FVector::ZeroVector;
	constexpr float TimeReplicationThreshold = 0.1f;
	constexpr float LocationReplicationThresholdSq = 1.0f;
	const bool bActiveChanged = bReplicatedStaffClashActive != bStaffClashActive;
	const bool bShouldUpdate = bForce
		|| bActiveChanged
		|| ReplicatedStaffClashOpponent.Get() != SafeOpponent
		|| FMath::Abs(ReplicatedStaffClashRemainingTime - SafeRemainingTime) >= TimeReplicationThreshold
		|| ReplicatedStaffClashMashCount != SafeMashCount
		|| FVector::DistSquared(ReplicatedStaffClashLockedLocation, SafeLockedLocation) >= LocationReplicationThresholdSq;

	if (!bShouldUpdate)
	{
		return;
	}

	bReplicatedStaffClashActive = bStaffClashActive;
	ReplicatedStaffClashOpponent = SafeOpponent;
	ReplicatedStaffClashRemainingTime = SafeRemainingTime;
	ReplicatedStaffClashMashCount = SafeMashCount;
	ReplicatedStaffClashLockedLocation = SafeLockedLocation;
	if (bForce || bActiveChanged)
	{
		++ReplicatedStaffClashSequence;
	}
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::QuickBonk()
{
	if (const UWorld* World = GetWorld())
	{
		if (World->GetNetMode() != NM_Standalone)
		{
			if (IsPrototypeInputBlockedForLocalInput())
			{
				return;
			}

			if (!HasAuthority() && !IsLocallyControlled())
			{
				UE_LOG(LogTemp, Verbose, TEXT("Ignoring QuickBonk from a non-owned network proxy."));
				return;
			}

			if (HasAuthority())
			{
				HandleQuickBonkRequestOnServer();
			}
			else if (bStaffClashActive)
			{
				ServerRequestStaffClashMash();
			}
			else
			{
				ServerRequestQuickBonk();
			}
			return;
		}
	}

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Verbose, TEXT("Ignoring client-side QuickBonk until server hit confirmation is implemented."));
		return;
	}

	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	if (bStaffClashActive)
	{
		RegisterStaffClashMash();
		return;
	}

	if (!CanQuickBonk())
	{
		return;
	}

	StartQuickBonkOnAuthority();
}

void AWizardStaffWizardCharacter::ServerRequestQuickBonk_Implementation()
{
	HandleQuickBonkRequestOnServer();
}

void AWizardStaffWizardCharacter::ServerRequestStaffClashMash_Implementation()
{
	HandleStaffClashMashRequestOnServer();
}

void AWizardStaffWizardCharacter::HandleQuickBonkRequestOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	AController* OwningController = GetController();
	if (!OwningController || OwningController->GetPawn() != this)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rejected networked QuickBonk request for Wizard %d: wizard is not possessed by its controller."), GetDebugPlayerIndex() + 1);
		return;
	}

	if (bPrototypeInputLocked)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Rejected networked QuickBonk request for Wizard %d: input locked."), GetDebugPlayerIndex() + 1);
		return;
	}

	if (bStaffClashActive)
	{
		RegisterStaffClashMash();
		return;
	}

	if (!CanQuickBonk())
	{
		UE_LOG(LogTemp, Verbose, TEXT("Rejected networked QuickBonk request for Wizard %d: cooldown or active swing."), GetDebugPlayerIndex() + 1);
		return;
	}

	StartQuickBonkOnAuthority();
}

void AWizardStaffWizardCharacter::HandleStaffClashMashRequestOnServer()
{
	if (!HasAuthority() || bPrototypeInputLocked || !bStaffClashActive)
	{
		return;
	}

	AController* OwningController = GetController();
	if (!OwningController || OwningController->GetPawn() != this)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rejected Staff Clash mash for Wizard %d: wizard is not possessed by its controller."), GetDebugPlayerIndex() + 1);
		return;
	}

	RegisterStaffClashMash();
}

bool AWizardStaffWizardCharacter::StartQuickBonkOnAuthority()
{
	if (!HasAuthority() || bPrototypeInputLocked || !CanQuickBonk())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float ImpactDelay = GetQuickBonkImpactDelay();
	QuickBonkVisualTimeRemaining = ImpactDelay;
	QuickBonkImpactTimeRemaining = ImpactDelay;
	QuickBonkHitCountThisSwing = 0;
	QuickBonkHitWizardsThisSwing.Reset();
	QuickBonkHitIngredientsThisSwing.Reset();
	bQuickBonkHitResolved = false;
	++AuthoritativeQuickBonkSequence;

	if (UWorld* MutableWorld = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = MutableWorld->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RecordTelemetryBonkAttempt(this);
		}
	}

	if (World && BonkTuning.BonkSwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BonkTuning.BonkSwingSound, GetActorLocation());
	}

	SyncReplicatedQuickBonkStartFromAuthority(ImpactDelay);
	return true;
}

void AWizardStaffWizardCharacter::SyncReplicatedQuickBonkStartFromAuthority(float VisualDuration)
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_Standalone)
	{
		return;
	}

	ReplicatedQuickBonkVisualDuration = FMath::Max(VisualDuration, 0.01f);
	++ReplicatedQuickBonkSequence;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::CancelQuickBonkStateForReset()
{
	ClearQuickBonkState(true);
}

void AWizardStaffWizardCharacter::CancelStaffClashStateForReset()
{
	AWizardStaffWizardCharacter* Opponent = StaffClashOpponent.Get();
	ClearStaffClashState(true);
	if (Opponent && Opponent != this && Opponent->StaffClashOpponent.Get() == this)
	{
		Opponent->ClearStaffClashState(true);
	}
}

void AWizardStaffWizardCharacter::CancelMovementStateForRespawn()
{
	CancelQuickBonkStateForReset();
	CancelStaffClashStateForReset();
	ClearStaffSnapReadabilityCue(true);
	EndBroomBoost();
	BroomBoostLandingCooldownRemaining = 0.0f;
	bBroomBoostAvailable = true;
	bBroomBoostNeedsLandingCooldown = false;
	SloshTurnCarryDegreesPerSecond = 0.0f;
}

void AWizardStaffWizardCharacter::ClearQuickBonkState(bool bSyncReplicatedCancel)
{
	QuickBonkVisualTimeRemaining = 0.0f;
	QuickBonkImpactTimeRemaining = 0.0f;
	QuickBonkHitCountThisSwing = 0;
	QuickBonkHitWizardsThisSwing.Reset();
	QuickBonkHitIngredientsThisSwing.Reset();
	bQuickBonkHitResolved = true;

	if (StaffRoot)
	{
		StaffRoot->SetRelativeRotation(StaffRootDefaultRelativeRotation + SloshStaffVisualRotation + HitReactionStaffVisualRotation);
	}

	if (!bSyncReplicatedCancel || !HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_Standalone)
	{
		return;
	}

	++ReplicatedQuickBonkCancelSequence;
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::UseReward()
{
	if (const UWorld* World = GetWorld())
	{
		if (World->GetNetMode() != NM_Standalone)
		{
			if (IsPrototypeInputBlockedForLocalInput())
			{
				return;
			}

			if (!HasAuthority() && !IsLocallyControlled())
			{
				UE_LOG(LogTemp, Verbose, TEXT("Ignoring UseReward from a non-owned network proxy."));
				return;
			}

			if (HasAuthority())
			{
				HandleNetworkedUseRewardRequestOnServer();
			}
			else
			{
				ServerUseReward();
			}
			return;
		}
	}

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Verbose, TEXT("Ignoring client-side UseReward until Use Reward RPC and projectile replication are implemented."));
		return;
	}

	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	if (bStaffClashActive)
	{
		if (GEngine && bShowWizardDebug && AWizardStaffHUD::IsFullDebugMode(this))
		{
			GEngine->AddOnScreenDebugMessage(2800 + GetDebugPlayerIndex(), 0.7f, FColor::Cyan, TEXT("Staff Clash: mash bonk, not spells."));
		}
		return;
	}

	if (!HasCarriedBrewReward())
	{
		if (GEngine && bShowWizardDebug)
		{
			GEngine->AddOnScreenDebugMessage(2800 + GetDebugPlayerIndex(), 1.0f, FColor::Silver, FString::Printf(TEXT("P%d has no brew reward."), GetDebugPlayerIndex() + 1));
		}
		return;
	}

	const EWizardBrewRewardType RewardToUse = CarriedBrewReward;
	const FString RewardName = GetBrewRewardDisplayName(RewardToUse);
	bool bUsedReward = false;

	switch (RewardToUse)
	{
	case EWizardBrewRewardType::ArcanePinball:
		bUsedReward = FireArcanePinball();
		break;
	case EWizardBrewRewardType::None:
	default:
		break;
	}

	if (bUsedReward)
	{
		ClearCarriedBrewReward();
		const TCHAR* RewardActionText = RewardToUse == EWizardBrewRewardType::ArcanePinball ? TEXT("cast") : TEXT("used");
		const FString RewardMessage = FString::Printf(TEXT("P%d %s %s!"), GetDebugPlayerIndex() + 1, RewardActionText, *RewardName);
		AWizardStaffHUD::PushGameplayMessage(this, RewardMessage, FColor::Magenta, 2.2f, EWizardHudMessageCategory::Powerup);
		if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
		{
			GEngine->AddOnScreenDebugMessage(
				2800 + GetDebugPlayerIndex(),
				1.4f,
				FColor::Magenta,
				RewardMessage);
		}
		UE_LOG(LogTemp, Log, TEXT("Wizard %d used brew reward: %s."), GetDebugPlayerIndex(), *RewardName);
	}
}

void AWizardStaffWizardCharacter::ServerUseReward_Implementation()
{
	HandleNetworkedUseRewardRequestOnServer();
}

void AWizardStaffWizardCharacter::HandleNetworkedUseRewardRequestOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	AController* OwningController = GetController();
	if (!OwningController || OwningController->GetPawn() != this)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rejected networked UseReward request for Wizard %d: wizard is not possessed by its controller."), GetDebugPlayerIndex() + 1);
		return;
	}

	if (bPrototypeInputLocked)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Rejected networked UseReward request for Wizard %d: input locked."), GetDebugPlayerIndex() + 1);
		return;
	}

	if (bStaffClashActive)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Rejected networked UseReward request for Wizard %d: Staff Clash active."), GetDebugPlayerIndex() + 1);
		return;
	}

	if (CarriedBrewReward == EWizardBrewRewardType::None)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Rejected networked UseReward request for Wizard %d: no carried reward."), GetDebugPlayerIndex() + 1);
		return;
	}

	const EWizardBrewRewardType RewardToUse = CarriedBrewReward;
	switch (RewardToUse)
	{
	case EWizardBrewRewardType::ArcanePinball:
		if (FireArcanePinball())
		{
			if (AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr)
			{
				const int32 PlayerIndex = GameMode->GetPlayerIndexForWizard(this);
				GameMode->PublishReplicatedGameplayEvent(
					EWizardReplicatedGameplayEventType::ArcanePinballCast,
					PlayerIndex == INDEX_NONE ? TEXT("Arcane Pinball cast") : FString::Printf(TEXT("P%d cast Arcane Pinball"), PlayerIndex + 1),
					PlayerIndex,
					INDEX_NONE,
					0.0f,
					true,
					2.2f);
			}
			UE_LOG(LogTemp, Log, TEXT("Accepted networked UseReward request for Wizard %d: %s consumed; spawned server-owned gameplay projectile."),
				GetDebugPlayerIndex() + 1,
				*GetBrewRewardDisplayName(RewardToUse));
			ClearCarriedBrewReward();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Rejected networked UseReward request for Wizard %d: failed to spawn Arcane Pinball gameplay projectile."), GetDebugPlayerIndex() + 1);
		}
		break;
	case EWizardBrewRewardType::None:
	default:
		UE_LOG(LogTemp, Warning, TEXT("Rejected networked UseReward request for Wizard %d: unsupported reward type."), GetDebugPlayerIndex() + 1);
		break;
	}
}

bool AWizardStaffWizardCharacter::TryGrantBrewReward(EWizardBrewRewardType RewardType, bool bReplaceExistingReward)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Verbose, TEXT("Ignoring client-side brew reward grant request."));
		return false;
	}

	if (RewardType == EWizardBrewRewardType::None)
	{
		return false;
	}

	if (HasCarriedBrewReward() && !bReplaceExistingReward)
	{
		if (GEngine && bShowWizardDebug)
		{
			GEngine->AddOnScreenDebugMessage(
				2800 + GetDebugPlayerIndex(),
				1.0f,
				FColor::Cyan,
				FString::Printf(TEXT("P%d already has %s."), GetDebugPlayerIndex() + 1, *GetCarriedBrewRewardName()));
		}
		return false;
	}

	CarriedBrewReward = RewardType;
	SyncReplicatedCarriedBrewRewardFromAuthority();
	UpdateSpellbookVisual();
	if (RewardType == EWizardBrewRewardType::ArcanePinball)
	{
		if (AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr)
		{
			GameMode->RecordTelemetryArcanePinballRewardReceived(this);
		}
	}
	AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	const int32 RewardPlayerIndex = GameMode ? GameMode->GetPlayerIndexForWizard(this) : GetDebugPlayerIndex();
	const FString RewardMessage = RewardPlayerIndex == INDEX_NONE
		? FString::Printf(TEXT("Gained brew reward: %s"), *GetCarriedBrewRewardName())
		: FString::Printf(TEXT("P%d gained brew reward: %s"), RewardPlayerIndex + 1, *GetCarriedBrewRewardName());
	if (GameMode)
	{
		GameMode->PublishReplicatedGameplayEvent(
			EWizardReplicatedGameplayEventType::BrewRewardGranted,
			RewardMessage,
			RewardPlayerIndex,
			INDEX_NONE,
			0.0f,
			true,
			2.4f);
	}
	else
	{
		AWizardStaffHUD::PushGameplayMessage(this, RewardMessage, FColor::Magenta, 2.4f, EWizardHudMessageCategory::Powerup);
	}
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(
			2800 + GetDebugPlayerIndex(),
			1.5f,
			FColor::Magenta,
			RewardMessage);
	}

	UE_LOG(LogTemp, Log, TEXT("Wizard %d gained brew reward: %s."), GetDebugPlayerIndex(), *GetCarriedBrewRewardName());
	return true;
}

void AWizardStaffWizardCharacter::ClearCarriedBrewReward()
{
	if (!HasAuthority())
	{
		return;
	}

	CarriedBrewReward = EWizardBrewRewardType::None;
	SyncReplicatedCarriedBrewRewardFromAuthority();
	UpdateSpellbookVisual();
}

EWizardBrewRewardType AWizardStaffWizardCharacter::GetCarriedBrewReward() const
{
	return HasAuthority() ? CarriedBrewReward : ReplicatedCarriedBrewReward;
}

FString AWizardStaffWizardCharacter::GetCarriedBrewRewardName() const
{
	return GetBrewRewardDisplayName(GetCarriedBrewReward());
}

FString AWizardStaffWizardCharacter::GetBrewRewardDisplayName(EWizardBrewRewardType RewardType)
{
	switch (RewardType)
	{
	case EWizardBrewRewardType::ArcanePinball:
		return TEXT("Arcane Pinball");
	case EWizardBrewRewardType::None:
	default:
		return TEXT("None");
	}
}

FLinearColor AWizardStaffWizardCharacter::GetBrewRewardGlowColor(EWizardBrewRewardType RewardType)
{
	switch (RewardType)
	{
	case EWizardBrewRewardType::ArcanePinball:
		return FLinearColor(1.0f, 0.0f, 0.85f, 1.0f);
	case EWizardBrewRewardType::None:
	default:
		return FLinearColor::Black;
	}
}

bool AWizardStaffWizardCharacter::FireArcanePinball()
{
	if (!HasAuthority())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World || !ArcanePinballProjectileClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(2800 + GetDebugPlayerIndex(), 1.25f, FColor::Red, TEXT("Arcane Pinball projectile class is missing."));
		}
		return false;
	}

	FVector LaunchDirection = GetActorForwardVector();
	LaunchDirection.Z = 0.0f;
	if (!LaunchDirection.Normalize())
	{
		LaunchDirection = FVector::ForwardVector;
	}

	const FVector SpawnLocation = GetActorLocation() + (LaunchDirection * 92.0f) + FVector(0.0f, 0.0f, 58.0f);
	const FRotator SpawnRotation = LaunchDirection.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWizardStaffArcanePinballProjectile* Projectile = World->SpawnActor<AWizardStaffArcanePinballProjectile>(
		ArcanePinballProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);

	if (!Projectile)
	{
		return false;
	}

	Projectile->InitializeArcanePinball(this, ArcanePinballTuning, LaunchDirection);
	float CastStressGained = 0.0f;
	if (StaffComponent && ArcanePinballTuning.StressOnCast > 0.0f)
	{
		CastStressGained = ArcanePinballTuning.StressOnCast;
		const bool bSnapped = StaffComponent->AddStaffStress(ArcanePinballTuning.StressOnCast, TEXT("ArcanePinballCast"));
		if (GEngine && StaffComponent->StressTuning.bShowStressDebug && AWizardStaffHUD::IsFullDebugMode(this))
		{
			GEngine->AddOnScreenDebugMessage(
				2810 + GetDebugPlayerIndex(),
				1.2f,
				bSnapped ? FColor::Red : FColor::Orange,
				FString::Printf(TEXT("P%d spell cast stress +%.0f"), GetDebugPlayerIndex() + 1, ArcanePinballTuning.StressOnCast));
		}
	}
	if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
	{
		GameMode->RecordTelemetryArcanePinballCast(this, CastStressGained);
	}
	return true;
}

bool AWizardStaffWizardCharacter::SpawnArcanePinballReadabilityShell()
{
	if (!HasAuthority())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World || !ArcanePinballProjectileClass)
	{
		return false;
	}

	FVector LaunchDirection = GetActorForwardVector();
	LaunchDirection.Z = 0.0f;
	if (!LaunchDirection.Normalize())
	{
		LaunchDirection = FVector::ForwardVector;
	}

	const FVector SpawnLocation = GetActorLocation() + (LaunchDirection * 92.0f) + FVector(0.0f, 0.0f, 58.0f);
	const FRotator SpawnRotation = LaunchDirection.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWizardStaffArcanePinballProjectile* Projectile = World->SpawnActor<AWizardStaffArcanePinballProjectile>(
		ArcanePinballProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);

	if (!Projectile)
	{
		return false;
	}

	Projectile->InitializeArcanePinballReadabilityShell(this, ArcanePinballTuning, LaunchDirection);
	return true;
}

void AWizardStaffWizardCharacter::UpdateSpellbookVisual()
{
	const EWizardBrewRewardType VisualReward = GetCarriedBrewReward();
	const bool bHasReward = VisualReward != EWizardBrewRewardType::None;
	const FLinearColor RewardGlowColor = GetBrewRewardGlowColor(VisualReward);
	const FLinearColor BookColor = bHasReward
		? FLinearColor::LerpUsingHSV(SpellbookInactiveColor, RewardGlowColor, 0.35f)
		: SpellbookInactiveColor;

	if (SpellbookMesh)
	{
		SpellbookMesh->SetHiddenInGame(false);
		SpellbookMesh->SetVisibility(true, true);
		SpellbookMesh->SetRelativeLocation(SpellbookLocalLocation);
		SpellbookMesh->SetRelativeRotation(SpellbookLocalRotation);
		SpellbookMesh->SetRelativeScale3D(SpellbookScale);
	}

	if (!SpellbookMaterialInstance && SpellbookMesh)
	{
		SpellbookMaterialInstance = SpellbookMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (SpellbookMaterialInstance)
	{
		SpellbookMaterialInstance->SetVectorParameterValue(TEXT("Color"), BookColor);
		SpellbookMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), BookColor);
		SpellbookMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), bHasReward ? RewardGlowColor : FLinearColor::Black);
	}

	if (SpellbookGlowMesh)
	{
		SpellbookGlowMesh->SetRelativeLocation(SpellbookLocalLocation);
		SpellbookGlowMesh->SetRelativeRotation(SpellbookLocalRotation);
		SpellbookGlowMesh->SetRelativeScale3D(SpellbookGlowScale);
		SpellbookGlowMesh->SetHiddenInGame(!bHasReward);
		SpellbookGlowMesh->SetVisibility(bHasReward, true);
	}

	if (!SpellbookGlowMaterialInstance && SpellbookGlowMesh)
	{
		SpellbookGlowMaterialInstance = SpellbookGlowMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (SpellbookGlowMaterialInstance)
	{
		SpellbookGlowMaterialInstance->SetVectorParameterValue(TEXT("Color"), RewardGlowColor);
		SpellbookGlowMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), RewardGlowColor);
		SpellbookGlowMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), RewardGlowColor);
	}
}

void AWizardStaffWizardCharacter::ApplyPrototypeLocalInput(float ForwardValue, float RightValue, float TurnValue)
{
	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	MoveForward(FMath::Clamp(ForwardValue, -1.0f, 1.0f));
	MoveRight(FMath::Clamp(RightValue, -1.0f, 1.0f));
	Turn(FMath::Clamp(TurnValue, -1.0f, 1.0f));
}

void AWizardStaffWizardCharacter::SetPrototypeInputLocked(bool bNewLocked)
{
	const bool bStateChanged = bPrototypeInputLocked != bNewLocked;
	if (!bStateChanged && (!HasAuthority() || bReplicatedPrototypeInputLocked == bNewLocked))
	{
		return;
	}

	bPrototypeInputLocked = bNewLocked;
	if (HasAuthority())
	{
		bReplicatedPrototypeInputLocked = bNewLocked;
		ForceNetUpdate();
	}

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!bPrototypeInputLocked)
	{
		if (MovementComponent && MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
		return;
	}

	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
	BroomBoostTimeRemaining = 0.0f;
	bBroomBoostActive = false;
	SetBroomBoostVisualActive(false);
	SloshTurnCarryDegreesPerSecond = 0.0f;
}

void AWizardStaffWizardCharacter::SetPlaytestBot(bool bNewPlaytestBot)
{
	bIsPlaytestBot = bNewPlaytestBot;
	if (PlaytestBotComponent)
	{
		PlaytestBotComponent->SetBotEnabled(bIsPlaytestBot);
	}
	if (!bIsPlaytestBot)
	{
		ApplyPrototypeLocalInput(0.0f, 0.0f, 0.0f);
	}
}

void AWizardStaffWizardCharacter::BotPressJump()
{
	HandleJumpPressed();
}

bool AWizardStaffWizardCharacter::CanBotBroomBoost() const
{
	return CanBroomBoost();
}

float AWizardStaffWizardCharacter::GetQuickBonkRange() const
{
	return StaffComponent ? StaffComponent->GetStaffCollisionLength() : BonkTuning.BaseRange + (static_cast<float>(GetStaffSegmentCount()) * BonkTuning.RangePerStaffSegment);
}

float AWizardStaffWizardCharacter::GetQuickBonkKnockbackStrength() const
{
	const float SloshKnockbackBonus = FMath::Max(ManaSlosh, 0.0f) * FMath::Max(BonkTuning.KnockbackPerManaSlosh, 0.0f);
	const float SegmentKnockbackBonus = static_cast<float>(GetStaffSegmentCount()) * BonkTuning.KnockbackPerStaffSegment;
	const float ScaledKnockback = BonkTuning.KnockbackStrength + SloshKnockbackBonus + SegmentKnockbackBonus;
	const float ClampedKnockback = BonkTuning.MaxKnockbackStrength > 0.0f ? FMath::Min(ScaledKnockback, BonkTuning.MaxKnockbackStrength) : ScaledKnockback;
	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	const float CauldronVialMultiplier = GameMode && GameMode->IsCauldronCatastropheTrialActive()
		? ReplicatedCauldronVialEffectMultipliers.Z
		: 1.0f;
	return ClampedKnockback * GetPartyHallBonkMultiplier() * GetMegaStaffKnockbackMultiplier() * CauldronVialMultiplier;
}

float AWizardStaffWizardCharacter::GetQuickBonkCooldown() const
{
	const float HeftBonus = FMath::Clamp(static_cast<float>(GetHeavyStaffSegmentCount()) * StaffHeftTuning.BonkCooldownPerHeavySegment, 0.0f, FMath::Max(StaffHeftTuning.MaxBonkCooldownBonus, 0.0f));
	return FMath::Max(0.0f, BonkTuning.Cooldown + HeftBonus);
}

float AWizardStaffWizardCharacter::GetQuickBonkImpactDelay() const
{
	return GetQuickBonkVisualDuration();
}

float AWizardStaffWizardCharacter::GetQuickBonkVisualDuration() const
{
	const float HeftBonus = FMath::Clamp(static_cast<float>(GetHeavyStaffSegmentCount()) * StaffHeftTuning.BonkVisualDurationPerHeavySegment, 0.0f, FMath::Max(StaffHeftTuning.MaxBonkVisualDurationBonus, 0.0f));
	return FMath::Max(0.01f, BonkTuning.VisualDuration + HeftBonus);
}

bool AWizardStaffWizardCharacter::CanQuickBonk() const
{
	if (bStaffClashActive || bReplicatedCauldronBanking)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float TimeSeconds = World ? World->GetTimeSeconds() : 0.0f;
	return QuickBonkVisualTimeRemaining <= 0.0f && TimeSeconds >= LastQuickBonkTime + GetQuickBonkCooldown();
}

int32 AWizardStaffWizardCharacter::ActivateMegaStaffBrew(int32 BonusSegments, float Duration, float StressMultiplierDuringEffect, float KnockbackMultiplierDuringEffect, bool bRemoveTemporarySegmentsOnExpire, bool bShowDebug)
{
	if (!HasAuthority())
	{
		return 0;
	}

	if (!StaffComponent || BonusSegments <= 0)
	{
		return 0;
	}

	if (bMegaStaffBrewActive)
	{
		ClearMegaStaffBrew(bMegaStaffRemoveTemporarySegmentsOnExpire);
	}

	int32 GrantedSegments = 0;
	for (int32 SegmentIndex = 0; SegmentIndex < BonusSegments; ++SegmentIndex)
	{
		const int32 PreviousCount = StaffComponent->GetSegmentCount();
		const int32 NewCount = StaffComponent->AddStaffSegment();
		if (NewCount <= PreviousCount)
		{
			break;
		}
		++GrantedSegments;
	}

	if (GrantedSegments <= 0)
	{
		const FString FizzleMessage = FString::Printf(TEXT("P%d Mega Staff fizzled: staff already huge."), GetDebugPlayerIndex() + 1);
		AWizardStaffHUD::PushGameplayMessage(this, FizzleMessage, FColor::Orange, 2.0f, EWizardHudMessageCategory::Powerup);
		if (GEngine && bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
		{
			GEngine->AddOnScreenDebugMessage(2860 + GetDebugPlayerIndex(), 1.3f, FColor::Orange, FizzleMessage);
		}
		return 0;
	}

	AddManaSloshForStaffGrowth(GrantedSegments, TEXT("MegaStaffBrew"));
	bMegaStaffBrewActive = true;
	MegaStaffTemporarySegmentsRemaining = GrantedSegments;
	MegaStaffTimeRemaining = FMath::Max(Duration, 0.1f);
	MegaStaffStressMultiplierDuringEffect = FMath::Max(StressMultiplierDuringEffect, 0.0f);
	MegaStaffKnockbackMultiplierDuringEffect = FMath::Max(KnockbackMultiplierDuringEffect, 0.0f);
	bMegaStaffRemoveTemporarySegmentsOnExpire = bRemoveTemporarySegmentsOnExpire;
	bMegaStaffShowDebug = bShowDebug;
	bMegaStaffExpireWarningShown = false;
	UpdateMegaStaffVisual(0.0f);
	SyncReplicatedStaffSegmentCountFromAuthority();
	SyncReplicatedMegaStaffStateFromAuthority(true);

	if (GEngine && bMegaStaffShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(
			2860 + GetDebugPlayerIndex(),
			1.5f,
			FColor::Green,
			FString::Printf(TEXT("P%d MEGA STAFF! +%d segments for %.0fs"), GetDebugPlayerIndex() + 1, GrantedSegments, MegaStaffTimeRemaining));
	}

	return GrantedSegments;
}

void AWizardStaffWizardCharacter::ClearMegaStaffBrew(bool bRemoveTemporarySegments)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bMegaStaffBrewActive && bRemoveTemporarySegments && bMegaStaffRemoveTemporarySegmentsOnExpire && StaffComponent && MegaStaffTemporarySegmentsRemaining > 0)
	{
		const int32 DesiredSegmentCount = FMath::Max(StaffComponent->GetSegmentCount() - MegaStaffTemporarySegmentsRemaining, 0);
		StaffComponent->RebuildStaffSegmentsForCount(DesiredSegmentCount);
	}

	bMegaStaffBrewActive = false;
	MegaStaffTemporarySegmentsRemaining = 0;
	MegaStaffTimeRemaining = 0.0f;
	MegaStaffStressMultiplierDuringEffect = 1.0f;
	MegaStaffKnockbackMultiplierDuringEffect = 1.0f;
	bMegaStaffRemoveTemporarySegmentsOnExpire = true;
	bMegaStaffExpireWarningShown = false;
	UpdateMegaStaffVisual(0.0f);
	SyncReplicatedStaffSegmentCountFromAuthority();
	SyncReplicatedMegaStaffStateFromAuthority(true);
}

void AWizardStaffWizardCharacter::NotifyMegaStaffSegmentSnapped()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bMegaStaffBrewActive || MegaStaffTemporarySegmentsRemaining <= 0)
	{
		return;
	}

	--MegaStaffTemporarySegmentsRemaining;
	SyncReplicatedMegaStaffStateFromAuthority(true);

	const FString SnapMessage = FString::Printf(TEXT("P%d Mega segment snapped! Temp left: %d"), GetDebugPlayerIndex() + 1, MegaStaffTemporarySegmentsRemaining);
	AWizardStaffHUD::PushGameplayMessage(this, SnapMessage, FColor::Orange, 2.0f, EWizardHudMessageCategory::Powerup);
	if (GEngine && bMegaStaffShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(
			2862 + GetDebugPlayerIndex(),
			1.25f,
			FColor::Orange,
			SnapMessage);
	}

	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RecordTelemetryMegaStaffSegmentSnapped(this);
		}
	}
}

float AWizardStaffWizardCharacter::GetStaffStressEffectMultiplier() const
{
	return bMegaStaffBrewActive ? FMath::Max(MegaStaffStressMultiplierDuringEffect, 0.0f) : 1.0f;
}

float AWizardStaffWizardCharacter::GetMegaStaffKnockbackMultiplier() const
{
	return bMegaStaffBrewActive ? FMath::Max(MegaStaffKnockbackMultiplierDuringEffect, 0.0f) : 1.0f;
}

void AWizardStaffWizardCharacter::ApplyBonkReaction(FVector KnockbackDirection, float IncomingKnockbackStrength, float IncomingUpwardBoost, int32 AttackerStaffSegments)
{
	if (bStaffClashActive)
	{
		return;
	}

	KnockbackDirection.Z = 0.0f;
	if (!KnockbackDirection.Normalize())
	{
		KnockbackDirection = -GetActorForwardVector();
		KnockbackDirection.Z = 0.0f;
		KnockbackDirection.Normalize();
	}

	const float SloshSeverityMultiplier = 1.0f + (GetManaSloshAlpha() * HitReactionTuning.SloshToStumbleMultiplier);
	const float StaffSeverityMultiplier = 1.0f + (FMath::Max(AttackerStaffSegments, 0) * 0.025f);
	const float IncomingSeverity = IncomingKnockbackStrength * StaffSeverityMultiplier;
	const float StrongHitAlpha = FMath::Clamp(IncomingSeverity / FMath::Max(HitReactionTuning.StrongHitThreshold, 1.0f), 0.0f, 1.35f);
	const float ReactionDuration = HitReactionTuning.StumbleDuration * FMath::Lerp(0.8f, 1.2f, FMath::Clamp(StrongHitAlpha, 0.0f, 1.0f)) * SloshSeverityMultiplier;
	const bool bStrongHit = IncomingSeverity >= HitReactionTuning.StrongHitThreshold;
	const bool bKnockdown = HitReactionTuning.bEnableKnockdown && IncomingSeverity >= HitReactionTuning.KnockdownThreshold;

	LastHitReactionDirection = KnockbackDirection;
	LastHitReactionSeverity = StrongHitAlpha;
	HitStumbleTimeRemaining = FMath::Max(HitStumbleTimeRemaining, ReactionDuration);
	HitRecoveryTimeRemaining = FMath::Max(HitRecoveryTimeRemaining, ReactionDuration + HitReactionTuning.RecoveryTime);

	if (bStrongHit)
	{
		HitControlLossTimeRemaining = FMath::Max(HitControlLossTimeRemaining, HitReactionTuning.StrongHitControlLossDuration * SloshSeverityMultiplier);
	}

	if (bKnockdown)
	{
		HitKnockdownTimeRemaining = FMath::Max(HitKnockdownTimeRemaining, HitReactionTuning.KnockdownDuration * SloshSeverityMultiplier);
		HitStumbleTimeRemaining = FMath::Max(HitStumbleTimeRemaining, HitKnockdownTimeRemaining);
		HitRecoveryTimeRemaining = FMath::Max(HitRecoveryTimeRemaining, HitKnockdownTimeRemaining + HitReactionTuning.RecoveryTime);
	}

	const FVector KnockbackVelocity = (KnockbackDirection * IncomingKnockbackStrength * HitReactionTuning.KnockbackScale) + FVector(0.0f, 0.0f, IncomingUpwardBoost);
	LaunchCharacter(KnockbackVelocity, true, false);

	if (GEngine && HitReactionTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		const FString DebugText = FString::Printf(
			TEXT("P%d reaction: %s %.2fs | slosh %.0f%% | force %.0f"),
			GetDebugPlayerIndex() + 1,
			bKnockdown ? TEXT("knockdown") : (bStrongHit ? TEXT("strong stumble") : TEXT("stumble")),
			HitStumbleTimeRemaining,
			GetManaSloshAlpha() * 100.0f,
			IncomingKnockbackStrength);
		GEngine->AddOnScreenDebugMessage(2700 + GetDebugPlayerIndex(), 0.9f, bKnockdown ? FColor::Red : FColor::Orange, DebugText);
	}
}

bool AWizardStaffWizardCharacter::IsReactingToHit() const
{
	return HitStumbleTimeRemaining > 0.0f || HitRecoveryTimeRemaining > 0.0f || HitControlLossTimeRemaining > 0.0f || HitKnockdownTimeRemaining > 0.0f;
}

float AWizardStaffWizardCharacter::GetStaffHeftMovementMultiplier() const
{
	const float HeftPenalty = FMath::Clamp(static_cast<float>(GetHeavyStaffSegmentCount()) * StaffHeftTuning.MovementPenaltyPerHeavySegment, 0.0f, FMath::Max(StaffHeftTuning.MaxMovementPenalty, 0.0f));
	return FMath::Clamp(1.0f - HeftPenalty, 0.05f, 1.0f);
}

float AWizardStaffWizardCharacter::GetStaffHeftTurnMultiplier() const
{
	const float HeftPenalty = FMath::Clamp(static_cast<float>(GetHeavyStaffSegmentCount()) * StaffHeftTuning.TurnPenaltyPerHeavySegment, 0.0f, FMath::Max(StaffHeftTuning.MaxTurnPenalty, 0.0f));
	return FMath::Clamp(1.0f - HeftPenalty, 0.05f, 1.0f);
}

void AWizardStaffWizardCharacter::DebugAddStaffSegment()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugAddStaffSegment ignored on non-authority wizard. Use server smoke-test helpers from the listen-server host."));
		return;
	}

	if (StaffComponent)
	{
		const int32 PreviousSegmentCount = StaffComponent->GetSegmentCount();
		const int32 NewSegmentCount = StaffComponent->AddStaffSegment();
		if (NewSegmentCount > PreviousSegmentCount)
		{
			AddManaSloshForStaffGrowth(NewSegmentCount - PreviousSegmentCount, TEXT("DebugStaffGrowth"));
		}
	}
}

void AWizardStaffWizardCharacter::DebugRemoveStaffSegment()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugRemoveStaffSegment ignored on non-authority wizard. Use server smoke-test helpers from the listen-server host."));
		return;
	}

	if (StaffComponent)
	{
		StaffComponent->RemoveTopStaffSegment(false);
	}
}

void AWizardStaffWizardCharacter::DebugResetSlosh()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugResetSlosh ignored on non-authority wizard. Use server smoke-test helpers from the listen-server host."));
		return;
	}

	ManaSlosh = 0.0f;
	SloshTurnCarryDegreesPerSecond = 0.0f;
	SyncReplicatedManaSloshFromAuthority(true);
}

void AWizardStaffWizardCharacter::DebugMaxStaffStress()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugMaxStaffStress ignored on non-authority wizard. Use server smoke-test helpers from the listen-server host."));
		return;
	}

	if (StaffComponent)
	{
		StaffComponent->StaffStress = StaffComponent->StressTuning.MaxStaffStress;
		SyncReplicatedStaffStressFromAuthority(true);
	}
}

void AWizardStaffWizardCharacter::DebugForceSnapTopSegment()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugForceSnapTopSegment ignored on non-authority wizard. Use server smoke-test helpers from the listen-server host."));
		return;
	}

	if (StaffComponent)
	{
		StaffComponent->SnapTopStaffSegment();
	}
}

void AWizardStaffWizardCharacter::DebugRestartMugRunMatch()
{
	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RestartMugRunMatch();
		}
	}
}

void AWizardStaffWizardCharacter::DebugCyclePrototypeTuningPreset()
{
	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->CyclePrototypeTuningPreset();
		}
	}
}

void AWizardStaffWizardCharacter::CycleWizardHudMode()
{
	APlayerController* TargetPlayerController = nullptr;
	if (UWorld* World = GetWorld())
	{
		TargetPlayerController = World->GetFirstPlayerController();
	}
	if (!TargetPlayerController)
	{
		TargetPlayerController = Cast<APlayerController>(GetController());
	}

	if (TargetPlayerController)
	{
		if (AWizardStaffHUD* WizardHud = Cast<AWizardStaffHUD>(TargetPlayerController->GetHUD()))
		{
			WizardHud->CycleWizardHudMode();
		}
	}
}

void AWizardStaffWizardCharacter::DebugSetLowSlosh()
{
	SetDebugSloshAlpha(ManaTuning.DebugLowSloshAlpha);
}

void AWizardStaffWizardCharacter::DebugSetMediumSlosh()
{
	SetDebugSloshAlpha(ManaTuning.DebugMediumSloshAlpha);
}

void AWizardStaffWizardCharacter::DebugSetHighSlosh()
{
	SetDebugSloshAlpha(ManaTuning.DebugHighSloshAlpha);
}

void AWizardStaffWizardCharacter::DebugSetAbsurdSlosh()
{
	SetDebugSloshAlpha(ManaTuning.DebugAbsurdSloshAlpha);
}

void AWizardStaffWizardCharacter::HandleJumpPressed()
{
	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	if (bStaffClashActive)
	{
		return;
	}

	if (CanBroomBoost())
	{
		if (HasAuthority())
		{
			ActivateBroomBoost();
		}
		else if (IsLocallyControlled())
		{
			ServerRequestBroomBoost();
		}
		return;
	}

	Jump();
}

void AWizardStaffWizardCharacter::HandleJumpReleased()
{
	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	StopJumping();
}

void AWizardStaffWizardCharacter::ServerRequestBroomBoost_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	AController* OwningController = GetController();
	if (!OwningController || OwningController->GetPawn() != this)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rejected networked broom boost request for Wizard %d: wizard is not possessed by its controller."), GetDebugPlayerIndex() + 1);
		return;
	}

	if (bPrototypeInputLocked || bStaffClashActive)
	{
		return;
	}

	if (CanBroomBoost())
	{
		ActivateBroomBoost();
	}
}

void AWizardStaffWizardCharacter::MoveForward(float Value)
{
	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	if (!FMath::IsNearlyZero(Value))
	{
		if (StaffComponent)
		{
			StaffComponent->NotifyOwnerMovementInput(FMath::Abs(Value));
		}
		const float StaffControlMultiplier = StaffComponent ? StaffComponent->GetControlInputMultiplier() : 1.0f;
		AddMovementInput(GetActorForwardVector(), Value * MoveInputScale * StaffControlMultiplier * GetManaSloshMovementMultiplier() * GetHitReactionInputMultiplier() * GetStaffHeftMovementMultiplier());
	}
}

void AWizardStaffWizardCharacter::MoveRight(float Value)
{
	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	if (!FMath::IsNearlyZero(Value))
	{
		if (StaffComponent)
		{
			StaffComponent->NotifyOwnerMovementInput(FMath::Abs(Value));
		}
		const float StaffControlMultiplier = StaffComponent ? StaffComponent->GetControlInputMultiplier() : 1.0f;
		AddMovementInput(GetActorRightVector(), Value * MoveInputScale * StaffControlMultiplier * GetManaSloshMovementMultiplier() * GetHitReactionInputMultiplier() * GetStaffHeftMovementMultiplier());
	}
}

void AWizardStaffWizardCharacter::Turn(float Value)
{
	if (IsPrototypeInputBlockedForLocalInput())
	{
		return;
	}

	if (!FMath::IsNearlyZero(Value))
	{
		const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
		const float StaffControlMultiplier = StaffComponent ? StaffComponent->GetControlInputMultiplier() : 1.0f;
		AddActorWorldRotation(FRotator(0.0f, Value * TurnRateDegreesPerSecond * StaffControlMultiplier * GetManaSloshTurnMultiplier() * GetHitReactionInputMultiplier() * GetStaffHeftTurnMultiplier() * DeltaSeconds, 0.0f));
		if (!HasAuthority() && IsLocallyControlled())
		{
			ServerSetFacingYaw(GetActorRotation().Yaw);
		}

		const float TargetCarry = Value * ManaTuning.SloshOversteerDegreesPerSecond * GetManaSloshAlpha();
		SloshTurnCarryDegreesPerSecond = FMath::FInterpTo(SloshTurnCarryDegreesPerSecond, TargetCarry, DeltaSeconds, ManaTuning.SloshOversteerRecoverySpeed);
	}
}

void AWizardStaffWizardCharacter::ServerSetFacingYaw_Implementation(float NewYaw)
{
	if (!HasAuthority() || !FMath::IsFinite(NewYaw) || bPrototypeInputLocked || bStaffClashActive)
	{
		return;
	}

	AController* OwningController = GetController();
	if (!OwningController || OwningController->GetPawn() != this)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;
	if (ServerFacingYawLastUpdateTime < 0.0f)
	{
		ServerFacingYawLastUpdateTime = Now;
		ServerFacingYawTurnAllowanceDegrees = ServerFacingYawMaxBurstDegrees;
	}
	else
	{
		const float Elapsed = FMath::Max(Now - ServerFacingYawLastUpdateTime, 0.0f);
		const float StaffControlMultiplier = StaffComponent ? StaffComponent->GetControlInputMultiplier() : 1.0f;
		const float AllowedTurnRate = FMath::Max(
			TurnRateDegreesPerSecond
				* ServerFacingYawMaxInputScale
				* StaffControlMultiplier
				* GetManaSloshTurnMultiplier()
				* GetHitReactionInputMultiplier()
				* GetStaffHeftTurnMultiplier(),
			0.0f);
		ServerFacingYawTurnAllowanceDegrees = FMath::Min(
			ServerFacingYawMaxBurstDegrees,
			ServerFacingYawTurnAllowanceDegrees + (AllowedTurnRate * Elapsed));
		ServerFacingYawLastUpdateTime = Now;
	}

	FRotator NewRotation = GetActorRotation();
	NewRotation.Pitch = 0.0f;
	const float RequestedDelta = FMath::FindDeltaAngleDegrees(NewRotation.Yaw, FRotator::NormalizeAxis(NewYaw));
	const float AppliedDelta = FMath::Clamp(
		RequestedDelta,
		-ServerFacingYawTurnAllowanceDegrees,
		ServerFacingYawTurnAllowanceDegrees);
	NewRotation.Yaw = FRotator::NormalizeAxis(NewRotation.Yaw + AppliedDelta);
	NewRotation.Roll = 0.0f;
	SetActorRotation(NewRotation);
	ServerFacingYawTurnAllowanceDegrees = FMath::Max(0.0f, ServerFacingYawTurnAllowanceDegrees - FMath::Abs(AppliedDelta));
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedStaffSegmentCount()
{
	RebuildStaffVisualsFromReplicatedSegmentCount();
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedManaSlosh()
{
	ReplicatedMaxManaSlosh = FMath::Max(ReplicatedMaxManaSlosh, 1.0f);
	ReplicatedManaSlosh = FMath::Clamp(ReplicatedManaSlosh, 0.0f, ReplicatedMaxManaSlosh);
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedStaffStress()
{
	ReplicatedMaxStaffStress = FMath::Max(ReplicatedMaxStaffStress, 1.0f);
	ReplicatedStaffStress = FMath::Clamp(ReplicatedStaffStress, 0.0f, ReplicatedMaxStaffStress);
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedStaffSnapSequence()
{
	if (HasAuthority() || ReplicatedStaffSnapSequence <= 0 || ReplicatedStaffSnapSequence == LastProcessedStaffSnapSequence)
	{
		return;
	}

	LastProcessedStaffSnapSequence = ReplicatedStaffSnapSequence;
	if (ReplicatedLastSnapPlayerIndex == INDEX_NONE)
	{
		StaffSnapCueTimeRemaining = 0.0f;
		bLastStaffSnapCueWasMegaTemporarySegment = false;
		return;
	}

	RebuildStaffVisualsFromReplicatedSegmentCount();
	StartStaffSnapReadabilityCue(
		ReplicatedLastSnapSegmentCountAfter,
		bReplicatedLastSnapWasMegaTemporarySegment);
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedCarriedBrewReward()
{
	UpdateSpellbookVisual();
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedQuickBonkSequence()
{
	if (HasAuthority())
	{
		return;
	}

	QuickBonkVisualTimeRemaining = FMath::Max(ReplicatedQuickBonkVisualDuration, 0.01f);
	QuickBonkImpactTimeRemaining = 0.0f;
	QuickBonkHitCountThisSwing = 0;
	QuickBonkHitWizardsThisSwing.Reset();
	QuickBonkHitIngredientsThisSwing.Reset();
	bQuickBonkHitResolved = true;
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedQuickBonkCancelSequence()
{
	if (HasAuthority())
	{
		return;
	}

	ClearQuickBonkState(false);
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedOutOfArenaRespawnState()
{
	ReplicatedOutOfArenaRespawnRemainingTime = bReplicatedOutOfArenaRespawning
		? FMath::Max(ReplicatedOutOfArenaRespawnRemainingTime, 0.0f)
		: 0.0f;

	if (bReplicatedOutOfArenaRespawning)
	{
		ClearQuickBonkState(false);
		ClearStaffSnapReadabilityCue(false);
	}
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedStaffClashState()
{
	if (HasAuthority())
	{
		return;
	}

	if (!bReplicatedStaffClashActive)
	{
		ClearStaffClashState(false);
		return;
	}

	StaffClashOpponent = ReplicatedStaffClashOpponent;
	StaffClashLockedLocation = (!ReplicatedStaffClashLockedLocation.IsNearlyZero() || GetActorLocation().IsNearlyZero())
		? ReplicatedStaffClashLockedLocation
		: GetActorLocation();
	StaffClashTimeRemaining = FMath::Max(ReplicatedStaffClashRemainingTime, 0.0f);
	StaffClashMashCount = FMath::Max(ReplicatedStaffClashMashCount, 0);
	bStaffClashActive = true;
	bStaffClashResolver = false;
	bQuickBonkHitResolved = true;
	QuickBonkImpactTimeRemaining = 0.0f;
	QuickBonkVisualTimeRemaining = 0.0f;
	QuickBonkHitCountThisSwing = 0;
	QuickBonkHitWizardsThisSwing.Reset();
	QuickBonkHitIngredientsThisSwing.Reset();
	EndBroomBoost();
	StopJumping();
	LockStaffClashPosition();
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedMegaStaffState()
{
	if (HasAuthority())
	{
		return;
	}

	bMegaStaffBrewActive = bReplicatedMegaStaffActive;
	MegaStaffTimeRemaining = bMegaStaffBrewActive ? FMath::Max(ReplicatedMegaStaffRemainingTime, 0.0f) : 0.0f;
	MegaStaffTemporarySegmentsRemaining = bMegaStaffBrewActive ? FMath::Max(ReplicatedMegaStaffTemporarySegmentCount, 0) : 0;
	if (!bMegaStaffBrewActive)
	{
		MegaStaffStressMultiplierDuringEffect = 1.0f;
		MegaStaffKnockbackMultiplierDuringEffect = 1.0f;
		bMegaStaffRemoveTemporarySegmentsOnExpire = true;
		bMegaStaffExpireWarningShown = false;
	}
	UpdateMegaStaffVisual(0.0f);
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedBroomBoostActive()
{
	if (HasAuthority())
	{
		return;
	}

	SetBroomBoostVisualActive(bReplicatedBroomBoostActive);
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedPrototypeInputLocked()
{
	if (HasAuthority())
	{
		return;
	}

	bPrototypeInputLocked = bReplicatedPrototypeInputLocked;
	if (!bPrototypeInputLocked)
	{
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			if (MovementComponent->MovementMode == MOVE_None)
			{
				MovementComponent->SetMovementMode(MOVE_Walking);
			}
		}
		return;
	}

	StopJumping();
	EndBroomBoost();
	SloshTurnCarryDegreesPerSecond = 0.0f;
	if (IsLocallyControlled())
	{
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
	}
}

void AWizardStaffWizardCharacter::RebuildStaffVisualsFromReplicatedSegmentCount()
{
	if (!StaffComponent)
	{
		return;
	}

	if (StaffRoot)
	{
		StaffComponent->InitializeStaff(StaffRoot);
	}

	const int32 SafeSegmentCount = FMath::Clamp(
		ReplicatedStaffSegmentCount,
		0,
		FMath::Max(StaffComponent->VisualTuning.MaxTestSegments, 0));
	if (StaffComponent->GetSegmentCount() == SafeSegmentCount)
	{
		return;
	}

	StaffComponent->RebuildStaffSegmentsForCount(SafeSegmentCount);
}

void AWizardStaffWizardCharacter::RefreshColorFromPlayerState()
{
	int32 PlayerIndex = 0;
	bool bUsedWizardPlayerState = false;

	if (const AWizardStaffPlayerState* WizardPlayerState = GetPlayerState<AWizardStaffPlayerState>())
	{
		if (WizardPlayerState->GetWizardDisplaySlot() != INDEX_NONE)
		{
			PlayerIndex = WizardPlayerState->GetWizardColorIndex();
			bUsedWizardPlayerState = true;
		}
	}
	if (!bUsedWizardPlayerState)
	{
		if (const APlayerState* CurrentPlayerState = GetPlayerState())
		{
			PlayerIndex = CurrentPlayerState->GetPlayerId();
		}
	}

	ApplyPlayerColor(PlayerIndex);
}

void AWizardStaffWizardCharacter::UpdateManaSlosh(float DeltaSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bManaSloshLocked)
	{
		ManaSlosh = LockedManaSlosh;
	}
	else
	{
		ManaSlosh = FMath::Max(0.0f, ManaSlosh - (ManaTuning.SloshDecayPerSecond * DeltaSeconds));
	}

	StumbleCooldownRemaining = FMath::Max(0.0f, StumbleCooldownRemaining - DeltaSeconds);

	if (!FMath::IsNearlyZero(SloshTurnCarryDegreesPerSecond))
	{
		AddActorWorldRotation(FRotator(0.0f, SloshTurnCarryDegreesPerSecond * DeltaSeconds, 0.0f));
		if (!HasAuthority() && IsLocallyControlled())
		{
			ServerSetFacingYaw(GetActorRotation().Yaw);
		}
		SloshTurnCarryDegreesPerSecond = FMath::FInterpTo(SloshTurnCarryDegreesPerSecond, 0.0f, DeltaSeconds, ManaTuning.SloshOversteerRecoverySpeed);
	}

	const float SloshAlpha = GetManaSloshAlpha();
	if (StumbleCooldownRemaining <= 0.0f && SloshAlpha >= ManaTuning.StumbleMinSloshAlpha && ManaTuning.StumbleChancePerSecondAtMaxSlosh > 0.0f)
	{
		const float StumbleChance = ManaTuning.StumbleChancePerSecondAtMaxSlosh * SloshAlpha * DeltaSeconds;
		if (FMath::FRand() <= StumbleChance)
		{
			const float Direction = FMath::RandBool() ? 1.0f : -1.0f;
			SloshTurnCarryDegreesPerSecond += Direction * ManaTuning.StumbleTurnKickDegreesPerSecond * SloshAlpha;
			StumbleCooldownRemaining = ManaTuning.StumbleCooldown;
		}
	}

	SyncReplicatedManaSloshFromAuthority();
}

void AWizardStaffWizardCharacter::UpdateHitReaction(float DeltaSeconds)
{
	HitStumbleTimeRemaining = FMath::Max(0.0f, HitStumbleTimeRemaining - DeltaSeconds);
	HitRecoveryTimeRemaining = FMath::Max(0.0f, HitRecoveryTimeRemaining - DeltaSeconds);
	HitControlLossTimeRemaining = FMath::Max(0.0f, HitControlLossTimeRemaining - DeltaSeconds);
	HitKnockdownTimeRemaining = FMath::Max(0.0f, HitKnockdownTimeRemaining - DeltaSeconds);

	if (!IsReactingToHit())
	{
		HitReactionVisualRotation = FRotator::ZeroRotator;
		HitReactionStaffVisualRotation = FRotator::ZeroRotator;
		LastHitReactionSeverity = 0.0f;
		return;
	}

	const float StumbleDuration = FMath::Max(HitReactionTuning.StumbleDuration, 0.01f);
	const float RecoveryTime = FMath::Max(HitReactionTuning.RecoveryTime, 0.01f);
	const float StumbleAlpha = FMath::Clamp(HitStumbleTimeRemaining / StumbleDuration, 0.0f, 1.0f);
	const float RecoveryAlpha = HitStumbleTimeRemaining > 0.0f
		? StumbleAlpha
		: FMath::Clamp(HitRecoveryTimeRemaining / RecoveryTime, 0.0f, 1.0f);
	const float ReactionAlpha = FMath::Max(StumbleAlpha, RecoveryAlpha);
	const float VisualLeanDegrees = HitKnockdownTimeRemaining > 0.0f
		? HitReactionTuning.KnockdownVisualLeanDegrees
		: HitReactionTuning.StumbleVisualLeanDegrees;

	const FVector LocalKnockbackDirection = GetActorTransform().InverseTransformVectorNoScale(LastHitReactionDirection).GetSafeNormal();
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Wobble = FMath::Sin(TimeSeconds * 18.0f) * ReactionAlpha * 3.5f;
	const float SeverityScale = FMath::Clamp(LastHitReactionSeverity, 0.65f, 1.25f);
	const float LeanPitch = -LocalKnockbackDirection.X * VisualLeanDegrees * ReactionAlpha * SeverityScale;
	const float LeanRoll = LocalKnockbackDirection.Y * VisualLeanDegrees * ReactionAlpha * SeverityScale;

	HitReactionVisualRotation = FRotator(LeanPitch, 0.0f, LeanRoll + Wobble);
	HitReactionStaffVisualRotation = FRotator(0.0f, Wobble * 0.35f, (-LeanRoll * 0.35f) + (Wobble * 0.2f));
}

void AWizardStaffWizardCharacter::UpdateBroomBoost(float DeltaSeconds)
{
	if (!HasAuthority())
	{
		SetBroomBoostVisualActive(bReplicatedBroomBoostActive);
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		EndBroomBoost();
		return;
	}

	if (bStaffClashActive)
	{
		EndBroomBoost();
		return;
	}

	if (!MoveComp->IsFalling())
	{
		EndBroomBoost();

		if (bBroomBoostNeedsLandingCooldown)
		{
			if (BroomBoostLandingCooldownRemaining <= 0.0f)
			{
				BroomBoostLandingCooldownRemaining = FMath::Max(BroomBoostTuning.LandingCooldown, 0.0f);
			}

			BroomBoostLandingCooldownRemaining = FMath::Max(0.0f, BroomBoostLandingCooldownRemaining - DeltaSeconds);
			if (BroomBoostLandingCooldownRemaining <= 0.0f)
			{
				bBroomBoostNeedsLandingCooldown = false;
				bBroomBoostAvailable = true;
			}
		}
		else
		{
			BroomBoostLandingCooldownRemaining = 0.0f;
			bBroomBoostAvailable = true;
		}

		return;
	}

	if (!bBroomBoostActive)
	{
		return;
	}

	BroomBoostTimeRemaining = FMath::Max(0.0f, BroomBoostTimeRemaining - DeltaSeconds);
	if (BroomBoostTimeRemaining <= 0.0f)
	{
		EndBroomBoost();
		return;
	}

	const float ControlMultiplier = GetBroomBoostControlMultiplier();
	const FVector DesiredHorizontalVelocity = BroomBoostDirection * FMath::Max(BroomBoostTuning.ForwardSpeed * ControlMultiplier, 0.0f);
	const FVector CurrentHorizontalVelocity(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f);
	const float EffectiveLerpSpeed = FMath::Max(BroomBoostTuning.VelocityLerpSpeed * ControlMultiplier, 0.0f);
	const FVector NewHorizontalVelocity = FMath::VInterpTo(CurrentHorizontalVelocity, DesiredHorizontalVelocity, DeltaSeconds, EffectiveLerpSpeed);
	MoveComp->Velocity.X = NewHorizontalVelocity.X;
	MoveComp->Velocity.Y = NewHorizontalVelocity.Y;
	MoveComp->Velocity.Z = FMath::Max(MoveComp->Velocity.Z, BroomBoostTuning.MinVerticalVelocityDuringBoost);
}

void AWizardStaffWizardCharacter::UpdateMegaStaffBrew(float DeltaSeconds)
{
	if (!HasAuthority())
	{
		if (bMegaStaffBrewActive)
		{
			MegaStaffTimeRemaining = FMath::Max(0.0f, MegaStaffTimeRemaining - DeltaSeconds);
		}
		UpdateMegaStaffVisual(DeltaSeconds);
		return;
	}

	if (!bMegaStaffBrewActive)
	{
		UpdateMegaStaffVisual(DeltaSeconds);
		return;
	}

	MegaStaffTimeRemaining = FMath::Max(0.0f, MegaStaffTimeRemaining - DeltaSeconds);
	UpdateMegaStaffVisual(DeltaSeconds);
	SyncReplicatedMegaStaffStateFromAuthority();

	if (!bMegaStaffExpireWarningShown && MegaStaffExpireWarningTime > 0.0f && MegaStaffTimeRemaining <= MegaStaffExpireWarningTime)
	{
		bMegaStaffExpireWarningShown = true;
		const FString WarningMessage = FString::Printf(TEXT("P%d Mega Staff fading! %.0fs left"), GetDebugPlayerIndex() + 1, MegaStaffTimeRemaining);
		AWizardStaffHUD::PushGameplayMessage(this, WarningMessage, FColor::Orange, 2.0f, EWizardHudMessageCategory::Powerup);
		if (GEngine && bMegaStaffShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
		{
			GEngine->AddOnScreenDebugMessage(
				2861 + GetDebugPlayerIndex(),
				1.4f,
				FColor::Orange,
				WarningMessage);
		}
	}

	if (MegaStaffTimeRemaining <= 0.0f)
	{
		ExpireMegaStaffBrew();
	}
}

void AWizardStaffWizardCharacter::ExpireMegaStaffBrew()
{
	const int32 TempSegmentsToRemove = MegaStaffTemporarySegmentsRemaining;
	const bool bShouldRemoveSegments = bMegaStaffRemoveTemporarySegmentsOnExpire;
	ClearMegaStaffBrew(bShouldRemoveSegments);

	const FString ExpireMessage = FString::Printf(TEXT("P%d Mega Staff ended%s"), GetDebugPlayerIndex() + 1, bShouldRemoveSegments && TempSegmentsToRemove > 0 ? TEXT(": temp staff faded") : TEXT(": temp staff already gone"));
	AWizardStaffHUD::PushGameplayMessage(this, ExpireMessage, FColor::Orange, 2.2f, EWizardHudMessageCategory::Powerup);
	if (AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr)
	{
		const int32 PlayerIndex = GameMode->GetPlayerIndexForWizard(this);
		GameMode->PublishReplicatedGameplayEvent(
			EWizardReplicatedGameplayEventType::MegaStaffExpired,
			PlayerIndex == INDEX_NONE ? TEXT("Mega Staff ended") : FString::Printf(TEXT("P%d Mega Staff ended"), PlayerIndex + 1),
			PlayerIndex,
			INDEX_NONE,
			static_cast<float>(TempSegmentsToRemove),
			false);
	}
	if (GEngine && bMegaStaffShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(
			2860 + GetDebugPlayerIndex(),
			1.3f,
			FColor::Orange,
			ExpireMessage);
	}
}

void AWizardStaffWizardCharacter::UpdateMegaStaffVisual(float DeltaSeconds)
{
	(void)DeltaSeconds;

	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Pulse = 0.5f + (FMath::Sin(TimeSeconds * 9.5f) * 0.5f);
	const float MegaMarkerScale = bMegaStaffBrewActive
		? FMath::Max(MegaStaffMarkerScaleMultiplier + (Pulse * MegaStaffMarkerPulseScale), 1.0f)
		: 1.0f;

	if (PlayerMarkerMesh)
	{
		PlayerMarkerMesh->SetRelativeScale3D(FVector(
			PlayerMarkerScale.X * MegaMarkerScale,
			PlayerMarkerScale.Y * MegaMarkerScale,
			PlayerMarkerScale.Z));
	}

	FLinearColor MarkerColor = CurrentPlayerMarkerColor;
	if (bMegaStaffBrewActive)
	{
		MarkerColor = FLinearColor::LerpUsingHSV(CurrentPlayerMarkerColor, MegaStaffVisualColor, 0.52f + (Pulse * 0.34f));
	}
	if (PlayerMarkerMaterialInstance)
	{
		PlayerMarkerMaterialInstance->SetVectorParameterValue(TEXT("Color"), MarkerColor);
		PlayerMarkerMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), MarkerColor);
		PlayerMarkerMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), bMegaStaffBrewActive ? MarkerColor : FLinearColor::Black);
	}
	if (HatMaterialInstance)
	{
		FLinearColor HatColor = CurrentHatColor;
		if (bMegaStaffBrewActive)
		{
			HatColor = FLinearColor::LerpUsingHSV(CurrentHatColor, MegaStaffVisualColor, 0.38f);
		}
		HatMaterialInstance->SetVectorParameterValue(TEXT("Color"), HatColor);
		HatMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), HatColor);
	}
}

bool AWizardStaffWizardCharacter::CanBroomBoost() const
{
	const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	return BroomBoostTuning.bEnableBroomBoost
		&& MoveComp
		&& MoveComp->IsFalling()
		&& bBroomBoostAvailable
		&& !bBroomBoostActive;
}

void AWizardStaffWizardCharacter::ActivateBroomBoost()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	BroomBoostDirection = GetActorForwardVector();
	BroomBoostDirection.Z = 0.0f;
	if (!BroomBoostDirection.Normalize())
	{
		BroomBoostDirection = FVector::ForwardVector;
	}

	const float DisorderThreshold = FMath::Max(BroomBoostTuning.HighSloshDisorderThreshold, 0.0f);
	const float FullDisorderSlosh = FMath::Max(BroomBoostTuning.FullDisorderSlosh, DisorderThreshold + 1.0f);
	const float RawDisorderAlpha = BroomBoostTuning.bEnableHighSloshDisorder && ManaSlosh >= DisorderThreshold
		? FMath::Clamp((ManaSlosh - DisorderThreshold) / (FullDisorderSlosh - DisorderThreshold), 0.0f, 1.0f)
		: 0.0f;
	float DisorderAlpha = RawDisorderAlpha > 0.0f || (BroomBoostTuning.bEnableHighSloshDisorder && ManaSlosh >= DisorderThreshold)
		? FMath::Clamp(FMath::Lerp(FMath::Clamp(BroomBoostTuning.MinDisorderAlphaAtThreshold, 0.0f, 1.0f), 1.0f, RawDisorderAlpha), 0.0f, 1.0f)
		: 0.0f;
	if (DisorderAlpha > 0.0f)
	{
		const float DisorderChance = FMath::Clamp(BroomBoostTuning.HighSloshDisorderChanceAtFullSlosh, 0.0f, 1.0f) * DisorderAlpha;
		if (FMath::FRand() > DisorderChance)
		{
			DisorderAlpha = 0.0f;
		}
	}

	if (DisorderAlpha > 0.0f)
	{
		const float MaxDisorderYaw = FMath::Clamp(BroomBoostTuning.MaxDisorderYawDegrees, 0.0f, 180.0f);
		const float YawJitter = FMath::FRandRange(-MaxDisorderYaw, MaxDisorderYaw) * DisorderAlpha;
		BroomBoostDirection = FRotator(0.0f, YawJitter, 0.0f).RotateVector(BroomBoostDirection).GetSafeNormal2D();
		if (BroomBoostDirection.IsNearlyZero())
		{
			BroomBoostDirection = FVector::ForwardVector;
		}
	}

	bBroomBoostAvailable = false;
	bBroomBoostActive = true;
	bBroomBoostNeedsLandingCooldown = true;
	BroomBoostLandingCooldownRemaining = 0.0f;
	BroomBoostTimeRemaining = FMath::Max(BroomBoostTuning.BoostDuration, 0.01f);
	bReplicatedBroomBoostActive = true;
	SetBroomBoostVisualActive(true);
	ForceNetUpdate();

	const float ControlMultiplier = GetBroomBoostControlMultiplier();
	const FVector SideDirection = FVector::CrossProduct(FVector::UpVector, BroomBoostDirection).GetSafeNormal();
	const float MaxDisorderSideImpulse = FMath::Max(BroomBoostTuning.MaxDisorderSideImpulse, 0.0f);
	const float MaxDisorderVerticalImpulse = FMath::Max(BroomBoostTuning.MaxDisorderVerticalImpulse, 0.0f);
	const FVector DisorderSideImpulse = SideDirection * (FMath::FRandRange(-MaxDisorderSideImpulse, MaxDisorderSideImpulse) * DisorderAlpha);
	const float DisorderVerticalImpulse = FMath::FRandRange(-MaxDisorderVerticalImpulse, MaxDisorderVerticalImpulse) * DisorderAlpha;
	const FVector LaunchVelocity =
		(BroomBoostDirection * FMath::Max(BroomBoostTuning.InitialForwardBoost * ControlMultiplier, 0.0f))
		+ DisorderSideImpulse
		+ FVector(0.0f, 0.0f, (BroomBoostTuning.InitialUpwardBoost * ControlMultiplier) + DisorderVerticalImpulse);
	LaunchCharacter(LaunchVelocity, false, false);

	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RecordTelemetryBroomBoostUsed(this);
		}
	}

	if (GEngine && BroomBoostTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(
			2600 + GetDebugPlayerIndex(),
			1.0f,
			DisorderAlpha > 0.0f ? FColor::Orange : FColor::Yellow,
			DisorderAlpha > 0.0f
				? FString::Printf(TEXT("P%d disorderly broom boost! Mana Slosh %.0f"), GetDebugPlayerIndex() + 1, ManaSlosh)
				: FString::Printf(TEXT("P%d broom boost!"), GetDebugPlayerIndex() + 1));
	}
}

void AWizardStaffWizardCharacter::EndBroomBoost()
{
	if (!bBroomBoostActive && BroomBoostTimeRemaining <= 0.0f)
	{
		SetBroomBoostVisualActive(false);
		return;
	}

	bBroomBoostActive = false;
	BroomBoostTimeRemaining = 0.0f;
	if (HasAuthority())
	{
		bReplicatedBroomBoostActive = false;
	}
	SetBroomBoostVisualActive(false);
	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

void AWizardStaffWizardCharacter::SetBroomBoostVisualActive(bool bNewActive)
{
	if (BroomHandleMesh)
	{
		BroomHandleMesh->SetRelativeLocation(BroomBoostTuning.BroomLocalLocation);
		BroomHandleMesh->SetRelativeScale3D(BroomBoostTuning.HandleScale);
		BroomHandleMesh->SetHiddenInGame(!bNewActive);
		BroomHandleMesh->SetVisibility(bNewActive, true);
	}

	if (BroomBristleMesh)
	{
		BroomBristleMesh->SetRelativeLocation(BroomBoostTuning.BristleLocalLocation);
		BroomBristleMesh->SetRelativeScale3D(BroomBoostTuning.BristleScale);
		BroomBristleMesh->SetHiddenInGame(!bNewActive);
		BroomBristleMesh->SetVisibility(bNewActive, true);
	}
}

float AWizardStaffWizardCharacter::GetBroomBoostControlMultiplier() const
{
	const float SloshPenalty = GetManaSloshAlpha() * FMath::Clamp(BroomBoostTuning.SloshControlPenaltyAtMax, 0.0f, 0.95f);
	const float HeftPenalty = FMath::Clamp(
		static_cast<float>(GetHeavyStaffSegmentCount()) * FMath::Max(BroomBoostTuning.StaffHeftControlPenaltyPerHeavySegment, 0.0f),
		0.0f,
		FMath::Clamp(BroomBoostTuning.MaxStaffHeftControlPenalty, 0.0f, 0.95f));
	const float MinimumMultiplier = FMath::Clamp(BroomBoostTuning.MinControlMultiplier, 0.05f, 1.0f);
	return FMath::Clamp(1.0f - SloshPenalty - HeftPenalty, MinimumMultiplier, 1.0f);
}

bool AWizardStaffWizardCharacter::IsPrototypeInputBlockedForLocalInput() const
{
	if (bPrototypeInputLocked || bReplicatedPrototypeInputLocked)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Standalone)
	{
		return false;
	}

	const AWizardStaffGameState* WizardGameState = World->GetGameState<AWizardStaffGameState>();
	if (!WizardGameState)
	{
		return false;
	}

	const EWizardPartyMatchState ReplicatedPartyState = WizardGameState->GetReplicatedPartyMatchState();
	const EWizardTrialState ReplicatedTrialState = WizardGameState->GetReplicatedActiveTrialState();
	if (ReplicatedPartyState == EWizardPartyMatchState::Trial)
	{
		return ReplicatedTrialState != EWizardTrialState::Active;
	}
	if (ReplicatedPartyState == EWizardPartyMatchState::FinalRound)
	{
		return ReplicatedTrialState != EWizardTrialState::Active;
	}
	if (ReplicatedPartyState == EWizardPartyMatchState::Results)
	{
		return true;
	}

	return false;
}

void AWizardStaffWizardCharacter::UpdateSloshMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	MoveComp->MaxWalkSpeed = WalkSpeed * ReplicatedCauldronMovementMultipliers.Z * ReplicatedCauldronVialEffectMultipliers.X * ReplicatedCauldronBankingMovementMultipliers.X;
	MoveComp->MaxAcceleration = MaxAcceleration * GetManaSloshAccelerationMultiplier() * ReplicatedCauldronMovementMultipliers.W * ReplicatedCauldronVialEffectMultipliers.Y * ReplicatedCauldronBankingMovementMultipliers.Y;
	MoveComp->BrakingDecelerationWalking = BrakingDecelerationWalking * GetManaSloshBrakingMultiplier() * ReplicatedCauldronMovementMultipliers.Y;
	MoveComp->GroundFriction = DefaultCauldronGroundFriction * ReplicatedCauldronMovementMultipliers.X;
	MoveComp->RotationRate.Yaw = DefaultCauldronRotationRateYaw * ReplicatedCauldronTurningMultiplier;
	MoveComp->JumpZVelocity = HopVelocity;
}

void AWizardStaffWizardCharacter::SetCauldronHazardMovementMultipliers(float FrictionMultiplier, float BrakingMultiplier, float SpeedMultiplier, float AccelerationMultiplier, float TurningMultiplier)
{
	if (!HasAuthority())
	{
		return;
	}

	const FVector4 SafeMultipliers(
		FMath::Clamp(FrictionMultiplier, 0.05f, 1.0f),
		FMath::Clamp(BrakingMultiplier, 0.05f, 1.0f),
		FMath::Clamp(SpeedMultiplier, 0.05f, 1.0f),
		FMath::Clamp(AccelerationMultiplier, 0.05f, 1.0f));
	const float SafeTurningMultiplier = FMath::Clamp(TurningMultiplier, 0.05f, 1.0f);
	if (ReplicatedCauldronMovementMultipliers == SafeMultipliers && FMath::IsNearlyEqual(ReplicatedCauldronTurningMultiplier, SafeTurningMultiplier))
	{
		return;
	}
	ReplicatedCauldronMovementMultipliers = SafeMultipliers;
	ReplicatedCauldronTurningMultiplier = SafeTurningMultiplier;
	UpdateSloshMovementSettings();
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SetCauldronBankingMovementMultipliers(bool bBanking, float SpeedMultiplier, float AccelerationMultiplier)
{
	if (!HasAuthority())
	{
		return;
	}

	const FVector2D SafeMultipliers(
		FMath::Clamp(SpeedMultiplier, 0.05f, 1.0f),
		FMath::Clamp(AccelerationMultiplier, 0.05f, 1.0f));
	if (bReplicatedCauldronBanking == bBanking && ReplicatedCauldronBankingMovementMultipliers.Equals(SafeMultipliers))
	{
		return;
	}

	bReplicatedCauldronBanking = bBanking;
	ReplicatedCauldronBankingMovementMultipliers = SafeMultipliers;
	UpdateSloshMovementSettings();
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SetCauldronVialEffectState(EWizardCauldronVialType ActiveVial, int32 VialCount, float SpeedMultiplier, float AccelerationMultiplier, float BonkKnockbackMultiplier, float InstabilityMultiplier)
{
	if (!HasAuthority())
	{
		return;
	}

	const FVector SafeMultipliers(
		FMath::Max(SpeedMultiplier, 0.05f),
		FMath::Max(AccelerationMultiplier, 0.05f),
		FMath::Max(BonkKnockbackMultiplier, 0.05f));
	const int32 SafeVialCount = FMath::Max(VialCount, 0);
	const float SafeInstabilityMultiplier = FMath::Max(InstabilityMultiplier, 1.0f);
	if (ReplicatedActiveCauldronVial == ActiveVial && ReplicatedCauldronVialCount == SafeVialCount && ReplicatedCauldronVialEffectMultipliers.Equals(SafeMultipliers) && FMath::IsNearlyEqual(ReplicatedCauldronVialInstabilityMultiplier, SafeInstabilityMultiplier))
	{
		return;
	}

	ReplicatedActiveCauldronVial = ActiveVial;
	ReplicatedCauldronVialCount = SafeVialCount;
	ReplicatedCauldronVialEffectMultipliers = SafeMultipliers;
	ReplicatedCauldronVialInstabilityMultiplier = SafeInstabilityMultiplier;
	UpdateSloshMovementSettings();
	ForceNetUpdate();
}

bool AWizardStaffWizardCharacter::ApplyCauldronVialInstabilityStress(float InstabilityMultiplier, float& OutBaseStress, float& OutFinalStress)
{
	OutBaseStress = 0.0f;
	OutFinalStress = 0.0f;
	if (!HasAuthority() || !StaffComponent)
	{
		return false;
	}

	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	if (GameMode && GameMode->ShouldDisablePartyHallBonkStress())
	{
		return false;
	}

	const float SafeInstabilityMultiplier = FMath::Max(InstabilityMultiplier, 1.0f);
	OutBaseStress = StaffComponent->StressTuning.StressGainedPerBonk * BonkTuning.HitStressMultiplier;
	OutFinalStress = OutBaseStress * StaffComponent->GetStressMultiplier() * GetStaffStressEffectMultiplier() * SafeInstabilityMultiplier;
	const bool bSnapped = StaffComponent->AddStaffStress(OutBaseStress * SafeInstabilityMultiplier, TEXT("CauldronVialInstability"));
	PlayBonkStressFeedback(OutFinalStress, bSnapped, 1);
	return true;
}

void AWizardStaffWizardCharacter::TriggerCauldronSlipperySkid(float SloshAlpha, float Duration, float MinimumImpulse, float MaximumImpulse)
{
	if (!HasAuthority())
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || !MoveComp->IsMovingOnGround())
	{
		return;
	}

	FVector HorizontalVelocity(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f);
	FVector TravelDirection = HorizontalVelocity.SizeSquared() >= FMath::Square(60.0f) ? HorizontalVelocity.GetSafeNormal2D() : GetActorForwardVector().GetSafeNormal2D();
	if (TravelDirection.IsNearlyZero())
	{
		return;
	}

	if (CauldronSlipperySkidRemainingTime <= 0.0f)
	{
		CauldronSlipperySkidDirection = TravelDirection;
		const float Impulse = FMath::Lerp(FMath::Max(MinimumImpulse, 0.0f), FMath::Max(MaximumImpulse, MinimumImpulse), FMath::Clamp(SloshAlpha, 0.0f, 1.0f));
		MoveComp->Velocity += CauldronSlipperySkidDirection * Impulse;
	}

	CauldronSlipperySkidRemainingTime = FMath::Max(CauldronSlipperySkidRemainingTime, FMath::Max(Duration, 0.0f));
}

bool AWizardStaffWizardCharacter::UpdateCauldronSlipperySkid(float DeltaSeconds)
{
	if (!HasAuthority() || CauldronSlipperySkidRemainingTime <= 0.0f)
	{
		return false;
	}

	CauldronSlipperySkidRemainingTime = FMath::Max(CauldronSlipperySkidRemainingTime - FMath::Max(DeltaSeconds, 0.0f), 0.0f);
	if (CauldronSlipperySkidRemainingTime <= 0.0f)
	{
		CauldronSlipperySkidDirection = FVector::ZeroVector;
		return false;
	}

	return true;
}

void AWizardStaffWizardCharacter::ApplyCauldronSlipperyGlide(float DeltaSeconds, float Acceleration)
{
	if (!HasAuthority() || DeltaSeconds <= 0.0f || CauldronSlipperySkidDirection.IsNearlyZero())
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || !MoveComp->IsMovingOnGround())
	{
		return;
	}

	MoveComp->Velocity += CauldronSlipperySkidDirection * FMath::Max(Acceleration, 0.0f) * DeltaSeconds;
}

void AWizardStaffWizardCharacter::ClearCauldronSlipperySkid()
{
	CauldronSlipperySkidRemainingTime = 0.0f;
	CauldronSlipperySkidDirection = FVector::ZeroVector;
}

void AWizardStaffWizardCharacter::ApplyCauldronStickyTetherReel(float DeltaSeconds)
{
	if (!HasAuthority() || !bReplicatedCauldronStickyTethered || bBroomBoostActive || DeltaSeconds <= 0.0f)
	{
		return;
	}

	FVector ToAnchor = ReplicatedCauldronStickyTetherAnchor - GetActorLocation();
	ToAnchor.Z = 0.0f;
	const float Distance = ToAnchor.Size();
	if (Distance <= CauldronStickyTetherSlackDistance || !ToAnchor.Normalize())
	{
		return;
	}

	const float ReelSpeed = FMath::Clamp((Distance - CauldronStickyTetherSlackDistance) * 0.55f, 28.0f, 115.0f);
	SetActorLocation(GetActorLocation() + (ToAnchor * ReelSpeed * DeltaSeconds), true);
}

void AWizardStaffWizardCharacter::SetCauldronStickyTetherState(bool bNewTethered, const FVector& AnchorLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	const FVector SafeAnchor = bNewTethered ? AnchorLocation : FVector::ZeroVector;
	if (bReplicatedCauldronStickyTethered == bNewTethered && ReplicatedCauldronStickyTetherAnchor.Equals(SafeAnchor, 1.0f))
	{
		return;
	}

	bReplicatedCauldronStickyTethered = bNewTethered;
	ReplicatedCauldronStickyTetherAnchor = SafeAnchor;
	UpdateCauldronStickyTetherVisual();
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SetCauldronCurseState(bool bCursed, float RemainingTime)
{
	if (!HasAuthority())
	{
		return;
	}

	const float SafeRemainingTime = bCursed ? FMath::GridSnap(FMath::Max(RemainingTime, 0.0f), 0.1f) : 0.0f;
	if (bReplicatedCauldronCursed == bCursed && FMath::IsNearlyEqual(ReplicatedCauldronCurseRemainingTime, SafeRemainingTime))
	{
		if (!bCursed)
		{
			// Reset paths may repeat a clear after a visual attachment changed locally.
			// Re-applying the readable state makes the stale orb/aura harmless.
			OnRep_ReplicatedCauldronState();
		}
		return;
	}
	bReplicatedCauldronCursed = bCursed;
	ReplicatedCauldronCurseRemainingTime = SafeRemainingTime;
	if (bCursed)
	{
		bCauldronCurseOrbAttachedToLooseSegment = false;
		AttachCauldronCurseOrbToTopStaffSegment();
	}
	OnRep_ReplicatedCauldronState();
	ForceNetUpdate();
}

void AWizardStaffWizardCharacter::SetCauldronCurseVisualRelativeLocation(const FVector& RelativeLocation)
{
	if (HasAuthority() && CauldronCurseOrbMesh)
	{
		ReplicatedCauldronCurseOrbRelativeLocation = RelativeLocation;
		OnRep_ReplicatedCauldronState();
		ForceNetUpdate();
	}
}

void AWizardStaffWizardCharacter::AttachCauldronCurseOrbToLooseSegment(AActor* LooseSegment)
{
	if (!CauldronCurseOrbMesh || !IsValid(LooseSegment) || !LooseSegment->GetRootComponent())
	{
		return;
	}

	CauldronCurseOrbMesh->AttachToComponent(LooseSegment->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	CauldronCurseOrbMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 62.0f));
	CauldronCurseOrbMesh->SetHiddenInGame(false);
	CauldronCurseOrbMesh->SetVisibility(true);
	bCauldronCurseOrbAttachedToLooseSegment = true;
}

void AWizardStaffWizardCharacter::DetachCauldronCurseOrbAtWorldLocation(const FVector& WorldLocation)
{
	if (!CauldronCurseOrbMesh)
	{
		return;
	}

	CauldronCurseOrbMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	CauldronCurseOrbMesh->SetWorldLocation(WorldLocation);
	CauldronCurseOrbMesh->SetHiddenInGame(false);
	CauldronCurseOrbMesh->SetVisibility(true);
	bCauldronCurseOrbAttachedToLooseSegment = true;
}

void AWizardStaffWizardCharacter::ClearCauldronCurseOrbLooseSegmentAttachment()
{
	if (!CauldronCurseOrbMesh || !StaffRoot)
	{
		return;
	}

	CauldronCurseOrbMesh->AttachToComponent(StaffRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	CauldronCurseOrbMesh->SetRelativeLocation(ReplicatedCauldronCurseOrbRelativeLocation);
	bCauldronCurseOrbAttachedToLooseSegment = false;
}

void AWizardStaffWizardCharacter::RefreshCauldronCurseOrbAttachment()
{
	if (bReplicatedCauldronCursed && !bCauldronCurseOrbAttachedToLooseSegment)
	{
		AttachCauldronCurseOrbToTopStaffSegment();
	}
}

FVector AWizardStaffWizardCharacter::GetCauldronCurseOrbWorldLocation() const
{
	return CauldronCurseOrbMesh ? CauldronCurseOrbMesh->GetComponentLocation() : GetActorLocation();
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedCauldronState()
{
	if (!CauldronCurseOrbMesh)
	{
		return;
	}
	CauldronCurseOrbMesh->SetHiddenInGame(!bReplicatedCauldronCursed);
	CauldronCurseOrbMesh->SetVisibility(bReplicatedCauldronCursed);
	for (UStaticMeshComponent* AuraMesh : CauldronCurseAuraMeshes)
	{
		if (AuraMesh)
		{
			AuraMesh->SetHiddenInGame(!bReplicatedCauldronCursed);
			AuraMesh->SetVisibility(bReplicatedCauldronCursed, true);
		}
	}
	if (bReplicatedCauldronCursed && !bCauldronCurseOrbAttachedToLooseSegment)
	{
		AttachCauldronCurseOrbToTopStaffSegment();
	}
	else
	{
		ClearCauldronCurseOrbLooseSegmentAttachment();
		CauldronCurseOrbMesh->SetRelativeLocation(ReplicatedCauldronCurseOrbRelativeLocation);
	}
}

void AWizardStaffWizardCharacter::AttachCauldronCurseOrbToTopStaffSegment()
{
	if (!CauldronCurseOrbMesh || !StaffRoot)
	{
		return;
	}

	if (UStaticMeshComponent* TopSegment = StaffComponent ? StaffComponent->GetTopStaffSegmentMesh() : nullptr)
	{
		// Segment anchors vary with the runtime staff rebuild, so use the visible
		// segment's bounds instead of its local origin. This keeps the curse obvious
		// above the staff tip for both short and grown staffs.
		if (CauldronCurseOrbMesh->GetAttachParent())
		{
			CauldronCurseOrbMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		}

		const FVector OrbLocation = TopSegment->Bounds.Origin + FVector(0.0f, 0.0f, TopSegment->Bounds.BoxExtent.Z + 52.0f);
		CauldronCurseOrbMesh->SetWorldLocation(OrbLocation);
		CauldronCurseOrbMesh->SetWorldRotation(FRotator::ZeroRotator);
		return;
	}

	if (CauldronCurseOrbMesh->GetAttachParent() != StaffRoot)
	{
		CauldronCurseOrbMesh->AttachToComponent(StaffRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	CauldronCurseOrbMesh->SetRelativeLocation(ReplicatedCauldronCurseOrbRelativeLocation + FVector(0.0f, 0.0f, 45.0f));
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedCauldronMovement()
{
	UpdateSloshMovementSettings();
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedCauldronVialState()
{
	ReplicatedCauldronVialCount = FMath::Max(ReplicatedCauldronVialCount, 0);
	ReplicatedCauldronVialEffectMultipliers.X = FMath::Max(ReplicatedCauldronVialEffectMultipliers.X, 0.05f);
	ReplicatedCauldronVialEffectMultipliers.Y = FMath::Max(ReplicatedCauldronVialEffectMultipliers.Y, 0.05f);
	ReplicatedCauldronVialEffectMultipliers.Z = FMath::Max(ReplicatedCauldronVialEffectMultipliers.Z, 0.05f);
	ReplicatedCauldronVialInstabilityMultiplier = FMath::Max(ReplicatedCauldronVialInstabilityMultiplier, 1.0f);
	UpdateSloshMovementSettings();
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedCauldronBanking()
{
	ReplicatedCauldronBankingMovementMultipliers.X = FMath::Clamp(ReplicatedCauldronBankingMovementMultipliers.X, 0.05f, 1.0f);
	ReplicatedCauldronBankingMovementMultipliers.Y = FMath::Clamp(ReplicatedCauldronBankingMovementMultipliers.Y, 0.05f, 1.0f);
	UpdateSloshMovementSettings();
}

void AWizardStaffWizardCharacter::OnRep_ReplicatedCauldronStickyTether()
{
	UpdateCauldronStickyTetherVisual();
}

void AWizardStaffWizardCharacter::UpdateCauldronStickyTether(float DeltaSeconds)
{
	if (HasAuthority() && bReplicatedCauldronStickyTethered)
	{
		FVector ToAnchor = ReplicatedCauldronStickyTetherAnchor - GetActorLocation();
		ToAnchor.Z = 0.0f;
		const float Distance = ToAnchor.Size();
		if (bBroomBoostActive && Distance >= CauldronStickyTetherBreakDistance)
		{
			SetCauldronStickyTetherState(false);
		}
		else if (!bBroomBoostActive && Distance > CauldronStickyTetherSlackDistance && ToAnchor.Normalize())
		{
			if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
			{
				const float DesiredReelSpeed = FMath::Clamp((Distance - CauldronStickyTetherSlackDistance) * 1.1f, 75.0f, 245.0f);
				const FVector CurrentHorizontalVelocity(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f);
				const FVector DesiredHorizontalVelocity = ToAnchor * DesiredReelSpeed;
				const FVector ReeledVelocity = FMath::VInterpTo(CurrentHorizontalVelocity, DesiredHorizontalVelocity, DeltaSeconds, 4.5f);
				MoveComp->Velocity.X = ReeledVelocity.X;
				MoveComp->Velocity.Y = ReeledVelocity.Y;
			}
		}
	}

	UpdateCauldronStickyTetherVisual();
}

void AWizardStaffWizardCharacter::UpdateCauldronStickyTetherVisual()
{
	if (!CauldronStickyTetherMesh)
	{
		return;
	}

	const bool bShouldShow = bReplicatedCauldronStickyTethered;
	CauldronStickyTetherMesh->SetHiddenInGame(!bShouldShow);
	CauldronStickyTetherMesh->SetVisibility(bShouldShow);
	if (!bShouldShow)
	{
		return;
	}

	const FVector Anchor = ReplicatedCauldronStickyTetherAnchor + FVector(0.0f, 0.0f, 5.0f);
	const FVector WizardPoint = GetActorLocation() + FVector(0.0f, 0.0f, 28.0f);
	const FVector TetherVector = WizardPoint - Anchor;
	const float Length = FMath::Max(TetherVector.Size(), 1.0f);
	const FVector Direction = TetherVector / Length;
	CauldronStickyTetherMesh->SetWorldLocation((Anchor + WizardPoint) * 0.5f);
	CauldronStickyTetherMesh->SetWorldRotation(FQuat::FindBetweenNormals(FVector::UpVector, Direction));
	CauldronStickyTetherMesh->SetWorldScale3D(FVector(0.045f, 0.045f, Length / 100.0f));

}

void AWizardStaffWizardCharacter::UpdateCauldronCurseVisual(float DeltaSeconds)
{
	if (!bReplicatedCauldronCursed || !CauldronCurseOrbMesh)
	{
		return;
	}
	if (!bCauldronCurseOrbAttachedToLooseSegment)
	{
		AttachCauldronCurseOrbToTopStaffSegment();
	}
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : DeltaSeconds;
	const float Pulse = 0.58f + (FMath::Sin(Time * 12.0f) * 0.09f);
	CauldronCurseOrbMesh->SetWorldScale3D(FVector(Pulse));
	UpdateCauldronCurseAuraVisual(DeltaSeconds);
}

void AWizardStaffWizardCharacter::UpdateCauldronCurseAuraVisual(float DeltaSeconds)
{
	const bool bShowAura = bReplicatedCauldronCursed && !bCauldronCurseOrbAttachedToLooseSegment;
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (int32 AuraIndex = 0; AuraIndex < CauldronCurseAuraMeshes.Num(); ++AuraIndex)
	{
		UStaticMeshComponent* AuraMesh = CauldronCurseAuraMeshes[AuraIndex];
		if (!AuraMesh)
		{
			continue;
		}
		AuraMesh->SetHiddenInGame(!bShowAura);
		AuraMesh->SetVisibility(bShowAura, true);
		if (!bShowAura)
		{
			continue;
		}

		const float Phase = (Time * 2.3f) + (AuraIndex * (2.0f * UE_PI / FMath::Max(CauldronCurseAuraMeshes.Num(), 1)));
		const float Radius = 54.0f + (AuraIndex % 3) * 24.0f + FMath::Sin(Phase * 1.7f) * 12.0f;
		const float Height = 42.0f + (AuraIndex % 4) * 23.0f + FMath::Sin(Phase * 2.4f) * 26.0f;
		AuraMesh->SetWorldLocation(GetActorLocation() + FVector(FMath::Cos(Phase) * Radius, FMath::Sin(Phase) * Radius, Height));
		const float Scale = 0.14f + ((FMath::Sin(Phase * 2.8f) + 1.0f) * 0.075f);
		AuraMesh->SetWorldScale3D(FVector(Scale));
	}
}

void AWizardStaffWizardCharacter::UpdateSloshVisuals(float DeltaSeconds)
{
	const float SloshAlpha = GetReadableManaSloshAlpha() * ManaTuning.SloshVisualIntensity;
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float WobblePhase = TimeSeconds * ManaTuning.SloshVisualWobbleFrequency + static_cast<float>(GetDebugPlayerIndex());
	const float LeanRoll = FMath::Sin(WobblePhase) * ManaTuning.SloshVisualLeanDegrees * SloshAlpha;
	const float LeanPitch = FMath::Cos(WobblePhase * 0.73f) * ManaTuning.SloshVisualLeanDegrees * 0.35f * SloshAlpha;
	const FRotator LeanRotation(LeanPitch, 0.0f, LeanRoll);
	const FRotator CombinedLeanRotation = LeanRotation + HitReactionVisualRotation;
	const float StaffYaw = FMath::Sin(WobblePhase * 0.83f) * ManaTuning.SloshStaffWobbleDegrees * 0.45f * SloshAlpha;
	const float StaffRoll = FMath::Cos(WobblePhase * 1.17f) * ManaTuning.SloshStaffWobbleDegrees * SloshAlpha;
	SloshStaffVisualRotation = FRotator(0.0f, StaffYaw, StaffRoll);

	if (RobeMesh)
	{
		RobeMesh->SetRelativeRotation(CombinedLeanRotation);
	}
	if (HatMesh)
	{
		HatMesh->SetRelativeRotation(CombinedLeanRotation);
	}
	if (FaceMesh)
	{
		FaceMesh->SetRelativeRotation(CombinedLeanRotation);
	}
}

void AWizardStaffWizardCharacter::UpdateBonkAttack(float DeltaSeconds)
{
	if (bStaffClashActive)
	{
		return;
	}

	if (bQuickBonkHitResolved)
	{
		return;
	}

	QuickBonkImpactTimeRemaining = FMath::Max(0.0f, QuickBonkImpactTimeRemaining - DeltaSeconds);
	if (QuickBonkImpactTimeRemaining <= 0.0f)
	{
		QuickBonkHitCountThisSwing = PerformQuickBonkHitDetection();
		ResolveQuickBonkHit();
	}
}

void AWizardStaffWizardCharacter::UpdateBonkVisual(float DeltaSeconds)
{
	if (!StaffRoot)
	{
		return;
	}

	FRotator SnapCueRotation = FRotator::ZeroRotator;
	if (StaffSnapCueTimeRemaining > 0.0f)
	{
		StaffSnapCueTimeRemaining = FMath::Max(0.0f, StaffSnapCueTimeRemaining - DeltaSeconds);
		const float CueAlpha = FMath::Clamp(StaffSnapCueTimeRemaining / FMath::Max(StaffSnapCueDurationSeconds, 0.01f), 0.0f, 1.0f);
		const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		const float MegaScale = bLastStaffSnapCueWasMegaTemporarySegment ? 1.25f : 1.0f;
		const float ShakeStrength = FMath::InterpEaseOut(0.0f, 1.0f, CueAlpha, 2.0f) * MegaScale;
		SnapCueRotation = FRotator(
			FMath::Sin(TimeSeconds * 44.0f) * StaffSnapCuePitchDegrees * ShakeStrength,
			0.0f,
			FMath::Cos(TimeSeconds * 39.0f) * StaffSnapCueRollDegrees * ShakeStrength);
	}

	if (bStaffClashActive)
	{
		const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		const float MashShake = FMath::Clamp(static_cast<float>(StaffClashMashCount) * 0.22f, 0.0f, 5.0f);
		const float ClashWobble = FMath::Sin(TimeSeconds * 28.0f) * MashShake;
		const FRotator ClashRotation(BonkTuning.StrikeEndPitchDegrees, ClashWobble * 0.25f, ClashWobble);
		StaffRoot->SetRelativeRotation(StaffRootDefaultRelativeRotation + SloshStaffVisualRotation + HitReactionStaffVisualRotation + ClashRotation + SnapCueRotation);
		return;
	}

	if (bReplicatedCauldronBanking)
	{
		const FRotator BankingRotation(BonkTuning.StrikeEndPitchDegrees, 0.0f, 0.0f);
		StaffRoot->SetRelativeRotation(StaffRootDefaultRelativeRotation + SloshStaffVisualRotation + HitReactionStaffVisualRotation + BankingRotation + SnapCueRotation);
		return;
	}

	if (QuickBonkVisualTimeRemaining <= 0.0f)
	{
		StaffRoot->SetRelativeRotation(StaffRootDefaultRelativeRotation + SloshStaffVisualRotation + HitReactionStaffVisualRotation + SnapCueRotation);
		return;
	}

	QuickBonkVisualTimeRemaining = FMath::Max(0.0f, QuickBonkVisualTimeRemaining - DeltaSeconds);
	const float Duration = FMath::Max(GetQuickBonkVisualDuration(), 0.01f);
	const float SwingAlpha = 1.0f - (QuickBonkVisualTimeRemaining / Duration);
	const float StrikeAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, SwingAlpha, FMath::Max(BonkTuning.StrikeEaseExponent, 0.1f));
	const float StrikePitch = FMath::Lerp(BonkTuning.StrikeStartPitchDegrees, BonkTuning.StrikeEndPitchDegrees, StrikeAlpha);
	const float SideWobble = FMath::Sin(SwingAlpha * UE_PI) * BonkTuning.StrikeSideWobbleDegrees;
	const FRotator SwingRotation(StrikePitch, SideWobble * 0.35f, SideWobble);

	StaffRoot->SetRelativeRotation(StaffRootDefaultRelativeRotation + SloshStaffVisualRotation + HitReactionStaffVisualRotation + SwingRotation + SnapCueRotation);
}

void AWizardStaffWizardCharacter::ResolveQuickBonkHit()
{
	if (bQuickBonkHitResolved)
	{
		return;
	}

	bQuickBonkHitResolved = true;
	if (const UWorld* World = GetWorld())
	{
		LastQuickBonkTime = World->GetTimeSeconds();
	}
	else
	{
		LastQuickBonkTime += GetQuickBonkCooldown();
	}
	const int32 HitCount = QuickBonkHitCountThisSwing;

	AddQuickBonkStressForHitCount(HitCount, FName(HitCount > 0 ? TEXT("QuickBonkHit") : TEXT("QuickBonkWhiff")));

	QuickBonkImpactTimeRemaining = 0.0f;
	QuickBonkHitCountThisSwing = 0;
	QuickBonkHitWizardsThisSwing.Reset();
	QuickBonkHitIngredientsThisSwing.Reset();
}

void AWizardStaffWizardCharacter::AddQuickBonkStressForHitCount(int32 HitCount, FName StressSource)
{
	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	const bool bSkipPartyHallStress = GameMode && GameMode->ShouldDisablePartyHallBonkStress();
	if (!StaffComponent || bSkipPartyHallStress)
	{
		return;
	}

	const float StressMultiplier = HitCount > 0 ? BonkTuning.HitStressMultiplier : BonkTuning.WhiffStressMultiplier;
	const float BaseStress = StaffComponent->StressTuning.StressGainedPerBonk * StressMultiplier;
	const float StressAdded = BaseStress * StaffComponent->GetStressMultiplier() * GetStaffStressEffectMultiplier();
	const FName SafeStressSource = StressSource.IsNone()
		? FName(HitCount > 0 ? TEXT("QuickBonkHit") : TEXT("QuickBonkWhiff"))
		: StressSource;
	const bool bSnapped = StaffComponent->AddStaffStress(BaseStress, SafeStressSource);
	PlayBonkStressFeedback(StressAdded, bSnapped, HitCount);
}

int32 AWizardStaffWizardCharacter::PerformQuickBonkHitDetection()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	TSet<AWizardStaffWizardCharacter*> HitWizards;
	TSet<AWizardStaffCauldronIngredient*> HitIngredients;
	AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>();
	if (GameMode)
	{
		if (GameMode->HandleCauldronQuickBonk(this, AuthoritativeQuickBonkSequence))
		{
			++QuickBonkHitCountThisSwing;
		}
		TArray<AWizardStaffCauldronIngredient*> ArcHitIngredients;
		GameMode->GatherCauldronIngredientsInBonkArc(this, ArcHitIngredients);
		for (AWizardStaffCauldronIngredient* Ingredient : ArcHitIngredients)
		{
			if (Ingredient && !QuickBonkHitIngredientsThisSwing.Contains(Ingredient))
			{
				QuickBonkHitIngredientsThisSwing.Add(Ingredient);
				HitIngredients.Add(Ingredient);
			}
		}
	}

	UBoxComponent* StaffCollisionBox = StaffComponent ? StaffComponent->GetStaffCollisionBox() : nullptr;
	if (!StaffCollisionBox || StaffCollisionBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		if (GameMode)
		{
			for (AWizardStaffCauldronIngredient* Ingredient : HitIngredients)
			{
				if (GameMode->HandleCauldronIngredientBonked(this, Ingredient))
				{
					++QuickBonkHitCountThisSwing;
				}
			}
		}

		if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
		{
			GEngine->AddOnScreenDebugMessage(2600 + GetDebugPlayerIndex(), 0.75f, FColor::Orange, TEXT("Quick Bonk: staff collision inactive."));
		}
		return 0;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WizardQuickBonk), false, this);
	QueryParams.AddIgnoredActor(this);

	bool bHitReadyBell = false;

	TArray<FOverlapResult> Overlaps;
	const FVector ContactExtent = StaffCollisionBox->GetScaledBoxExtent() + FVector(FMath::Max(BonkTuning.StaffContactPadding, 0.0f));
	World->OverlapMultiByObjectType(
		Overlaps,
		StaffCollisionBox->GetComponentLocation(),
		StaffCollisionBox->GetComponentQuat(),
		ObjectQueryParams,
		FCollisionShape::MakeBox(ContactExtent),
		QueryParams);

	if (BonkTuning.bShowDebug)
	{
		const FColor DebugColor = Overlaps.Num() > 0 ? FColor::Green : FColor::Orange;
		DrawDebugBox(World, StaffCollisionBox->GetComponentLocation(), ContactExtent, StaffCollisionBox->GetComponentQuat(), DebugColor, false, 0.22f, 0, 3.0f);
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		UPrimitiveComponent* HitComponent = Overlap.GetComponent();
		if (!bHitReadyBell && HitComponent && HitComponent->ComponentHasTag(AWizardStaffPartyHall::GetReadyBellComponentTag()))
		{
			bHitReadyBell = true;
			if (GameMode)
			{
				GameMode->NotifyPartyHallReadyBellBonked(this);
			}
			continue;
		}

		AWizardStaffWizardCharacter* HitWizard = Cast<AWizardStaffWizardCharacter>(Overlap.GetActor());
		if (HitWizard && HitWizard != this && !QuickBonkHitWizardsThisSwing.Contains(HitWizard))
		{
			QuickBonkHitWizardsThisSwing.Add(HitWizard);
			HitWizards.Add(HitWizard);
		}

		AWizardStaffCauldronIngredient* HitIngredient = Cast<AWizardStaffCauldronIngredient>(Overlap.GetActor());
		if (HitIngredient && !QuickBonkHitIngredientsThisSwing.Contains(HitIngredient))
		{
			QuickBonkHitIngredientsThisSwing.Add(HitIngredient);
			HitIngredients.Add(HitIngredient);
		}
	}

	if (GameMode)
	{
		TArray<AWizardStaffCauldronIngredient*> GeometryHitIngredients;
		GameMode->GatherCauldronIngredientsInBonkBox(
			StaffCollisionBox->GetComponentLocation(),
			StaffCollisionBox->GetComponentQuat(),
			ContactExtent,
			GeometryHitIngredients);
		for (AWizardStaffCauldronIngredient* Ingredient : GeometryHitIngredients)
		{
			if (Ingredient && !QuickBonkHitIngredientsThisSwing.Contains(Ingredient))
			{
				QuickBonkHitIngredientsThisSwing.Add(Ingredient);
				HitIngredients.Add(Ingredient);
			}
		}

		for (AWizardStaffCauldronIngredient* Ingredient : HitIngredients)
		{
			if (GameMode->HandleCauldronIngredientBonked(this, Ingredient))
			{
				++QuickBonkHitCountThisSwing;
			}
		}
	}

	for (AWizardStaffWizardCharacter* HitWizard : HitWizards)
	{
		if (ApplyBonkToTarget(HitWizard))
		{
			++QuickBonkHitCountThisSwing;
		}
	}

	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		const FString DebugText = FString::Printf(
			TEXT("P%d Quick Bonk impact: staff contact %.0f, knockback %.0f, strike %.2fs, cooldown %.2fs, hits %d%s"),
			GetDebugPlayerIndex() + 1,
			GetQuickBonkRange(),
			GetQuickBonkKnockbackStrength(),
			GetQuickBonkImpactDelay(),
			GetQuickBonkCooldown(),
			QuickBonkHitCountThisSwing,
			bHitReadyBell ? TEXT(" + bell") : TEXT(""));
		GEngine->AddOnScreenDebugMessage(2600 + GetDebugPlayerIndex(), 0.75f, FColor::Yellow, DebugText);
	}

	return QuickBonkHitCountThisSwing;
}

bool AWizardStaffWizardCharacter::TryStartStaffClashWith(AWizardStaffWizardCharacter* TargetWizard)
{
	if (!CanStartStaffClashWith(TargetWizard))
	{
		return false;
	}

	const float Duration = FMath::Max(StaffClashTuning.ClashDuration, 0.1f);
	StaffClashOpponent = TargetWizard;
	StaffClashLockedLocation = GetActorLocation();
	StaffClashTimeRemaining = Duration;
	StaffClashMashCount = 1;
	bStaffClashActive = true;
	bStaffClashResolver = true;
	bQuickBonkHitResolved = true;
	QuickBonkImpactTimeRemaining = 0.0f;
	QuickBonkVisualTimeRemaining = 0.0f;
	QuickBonkHitCountThisSwing = 0;
	QuickBonkHitWizardsThisSwing.Reset();
	QuickBonkHitIngredientsThisSwing.Reset();

	TargetWizard->StaffClashOpponent = this;
	TargetWizard->StaffClashLockedLocation = TargetWizard->GetActorLocation();
	TargetWizard->StaffClashTimeRemaining = Duration;
	TargetWizard->StaffClashMashCount = 1;
	TargetWizard->bStaffClashActive = true;
	TargetWizard->bStaffClashResolver = false;
	TargetWizard->bQuickBonkHitResolved = true;
	TargetWizard->QuickBonkImpactTimeRemaining = 0.0f;
	TargetWizard->QuickBonkVisualTimeRemaining = 0.0f;
	TargetWizard->QuickBonkHitCountThisSwing = 0;
	TargetWizard->QuickBonkHitWizardsThisSwing.Reset();
	TargetWizard->QuickBonkHitIngredientsThisSwing.Reset();

	EndBroomBoost();
	TargetWizard->EndBroomBoost();
	StopJumping();
	TargetWizard->StopJumping();
	LockStaffClashPosition();
	TargetWizard->LockStaffClashPosition();

	const FString ClashMessage = FString::Printf(TEXT("STAFF CLASH! P%d vs P%d - mash Bonk!"), GetDebugPlayerIndex() + 1, TargetWizard->GetDebugPlayerIndex() + 1);
	AWizardStaffHUD::PushGameplayMessage(this, ClashMessage, FColor::Cyan, 1.3f, EWizardHudMessageCategory::Gameplay);
	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RecordTelemetryStaffClashStarted(this, TargetWizard);
		}
	}
	if (GEngine && StaffClashTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(2900 + GetDebugPlayerIndex(), 1.5f, FColor::Cyan, ClashMessage);
	}

	SyncReplicatedStaffClashStateFromAuthority(true);
	TargetWizard->SyncReplicatedStaffClashStateFromAuthority(true);
	return true;
}

bool AWizardStaffWizardCharacter::CanStartStaffClashWith(const AWizardStaffWizardCharacter* TargetWizard) const
{
	if (!StaffClashTuning.bEnableStaffClash || !TargetWizard || TargetWizard == this)
	{
		return false;
	}

	if (bStaffClashActive || TargetWizard->bStaffClashActive)
	{
		return false;
	}

	if (bQuickBonkHitResolved || TargetWizard->bQuickBonkHitResolved)
	{
		return false;
	}

	const float TimingWindow = FMath::Max(StaffClashTuning.ClashTimingWindow, TargetWizard->StaffClashTuning.ClashTimingWindow);
	if (TargetWizard->QuickBonkImpactTimeRemaining > TimingWindow)
	{
		return false;
	}

	FVector ToTarget = TargetWizard->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	FVector ToThis = -ToTarget;
	if (!ToTarget.Normalize() || !ToThis.Normalize())
	{
		return false;
	}

	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
	FVector TargetForward = TargetWizard->GetActorForwardVector();
	TargetForward.Z = 0.0f;
	if (!Forward.Normalize() || !TargetForward.Normalize())
	{
		return false;
	}

	const float ThisFacingDot = FVector::DotProduct(Forward, ToTarget);
	const float TargetFacingDot = FVector::DotProduct(TargetForward, ToThis);
	const float ForwardDot = FVector::DotProduct(Forward, TargetForward);
	const float FacingThreshold = FMath::Min(StaffClashTuning.FacingDotThreshold, TargetWizard->StaffClashTuning.FacingDotThreshold);
	const float OpposingThreshold = FMath::Max(StaffClashTuning.OpposingForwardDotThreshold, TargetWizard->StaffClashTuning.OpposingForwardDotThreshold);

	return ThisFacingDot >= FacingThreshold
		&& TargetFacingDot >= FacingThreshold
		&& ForwardDot <= OpposingThreshold;
}

void AWizardStaffWizardCharacter::UpdateStaffClash(float DeltaSeconds)
{
	if (!bStaffClashActive)
	{
		return;
	}

	if (!HasAuthority())
	{
		LockStaffClashPosition();
		StaffClashTimeRemaining = FMath::Max(0.0f, StaffClashTimeRemaining - DeltaSeconds);
		return;
	}

	AWizardStaffWizardCharacter* Opponent = StaffClashOpponent.Get();
	if (!Opponent || !Opponent->bStaffClashActive || Opponent->StaffClashOpponent.Get() != this)
	{
		ClearStaffClashState();
		return;
	}

	LockStaffClashPosition();
	StaffClashTimeRemaining = FMath::Max(0.0f, StaffClashTimeRemaining - DeltaSeconds);
	SyncReplicatedStaffClashStateFromAuthority();
	if (bStaffClashResolver && StaffClashTimeRemaining <= 0.0f)
	{
		ResolveStaffClash();
	}
}

void AWizardStaffWizardCharacter::ResolveStaffClash()
{
	if (!bStaffClashActive || !bStaffClashResolver)
	{
		return;
	}

	AWizardStaffWizardCharacter* Opponent = StaffClashOpponent.Get();
	if (!Opponent)
	{
		ClearStaffClashState();
		return;
	}

	AWizardStaffWizardCharacter* Winner = nullptr;
	AWizardStaffWizardCharacter* Loser = nullptr;
	const int32 MyMashCount = StaffClashMashCount;
	const int32 OpponentMashCount = Opponent->StaffClashMashCount;
	if (MyMashCount > OpponentMashCount)
	{
		Winner = this;
		Loser = Opponent;
	}
	else if (OpponentMashCount > MyMashCount)
	{
		Winner = Opponent;
		Loser = this;
	}

	ClearStaffClashState();
	Opponent->ClearStaffClashState();

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	LastQuickBonkTime = Now;
	Opponent->LastQuickBonkTime = Now;

	if (Winner && Loser)
	{
		const int32 WinnerMashCount = Winner == this ? MyMashCount : OpponentMashCount;
		const int32 LoserMashCount = Winner == this ? OpponentMashCount : MyMashCount;
		const FString ResultMessage = FString::Printf(
			TEXT("P%d wins Staff Clash %d-%d!"),
			Winner->GetDebugPlayerIndex() + 1,
			WinnerMashCount,
			LoserMashCount);
		AWizardStaffHUD::PushGameplayMessage(this, ResultMessage, FColor::Yellow, 1.8f, EWizardHudMessageCategory::Gameplay);
		Winner->ApplyStaffClashWinningBonkToTarget(Loser, WinnerMashCount, LoserMashCount);
	}
	else
	{
		if (UWorld* World = GetWorld())
		{
			if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
			{
				GameMode->RecordTelemetryStaffClashTied(this, Opponent);
			}
		}
		AWizardStaffHUD::PushGameplayMessage(this, TEXT("Staff Clash tie! Both bonks fizzle."), FColor::Cyan, 1.5f, EWizardHudMessageCategory::Gameplay);
	}
}

void AWizardStaffWizardCharacter::ClearStaffClashState(bool bSyncReplicatedState)
{
	StaffClashOpponent.Reset();
	StaffClashLockedLocation = FVector::ZeroVector;
	StaffClashTimeRemaining = 0.0f;
	StaffClashMashCount = 0;
	LastNetworkStaffClashMashTime = -1000.0f;
	bStaffClashActive = false;
	bStaffClashResolver = false;
	if (bSyncReplicatedState)
	{
		SyncReplicatedStaffClashStateFromAuthority(true);
	}
}

void AWizardStaffWizardCharacter::RegisterStaffClashMash()
{
	if (!HasAuthority() || !bStaffClashActive)
	{
		return;
	}

	if (const UWorld* World = GetWorld(); World && World->GetNetMode() != NM_Standalone)
	{
		const float Now = World->GetTimeSeconds();
		if (Now < LastNetworkStaffClashMashTime + NetworkStaffClashMashMinIntervalSeconds)
		{
			return;
		}
		LastNetworkStaffClashMashTime = Now;
	}

	StaffClashMashCount = FMath::Min(StaffClashMashCount + 1, NetworkStaffClashMashCountLimit);
	SyncReplicatedStaffClashStateFromAuthority();
}

void AWizardStaffWizardCharacter::LockStaffClashPosition()
{
	if (!bStaffClashActive)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	SetActorLocation(StaffClashLockedLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AWizardStaffWizardCharacter::ApplyStaffClashWinningBonkToTarget(AWizardStaffWizardCharacter* TargetWizard, int32 WinnerMashCount, int32 LoserMashCount)
{
	if (!TargetWizard)
	{
		return;
	}

	FVector KnockbackDirection = TargetWizard->GetActorLocation() - GetActorLocation();
	KnockbackDirection.Z = 0.0f;
	if (!KnockbackDirection.Normalize())
	{
		KnockbackDirection = GetActorForwardVector();
		KnockbackDirection.Z = 0.0f;
		KnockbackDirection.Normalize();
	}

	const float BaseWinnerMultiplier = FMath::Max(StaffClashTuning.WinnerKnockbackMultiplier, 1.0f);
	const float SegmentMultiplierBonus = static_cast<float>(FMath::Max(GetStaffSegmentCount(), 0)) * FMath::Max(StaffClashTuning.WinnerKnockbackMultiplierPerStaffSegment, 0.0f);
	const float MaxWinnerMultiplier = FMath::Max(StaffClashTuning.WinnerMaxKnockbackMultiplier, BaseWinnerMultiplier);
	const float WinnerKnockbackMultiplier = FMath::Min(BaseWinnerMultiplier + SegmentMultiplierBonus, MaxWinnerMultiplier);
	const float KnockbackStrength = GetQuickBonkKnockbackStrength() * WinnerKnockbackMultiplier;
	const float UpwardBoost = BonkTuning.UpwardBoost * GetPartyHallBonkMultiplier() * FMath::Max(StaffClashTuning.WinnerUpwardBoostMultiplier, 0.0f);
	TargetWizard->ApplyBonkReaction(KnockbackDirection, KnockbackStrength, UpwardBoost, GetStaffSegmentCount());
	PlayBonkHitFeedback(TargetWizard, KnockbackDirection);
	AddQuickBonkStressForHitCount(1, FName(TEXT("StaffClashWin")));

	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RecordTelemetryStaffClashWon(this, TargetWizard);
			GameMode->NotifyCauldronWizardBonked(this, TargetWizard);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Wizard %d won Staff Clash over Wizard %d (%d-%d), staff %d, clash multiplier %.2f, knockback %.0f."),
		GetDebugPlayerIndex(),
		TargetWizard->GetDebugPlayerIndex(),
		WinnerMashCount,
		LoserMashCount,
		GetStaffSegmentCount(),
		WinnerKnockbackMultiplier,
		KnockbackStrength);
}

bool AWizardStaffWizardCharacter::ApplyBonkToTarget(AWizardStaffWizardCharacter* TargetWizard)
{
	if (!TargetWizard)
	{
		return false;
	}

	if (!TargetWizard->CanReceiveBonkOrSpellHits())
	{
		return false;
	}

	const bool bAllowStaffClash = HasAuthority();
	if (bAllowStaffClash && TryStartStaffClashWith(TargetWizard))
	{
		return false;
	}

	FVector KnockbackDirection = TargetWizard->GetActorLocation() - GetActorLocation();
	KnockbackDirection.Z = 0.0f;
	if (!KnockbackDirection.Normalize())
	{
		KnockbackDirection = GetActorForwardVector();
		KnockbackDirection.Z = 0.0f;
		KnockbackDirection.Normalize();
	}

	const float KnockbackStrength = GetQuickBonkKnockbackStrength();
	TargetWizard->ApplyBonkReaction(KnockbackDirection, KnockbackStrength, BonkTuning.UpwardBoost * GetPartyHallBonkMultiplier(), GetStaffSegmentCount());
	PlayBonkHitFeedback(TargetWizard, KnockbackDirection);
	if (UWorld* MutableWorld = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = MutableWorld->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->RecordTelemetryBonkLanded(this, TargetWizard);
			GameMode->NotifyCauldronWizardBonked(this, TargetWizard);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Wizard %d bonked Wizard %d with range %.0f and knockback %.0f."),
		GetDebugPlayerIndex(),
		TargetWizard->GetDebugPlayerIndex(),
		GetQuickBonkRange(),
		KnockbackStrength);

	return true;
}

void AWizardStaffWizardCharacter::PlayBonkHitFeedback(AWizardStaffWizardCharacter* TargetWizard, const FVector& KnockbackDirection) const
{
	if (!TargetWizard)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector ImpactLocation = TargetWizard->GetActorLocation() + FVector(0.0f, 0.0f, 52.0f);
	if (BonkTuning.BonkHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BonkTuning.BonkHitSound, ImpactLocation);
	}

	if (BonkTuning.bShowImpactFeedback)
	{
		const float Radius = FMath::Max(BonkTuning.ImpactFeedbackRadius, 1.0f);
		const float Duration = FMath::Max(BonkTuning.ImpactFeedbackDuration, 0.0f);
		DrawDebugSphere(World, ImpactLocation, Radius, 16, FColor::Yellow, false, Duration, 0, 4.0f);
		DrawDebugDirectionalArrow(World, ImpactLocation, ImpactLocation + (KnockbackDirection * (Radius * 2.4f)), Radius * 0.6f, FColor::Orange, false, Duration, 0, 4.0f);
		DrawDebugString(World, ImpactLocation + FVector(0.0f, 0.0f, Radius * 0.75f), TEXT("BONK!"), nullptr, FColor::Yellow, Duration, true);
	}
}

void AWizardStaffWizardCharacter::PlayBonkStressFeedback(float StressAdded, bool bSnapped, int32 HitCount) const
{
	if (!StaffComponent)
	{
		return;
	}

	const float StressPercent = StaffComponent->GetStaffStressAlpha() * 100.0f;
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		const FColor MessageColor = bSnapped ? FColor::Red : (StressPercent >= 70.0f ? FColor::Orange : FColor::Yellow);
		const FString DebugText = FString::Printf(
			TEXT("P%d Bonk %s | Stress +%.0f | Staff %.0f%%"),
			GetDebugPlayerIndex() + 1,
			HitCount > 0 ? TEXT("hit") : TEXT("whiff"),
			StressAdded,
			StressPercent);
		GEngine->AddOnScreenDebugMessage(2650 + GetDebugPlayerIndex(), 0.9f, MessageColor, DebugText);
	}

	if (BonkTuning.bShowImpactFeedback)
	{
		if (UWorld* World = GetWorld())
		{
			const FVector FeedbackLocation = StaffRoot ? StaffRoot->GetComponentLocation() + FVector(0.0f, 0.0f, 70.0f) : GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
			const FColor FeedbackColor = bSnapped ? FColor::Red : (StressPercent >= 70.0f ? FColor::Orange : FColor::Yellow);
			DrawDebugSphere(World, FeedbackLocation, 18.0f + (StaffComponent->GetStaffStressAlpha() * 18.0f), 12, FeedbackColor, false, BonkTuning.ImpactFeedbackDuration, 0, 2.0f);
		}
	}
}

void AWizardStaffWizardCharacter::HandleStaffSegmentSnapped(int32 NewSegmentCount, float RemainingStress)
{
	const bool bWasMegaTemporarySegment = bMegaStaffBrewActive && MegaStaffTemporarySegmentsRemaining > 0;
	NotifyMegaStaffSegmentSnapped();
	if (bManaSloshLocked)
	{
		LockedManaSlosh = FMath::Max(0.0f, LockedManaSlosh - ManaTuning.SloshReducedOnStaffSnap);
		ManaSlosh = LockedManaSlosh;
	}
	else
	{
		ManaSlosh = FMath::Max(0.0f, ManaSlosh - ManaTuning.SloshReducedOnStaffSnap);
	}
	SyncReplicatedStaffSegmentCountFromAuthority();
	SyncReplicatedManaSloshFromAuthority(true);
	SyncReplicatedStaffStressFromAuthority(true);
	SyncReplicatedStaffSnapCueFromAuthority(NewSegmentCount, bWasMegaTemporarySegment);

	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->NotifyCauldronVialSegmentSnapped(this, StaffComponent ? StaffComponent->GetLastRemovedSegmentTag() : NAME_None);
			GameMode->NotifyCauldronCursedWizardStaffSnapped(this);
		}

		if (World->GetNetMode() != NM_Standalone)
		{
			if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
			{
				const int32 PlayerIndex = GameMode->GetPlayerIndexForWizard(this);
				const FString SnapMessage = PlayerIndex == INDEX_NONE
					? FString::Printf(TEXT("Staff segment snapped! Staff %d"), FMath::Max(NewSegmentCount, 0))
					: FString::Printf(
						TEXT("P%d snapped %s! Staff %d"),
						PlayerIndex + 1,
						bWasMegaTemporarySegment ? TEXT("a Mega segment") : TEXT("a segment"),
						FMath::Max(NewSegmentCount, 0));
				GameMode->PublishReplicatedGameplayEvent(
					EWizardReplicatedGameplayEventType::StaffSegmentSnapped,
					SnapMessage,
					PlayerIndex,
					INDEX_NONE,
					static_cast<float>(FMath::Max(NewSegmentCount, 0)),
					false);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Wizard %d staff snapped. Segments %d, remaining stress %.1f, slosh %.1f."),
		GetDebugPlayerIndex(),
		NewSegmentCount,
		RemainingStress,
		ManaSlosh);
}

void AWizardStaffWizardCharacter::DrawSloshDebug() const
{
	if (!bShowWizardDebug || !GEngine || !AWizardStaffHUD::IsFullDebugMode(this))
	{
		return;
	}

	const int32 PlayerIndex = GetDebugPlayerIndex();
	const FString DebugText = FString::Printf(
		TEXT("P%d Staff %d | Mana Slosh %.0f/%.0f | Move %.0f%% Turn %.0f%% Accel %.0f%% Brake %.0f%% React %.0f%% Heft M%.0f%% T%.0f%% Bonk %.2fs"),
		PlayerIndex + 1,
		GetStaffSegmentCount(),
		ManaSlosh,
		ManaTuning.MaxSlosh,
		GetManaSloshMovementMultiplier() * 100.0f,
		GetManaSloshTurnMultiplier() * 100.0f,
		GetManaSloshAccelerationMultiplier() * 100.0f,
		GetManaSloshBrakingMultiplier() * 100.0f,
		GetHitReactionInputMultiplier() * 100.0f,
		GetStaffHeftMovementMultiplier() * 100.0f,
		GetStaffHeftTurnMultiplier() * 100.0f,
		GetQuickBonkCooldown());

	GEngine->AddOnScreenDebugMessage(2200 + PlayerIndex, 0.0f, FColor::Cyan, DebugText);
}

float AWizardStaffWizardCharacter::GetManaSloshMovementMultiplier() const
{
	return FMath::Clamp(1.0f - (GetManaSloshAlpha() * ManaTuning.MovementPenaltyPerSlosh), ManaTuning.MinMovementMultiplier, 1.0f);
}

float AWizardStaffWizardCharacter::GetManaSloshTurnMultiplier() const
{
	return FMath::Clamp(1.0f - (GetManaSloshAlpha() * ManaTuning.TurnPenaltyPerSlosh), ManaTuning.MinTurnMultiplier, 1.0f);
}

float AWizardStaffWizardCharacter::GetManaSloshAccelerationMultiplier() const
{
	return FMath::Clamp(1.0f - (GetManaSloshAlpha() * ManaTuning.AccelerationPenaltyPerSlosh), ManaTuning.MinAccelerationMultiplier, 1.0f);
}

float AWizardStaffWizardCharacter::GetManaSloshBrakingMultiplier() const
{
	return FMath::Clamp(1.0f - (GetManaSloshAlpha() * ManaTuning.BrakingPenaltyPerSlosh), ManaTuning.MinBrakingMultiplier, 1.0f);
}

float AWizardStaffWizardCharacter::GetHitReactionInputMultiplier() const
{
	if (bStaffClashActive)
	{
		return FMath::Clamp(StaffClashTuning.InputLockMultiplier, 0.0f, 1.0f);
	}

	if (HitKnockdownTimeRemaining > 0.0f)
	{
		return HitReactionTuning.KnockdownControlMultiplier;
	}

	if (HitControlLossTimeRemaining > 0.0f)
	{
		return HitReactionTuning.StrongHitControlMultiplier;
	}

	if (HitStumbleTimeRemaining > 0.0f)
	{
		return HitReactionTuning.StumbleControlMultiplier;
	}

	if (HitRecoveryTimeRemaining > 0.0f)
	{
		const float RecoveryAlpha = FMath::Clamp(HitRecoveryTimeRemaining / FMath::Max(HitReactionTuning.RecoveryTime, 0.01f), 0.0f, 1.0f);
		return FMath::Lerp(1.0f, HitReactionTuning.RecoveryControlMultiplier, RecoveryAlpha);
	}

	return 1.0f;
}

int32 AWizardStaffWizardCharacter::GetHeavyStaffSegmentCount() const
{
	return FMath::Max(GetStaffSegmentCount() - FMath::Max(StaffHeftTuning.SegmentCountBeforeHeftPenalty, 0), 0);
}

float AWizardStaffWizardCharacter::GetPartyHallBonkMultiplier() const
{
	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	return GameMode ? GameMode->GetPartyHallBonkKnockbackMultiplier() : 1.0f;
}

void AWizardStaffWizardCharacter::SetDebugSloshAlpha(float SloshAlpha)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetDebugSloshAlpha ignored on non-authority wizard. Use server smoke-test helpers from the listen-server host."));
		return;
	}

	ManaSlosh = FMath::Clamp(SloshAlpha, 0.0f, 1.0f) * FMath::Max(ManaTuning.MaxSlosh, 1.0f);
	SloshTurnCarryDegreesPerSecond = 0.0f;
	StumbleCooldownRemaining = 0.0f;
	UpdateSloshMovementSettings();
	SyncReplicatedManaSloshFromAuthority(true);

	if (GEngine)
	{
		const FString DebugText = FString::Printf(TEXT("P%d debug Mana Slosh set to %.0f/%.0f"), GetDebugPlayerIndex() + 1, ManaSlosh, ManaTuning.MaxSlosh);
		GEngine->AddOnScreenDebugMessage(2300 + GetDebugPlayerIndex(), 1.0f, FColor::Cyan, DebugText);
	}
}

int32 AWizardStaffWizardCharacter::GetDebugPlayerIndex() const
{
	if (const AWizardStaffPlayerState* WizardPlayerState = GetPlayerState<AWizardStaffPlayerState>())
	{
		const int32 DisplaySlot = WizardPlayerState->GetWizardDisplaySlot();
		if (DisplaySlot != INDEX_NONE)
		{
			return DisplaySlot;
		}
	}

	if (const APlayerState* CurrentPlayerState = GetPlayerState())
	{
		return FMath::Max(CurrentPlayerState->GetPlayerId(), 0);
	}

	return 0;
}
