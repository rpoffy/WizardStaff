#include "WizardStaffFinalRitualCircle.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AWizardStaffFinalRitualCircle::AWizardStaffFinalRitualCircle()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CircleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CircleMesh"));
	CircleMesh->SetupAttachment(SceneRoot);
	CircleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CircleMesh->SetGenerateOverlapEvents(false);
	CircleMesh->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CylinderMesh.Succeeded())
	{
		CircleMesh->SetStaticMesh(CylinderMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		CircleMaterial = BasicMaterial.Object;
		CircleMesh->SetMaterial(0, CircleMaterial);
	}
}

void AWizardStaffFinalRitualCircle::BeginPlay()
{
	Super::BeginPlay();
	ApplyCircleState();
}

void AWizardStaffFinalRitualCircle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffFinalRitualCircle, ReplicatedCircleCenter);
	DOREPLIFETIME(AWizardStaffFinalRitualCircle, ReplicatedCircleRadius);
	DOREPLIFETIME(AWizardStaffFinalRitualCircle, ReplicatedCircleColor);
	DOREPLIFETIME(AWizardStaffFinalRitualCircle, bReplicatedCircleVisible);
}

void AWizardStaffFinalRitualCircle::SetCircleState(const FVector& Center, float Radius, const FLinearColor& Color, bool bVisible)
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedCircleCenter = Center;
	ReplicatedCircleRadius = FMath::Max(Radius, 50.0f);
	ReplicatedCircleColor = Color;
	bReplicatedCircleVisible = bVisible;
	ApplyCircleState();
	ForceNetUpdate();
}

void AWizardStaffFinalRitualCircle::SetCircleVisible(bool bVisible)
{
	if (!HasAuthority())
	{
		return;
	}

	bReplicatedCircleVisible = bVisible;
	ApplyCircleState();
	ForceNetUpdate();
}

void AWizardStaffFinalRitualCircle::OnRep_CircleState()
{
	ApplyCircleState();
}

void AWizardStaffFinalRitualCircle::ApplyCircleState()
{
	SetActorLocation(ReplicatedCircleCenter + FVector(0.0f, 0.0f, 3.0f));

	const float CircleRadius = FMath::Max(ReplicatedCircleRadius, 50.0f);
	SetActorScale3D(FVector(CircleRadius / 50.0f, CircleRadius / 50.0f, 0.03f));
	SetActorHiddenInGame(!bReplicatedCircleVisible);

	if (CircleMesh)
	{
		CircleMesh->SetVisibility(bReplicatedCircleVisible, true);
		CircleMesh->SetHiddenInGame(!bReplicatedCircleVisible);
		CircleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CircleMesh->SetGenerateOverlapEvents(false);
	}

	ApplyCircleColor();
}

void AWizardStaffFinalRitualCircle::ApplyCircleColor()
{
	if (!CircleMesh)
	{
		return;
	}

	if (!CircleMaterialInstance)
	{
		CircleMaterialInstance = CircleMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (!CircleMaterialInstance)
	{
		return;
	}

	CircleMaterialInstance->SetVectorParameterValue(TEXT("Color"), ReplicatedCircleColor);
	CircleMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), ReplicatedCircleColor);
	CircleMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), ReplicatedCircleColor);
}
