#include "WizardStaffCauldronDepositArc.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

AWizardStaffCauldronDepositArc::AWizardStaffCauldronDepositArc()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);
	SetActorEnableCollision(false);

	ArcMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArcMesh"));
	SetRootComponent(ArcMesh);
	ArcMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArcMesh->SetGenerateOverlapEvents(false);
	ArcMesh->SetCanEverAffectNavigation(false);
	ArcMesh->SetCastShadow(false);
}

void AWizardStaffCauldronDepositArc::BeginPlay()
{
	Super::BeginPlay();
	ApplyPresentation();
}

void AWizardStaffCauldronDepositArc::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bPresentationReady || !ArcMesh)
	{
		return;
	}

	ElapsedTime = FMath::Max(ElapsedTime + FMath::Max(DeltaSeconds, 0.0f), 0.0f);
	const float Duration = FMath::Max(ReplicatedDuration, 0.01f);
	const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
	const float ArcHeight = FMath::Max(FVector::Dist2D(ReplicatedStartLocation, ReplicatedEndLocation) * 0.24f, 105.0f);
	const FVector ArcLocation = FMath::Lerp(ReplicatedStartLocation, ReplicatedEndLocation, Alpha) + FVector(0.0f, 0.0f, FMath::Sin(Alpha * UE_PI) * ArcHeight);
	const FRotator ArcRotation = ReplicatedStartRotation + FRotator(Alpha * 420.0f, Alpha * 220.0f, Alpha * 150.0f);
	SetActorLocationAndRotation(ArcLocation, ArcRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (Alpha < 1.0f)
	{
		return;
	}

	ArcMesh->SetHiddenInGame(true);
	if (HasAuthority())
	{
		Destroy();
	}
}

void AWizardStaffCauldronDepositArc::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWizardStaffCauldronDepositArc, ReplicatedSegmentMesh);
	DOREPLIFETIME(AWizardStaffCauldronDepositArc, ReplicatedStartLocation);
	DOREPLIFETIME(AWizardStaffCauldronDepositArc, ReplicatedStartRotation);
	DOREPLIFETIME(AWizardStaffCauldronDepositArc, ReplicatedStartScale);
	DOREPLIFETIME(AWizardStaffCauldronDepositArc, ReplicatedEndLocation);
	DOREPLIFETIME(AWizardStaffCauldronDepositArc, ReplicatedSegmentColor);
	DOREPLIFETIME(AWizardStaffCauldronDepositArc, ReplicatedDuration);
}

void AWizardStaffCauldronDepositArc::InitializeDepositArc(UStaticMesh* SegmentMesh, const FTransform& StartTransform, const FVector& EndLocation, const FLinearColor& SegmentColor, float DurationSeconds)
{
	if (!HasAuthority() || !SegmentMesh)
	{
		return;
	}

	ReplicatedSegmentMesh = SegmentMesh;
	ReplicatedStartLocation = StartTransform.GetLocation();
	ReplicatedStartRotation = StartTransform.Rotator();
	ReplicatedStartScale = StartTransform.GetScale3D();
	ReplicatedEndLocation = EndLocation;
	ReplicatedSegmentColor = SegmentColor;
	ReplicatedDuration = FMath::Max(DurationSeconds, 0.05f);
	ElapsedTime = 0.0f;
	ApplyPresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronDepositArc::OnRep_DepositArcPresentation()
{
	ElapsedTime = 0.0f;
	ApplyPresentation();
}

void AWizardStaffCauldronDepositArc::ApplyPresentation()
{
	if (!ArcMesh || !ReplicatedSegmentMesh)
	{
		return;
	}

	ArcMesh->SetStaticMesh(ReplicatedSegmentMesh);
	ArcMesh->SetRelativeScale3D(ReplicatedStartScale);
	ArcMesh->SetHiddenInGame(false);
	ArcMesh->SetVisibility(true, true);
	SetActorLocationAndRotation(ReplicatedStartLocation, ReplicatedStartRotation, false, nullptr, ETeleportType::TeleportPhysics);
	if (UMaterialInstanceDynamic* Material = ArcMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), ReplicatedSegmentColor);
		Material->SetVectorParameterValue(TEXT("BaseColor"), ReplicatedSegmentColor);
	}
	bPresentationReady = true;
}