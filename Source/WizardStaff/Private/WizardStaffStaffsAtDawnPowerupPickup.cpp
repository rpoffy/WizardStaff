#include "WizardStaffStaffsAtDawnPowerupPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffWizardCharacter.h"

AWizardStaffStaffsAtDawnPowerupPickup::AWizardStaffStaffsAtDawnPowerupPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	SetRootComponent(PickupCollision);
	PickupCollision->SetSphereRadius(PickupRadius);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupCollision->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	if (CylinderMesh.Succeeded())
	{
		BaseMeshAsset = CylinderMesh.Object;
	}
	if (SphereMesh.Succeeded())
	{
		GlowMeshAsset = SphereMesh.Object;
	}
	if (BasicMaterial.Succeeded())
	{
		PowerupMaterial = BasicMaterial.Object;
	}

	PowerupBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerupBaseMesh"));
	PowerupBaseMesh->SetupAttachment(PickupCollision);
	PowerupBaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PowerupBaseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));
	PowerupBaseMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.16f));

	PowerupGlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerupGlowMesh"));
	PowerupGlowMesh->SetupAttachment(PickupCollision);
	PowerupGlowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PowerupGlowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 58.0f));
	PowerupGlowMesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.28f));

	if (CylinderMesh.Succeeded())
	{
		PowerupBaseMesh->SetStaticMesh(CylinderMesh.Object);
	}
	if (SphereMesh.Succeeded())
	{
		PowerupGlowMesh->SetStaticMesh(SphereMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		PowerupBaseMesh->SetMaterial(0, BasicMaterial.Object);
		PowerupGlowMesh->SetMaterial(0, BasicMaterial.Object);
	}
}

void AWizardStaffStaffsAtDawnPowerupPickup::BeginPlay()
{
	Super::BeginPlay();

	PickupCollision->SetSphereRadius(PickupRadius);
	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AWizardStaffStaffsAtDawnPowerupPickup::HandlePickupOverlap);
	ApplyPowerupVisuals();
	ApplyPickupActiveState();
	SetPickupActive(true);
}

void AWizardStaffStaffsAtDawnPowerupPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PickupCollision)
	{
		PickupCollision->OnComponentBeginOverlap.RemoveDynamic(this, &AWizardStaffStaffsAtDawnPowerupPickup::HandlePickupOverlap);
		PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PickupCollision->SetGenerateOverlapEvents(false);
	}

	bIsPickupActive = false;
	ApplyPickupActiveState();

	Super::EndPlay(EndPlayReason);
}

void AWizardStaffStaffsAtDawnPowerupPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffStaffsAtDawnPowerupPickup, PowerupType);
	DOREPLIFETIME(AWizardStaffStaffsAtDawnPowerupPickup, bIsPickupActive);
}

void AWizardStaffStaffsAtDawnPowerupPickup::SetPickupActive(bool bNewActive)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsPickupActive = bNewActive;
	ApplyPickupActiveState();
	ForceNetUpdate();
}

void AWizardStaffStaffsAtDawnPowerupPickup::SetPowerupType(EWizardStaffsAtDawnPowerupType NewPowerupType)
{
	if (!HasAuthority())
	{
		return;
	}

	PowerupType = NewPowerupType;
	ApplyPowerupVisuals();
	ForceNetUpdate();
}

void AWizardStaffStaffsAtDawnPowerupPickup::HandlePickupOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsPickupActive)
	{
		return;
	}

	AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(OtherActor);
	if (!Wizard)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			GameMode->HandleStaffsAtDawnPowerupPickedUp(this, Wizard);
		}
	}
}

void AWizardStaffStaffsAtDawnPowerupPickup::OnRep_PickupActive()
{
	ApplyPickupActiveState();
}

void AWizardStaffStaffsAtDawnPowerupPickup::OnRep_PowerupType()
{
	ApplyPowerupVisuals();
}

void AWizardStaffStaffsAtDawnPowerupPickup::ApplyPowerupVisuals() const
{
	const FLinearColor PowerupColor = GetPowerupColor();
	ApplyVisualColor(PowerupBaseMesh, PowerupColor * 0.45f);
	ApplyVisualColor(PowerupGlowMesh, PowerupColor);
}

void AWizardStaffStaffsAtDawnPowerupPickup::ApplyPickupActiveState()
{
	SetActorHiddenInGame(!bIsPickupActive);
	SetActorEnableCollision(bIsPickupActive);

	if (PickupCollision)
	{
		PickupCollision->SetSphereRadius(PickupRadius);
		PickupCollision->SetCollisionEnabled(bIsPickupActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		PickupCollision->SetGenerateOverlapEvents(bIsPickupActive);
	}

	if (PowerupBaseMesh)
	{
		PowerupBaseMesh->SetHiddenInGame(!bIsPickupActive);
		PowerupBaseMesh->SetVisibility(bIsPickupActive, true);
	}
	if (PowerupGlowMesh)
	{
		PowerupGlowMesh->SetHiddenInGame(!bIsPickupActive);
		PowerupGlowMesh->SetVisibility(bIsPickupActive, true);
	}
}

void AWizardStaffStaffsAtDawnPowerupPickup::ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const
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

FLinearColor AWizardStaffStaffsAtDawnPowerupPickup::GetPowerupColor() const
{
	switch (PowerupType)
	{
	case EWizardStaffsAtDawnPowerupType::MegaStaffBrew:
		return MegaStaffBrewColor;
	case EWizardStaffsAtDawnPowerupType::None:
	default:
		return EmptyPowerupColor;
	}
}
