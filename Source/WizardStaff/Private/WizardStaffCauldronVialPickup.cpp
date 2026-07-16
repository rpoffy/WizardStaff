#include "WizardStaffCauldronVialPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffWizardCharacter.h"

AWizardStaffCauldronVialPickup::AWizardStaffCauldronVialPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	SetRootComponent(PickupCollision);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupCollision->SetGenerateOverlapEvents(true);

	VialBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VialBodyMesh"));
	VialBodyMesh->SetupAttachment(PickupCollision);
	VialBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VialBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 36.0f));
	VialBodyMesh->SetRelativeScale3D(FVector(0.24f, 0.24f, 0.52f));

	VialGlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VialGlowMesh"));
	VialGlowMesh->SetupAttachment(PickupCollision);
	VialGlowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VialGlowMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	VialGlowMesh->SetRelativeScale3D(FVector(0.18f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (CylinderMesh.Succeeded())
	{
		VialBodyMesh->SetStaticMesh(CylinderMesh.Object);
	}
	if (SphereMesh.Succeeded())
	{
		VialGlowMesh->SetStaticMesh(SphereMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		VialBodyMesh->SetMaterial(0, BasicMaterial.Object);
		VialGlowMesh->SetMaterial(0, BasicMaterial.Object);
	}
}

void AWizardStaffCauldronVialPickup::BeginPlay()
{
	Super::BeginPlay();
	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AWizardStaffCauldronVialPickup::HandlePickupOverlap);
	ApplyPresentation();
}

void AWizardStaffCauldronVialPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PickupCollision)
	{
		PickupCollision->OnComponentBeginOverlap.RemoveDynamic(this, &AWizardStaffCauldronVialPickup::HandlePickupOverlap);
		PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PickupCollision->SetGenerateOverlapEvents(false);
	}
	Super::EndPlay(EndPlayReason);
}

void AWizardStaffCauldronVialPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWizardStaffCauldronVialPickup, VialType);
	DOREPLIFETIME(AWizardStaffCauldronVialPickup, bPickupActive);
}

void AWizardStaffCauldronVialPickup::InitializeVial(EWizardCauldronVialType NewVialType, float NewPickupRadius)
{
	if (!HasAuthority())
	{
		return;
	}
	VialType = NewVialType;
	PickupRadius = FMath::Max(NewPickupRadius, 1.0f);
	bPickupActive = true;
	ApplyPresentation();
	ForceNetUpdate();
}

void AWizardStaffCauldronVialPickup::HandlePickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bPickupActive)
	{
		return;
	}

	AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(OtherActor);
	AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	if (!Wizard || !GameMode || !GameMode->HandleCauldronVialCollected(Wizard, VialType))
	{
		return;
	}

	bPickupActive = false;
	ApplyPresentation();
	ForceNetUpdate();
	Destroy();
}

void AWizardStaffCauldronVialPickup::OnRep_VialPresentation()
{
	ApplyPresentation();
}

void AWizardStaffCauldronVialPickup::ApplyPresentation()
{
	const bool bVisible = bPickupActive && VialType != EWizardCauldronVialType::None;
	const FLinearColor Color = GetWizardCauldronVialColor(VialType);
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(bVisible);
	PickupCollision->SetSphereRadius(PickupRadius);
	PickupCollision->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	PickupCollision->SetGenerateOverlapEvents(bVisible);
	for (UStaticMeshComponent* Mesh : { VialBodyMesh.Get(), VialGlowMesh.Get() })
	{
		if (Mesh)
		{
			Mesh->SetHiddenInGame(!bVisible);
			Mesh->SetVisibility(bVisible, true);
			if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
			{
				Material->SetVectorParameterValue(TEXT("Color"), Color);
				Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
			}
		}
	}
}
