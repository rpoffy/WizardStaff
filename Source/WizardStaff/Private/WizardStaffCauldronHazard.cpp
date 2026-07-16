#include "WizardStaffCauldronHazard.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AWizardStaffCauldronHazard::AWizardStaffCauldronHazard()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	HazardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HazardMesh"));
	SetRootComponent(HazardMesh);
	HazardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HazardMesh->SetGenerateOverlapEvents(false);

	EruptionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EruptionMesh"));
	EruptionMesh->SetupAttachment(HazardMesh);
	EruptionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EruptionMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CylinderMesh.Succeeded())
	{
		HazardMesh->SetStaticMesh(CylinderMesh.Object);
		EruptionMesh->SetStaticMesh(CylinderMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		HazardMesh->SetMaterial(0, BasicMaterial.Object);
		EruptionMesh->SetMaterial(0, BasicMaterial.Object);
	}
}

void AWizardStaffCauldronHazard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float BaseScale = HazardRadius / 50.0f;
	if (EruptionTimeRemaining > 0.0f)
	{
		EruptionTimeRemaining = FMath::Max(EruptionTimeRemaining - DeltaSeconds, 0.0f);
		const float Alpha = 1.0f - (EruptionTimeRemaining / 0.65f);
		const FVector TargetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 24.0f);
		const FVector ArcLocation = FMath::Lerp(EruptionOrigin, TargetLocation, Alpha) + FVector(0.0f, 0.0f, FMath::Sin(Alpha * UE_PI) * 210.0f);
		EruptionMesh->SetWorldLocation(ArcLocation);
		EruptionMesh->SetWorldScale3D(FVector(0.42f + (Alpha * 0.35f), 0.42f + (Alpha * 0.35f), 0.72f));
		HazardMesh->SetHiddenInGame(true);
		HazardMesh->SetVisibility(false);
		EruptionMesh->SetHiddenInGame(false);
		EruptionMesh->SetVisibility(true);
		return;
	}

	EruptionMesh->SetHiddenInGame(true);
	EruptionMesh->SetVisibility(false);
	HazardMesh->SetHiddenInGame(false);
	HazardMesh->SetVisibility(true);
	const float Wobble = HazardType == EWizardCauldronHazardType::Slippery
		? 1.0f + (FMath::Sin(Time * 5.2f) * 0.10f)
		: 1.0f + (FMath::Sin(Time * 2.6f) * 0.06f);
	HazardMesh->SetRelativeScale3D(FVector(BaseScale * Wobble, BaseScale * (2.0f - Wobble), 0.035f));
	HazardMesh->SetRelativeRotation(FRotator(0.0f, Time * (HazardType == EWizardCauldronHazardType::Slippery ? 22.0f : 8.0f), 0.0f));
}

void AWizardStaffCauldronHazard::BeginPlay()
{
	Super::BeginPlay();
	ApplyPresentation();
}

void AWizardStaffCauldronHazard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HazardMesh)
	{
		HazardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Super::EndPlay(EndPlayReason);
}

void AWizardStaffCauldronHazard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWizardStaffCauldronHazard, HazardType);
	DOREPLIFETIME(AWizardStaffCauldronHazard, HazardRadius);
	DOREPLIFETIME(AWizardStaffCauldronHazard, EruptionOrigin);
}

void AWizardStaffCauldronHazard::InitializeHazard(EWizardCauldronHazardType NewType, float NewRadius, const FVector& NewEruptionOrigin)
{
	if (!HasAuthority())
	{
		return;
	}
	HazardType = NewType;
	HazardRadius = FMath::Max(NewRadius, 25.0f);
	EruptionOrigin = NewEruptionOrigin;
	EruptionTimeRemaining = 0.65f;
	ApplyPresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronHazard::OnRep_HazardPresentation()
{
	ApplyPresentation();
}

void AWizardStaffCauldronHazard::ApplyPresentation()
{
	if (!HazardMesh)
	{
		return;
	}
	const float Scale = HazardRadius / 50.0f;
	HazardMesh->SetRelativeScale3D(FVector(Scale, Scale, 0.035f));
	const FLinearColor Color = HazardType == EWizardCauldronHazardType::Slippery
		? FLinearColor(0.05f, 0.65f, 1.0f)
		: FLinearColor(0.45f, 0.75f, 0.08f);
	if (UMaterialInstanceDynamic* Material = HazardMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
	if (UMaterialInstanceDynamic* Material = EruptionMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
		Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}
