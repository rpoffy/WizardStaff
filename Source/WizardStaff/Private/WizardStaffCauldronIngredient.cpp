#include "WizardStaffCauldronIngredient.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AWizardStaffCauldronIngredient::AWizardStaffCauldronIngredient()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(30.0f);

	IngredientMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IngredientMesh"));
	SetRootComponent(IngredientMesh);
	IngredientMesh->SetIsReplicated(true);
	IngredientMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	IngredientMesh->SetCollisionObjectType(ECC_WorldDynamic);
	IngredientMesh->SetCollisionResponseToAllChannels(ECR_Block);
	IngredientMesh->SetGenerateOverlapEvents(true);
	IngredientMesh->SetSimulatePhysics(true);
	IngredientMesh->SetLinearDamping(0.35f);
	IngredientMesh->SetAngularDamping(0.45f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (SphereMesh.Succeeded())
	{
		IngredientMesh->SetStaticMesh(SphereMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		IngredientMesh->SetMaterial(0, BasicMaterial.Object);
	}
	// Deliberately oversized for the current PIE bonk-contact diagnostic.
	IngredientMesh->SetRelativeScale3D(FVector(0.682f));
}

void AWizardStaffCauldronIngredient::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority() && IngredientMesh)
	{
		IngredientMesh->SetMassOverrideInKg(NAME_None, 16.0f, true);
	}
	else if (IngredientMesh)
	{
		IngredientMesh->SetGenerateOverlapEvents(false);
		IngredientMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IngredientMesh->SetSimulatePhysics(false);
	}
	ApplyPresentation();
}

void AWizardStaffCauldronIngredient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IngredientMesh)
	{
		IngredientMesh->SetGenerateOverlapEvents(false);
		IngredientMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IngredientMesh->SetSimulatePhysics(false);
	}
	Super::EndPlay(EndPlayReason);
}

void AWizardStaffCauldronIngredient::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWizardStaffCauldronIngredient, VariantIndex);
	DOREPLIFETIME(AWizardStaffCauldronIngredient, LastAttributionPlayerIndex);
	DOREPLIFETIME(AWizardStaffCauldronIngredient, bScored);
}

void AWizardStaffCauldronIngredient::InitializeIngredient(int32 NewVariantIndex)
{
	if (!HasAuthority())
	{
		return;
	}
	VariantIndex = FMath::Max(NewVariantIndex, 0);
	LastAttributionPlayerIndex = INDEX_NONE;
	bScored = false;
	ApplyPresentation();
	ForceNetUpdate();
}

bool AWizardStaffCauldronIngredient::ApplyAuthoritativeBonk(int32 PlayerIndex, const FVector& Direction, float ImpulseStrength)
{
	if (!HasAuthority() || bScored || PlayerIndex == INDEX_NONE || !IngredientMesh)
	{
		return false;
	}

	FVector SafeDirection = Direction;
	SafeDirection.Z = FMath::Max(SafeDirection.Z, 0.12f);
	if (!SafeDirection.Normalize())
	{
		SafeDirection = FVector::ForwardVector;
	}

	LastAttributionPlayerIndex = PlayerIndex;
	// AddImpulse can be swallowed by a sleeping body after replicated contact. Apply the
	// same authority-owned velocity change directly so a confirmed Quick Bonk is readable.
	IngredientMesh->WakeAllRigidBodies();
	const float SafeImpulseStrength = FMath::Max(ImpulseStrength, 0.0f);
	FVector VelocityKick = SafeDirection * SafeImpulseStrength;
	VelocityKick.Z = FMath::Max(VelocityKick.Z, SafeImpulseStrength * 0.18f);
	IngredientMesh->SetPhysicsLinearVelocity(IngredientMesh->GetPhysicsLinearVelocity() + VelocityKick, false);
	ForceNetUpdate();
	return true;
}

bool AWizardStaffCauldronIngredient::MarkScored()
{
	if (!HasAuthority() || bScored)
	{
		return false;
	}
	bScored = true;
	if (IngredientMesh)
	{
		IngredientMesh->SetGenerateOverlapEvents(false);
		IngredientMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IngredientMesh->SetSimulatePhysics(false);
	}
	SetActorHiddenInGame(true);
	ForceNetUpdate();
	return true;
}

void AWizardStaffCauldronIngredient::OnRep_IngredientPresentation()
{
	ApplyPresentation();
}

void AWizardStaffCauldronIngredient::ApplyPresentation()
{
	SetActorHiddenInGame(bScored);
	if (!IngredientMesh)
	{
		return;
	}

	static const FLinearColor Colors[] = {
		FLinearColor(0.95f, 0.18f, 0.12f),
		FLinearColor(0.95f, 0.75f, 0.08f),
		FLinearColor(0.35f, 0.85f, 0.22f),
		FLinearColor(0.60f, 0.22f, 0.95f)
	};
	if (UMaterialInstanceDynamic* Material = IngredientMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Colors[VariantIndex % UE_ARRAY_COUNT(Colors)]);
	}
}
