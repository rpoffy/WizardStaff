#include "WizardStaffManaMugPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffGameState.h"
#include "WizardStaffWizardCharacter.h"

AWizardStaffManaMugPickup::AWizardStaffManaMugPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	SetRootComponent(PickupCollision);
	PickupCollision->SetSphereRadius(PickupTuning.PickupRadius);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupCollision->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	if (CylinderMesh.Succeeded())
	{
		MugMeshAsset = CylinderMesh.Object;
	}
	if (SphereMesh.Succeeded())
	{
		ManaMeshAsset = SphereMesh.Object;
	}
	if (BasicMaterial.Succeeded())
	{
		MugMaterial = BasicMaterial.Object;
	}

	MugBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MugBodyMesh"));
	MugBodyMesh->SetupAttachment(PickupCollision);
	MugBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MugBodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 28.0f));
	MugBodyMesh->SetRelativeScale3D(FVector(0.32f, 0.32f, 0.42f));

	ManaTopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ManaTopMesh"));
	ManaTopMesh->SetupAttachment(PickupCollision);
	ManaTopMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ManaTopMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 54.0f));
	ManaTopMesh->SetRelativeScale3D(FVector(0.24f, 0.24f, 0.08f));

	HandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMesh"));
	HandleMesh->SetupAttachment(PickupCollision);
	HandleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HandleMesh->SetRelativeLocation(FVector(28.0f, 0.0f, 34.0f));
	HandleMesh->SetRelativeScale3D(FVector(0.10f, 0.08f, 0.32f));

	if (CylinderMesh.Succeeded())
	{
		MugBodyMesh->SetStaticMesh(CylinderMesh.Object);
	}
	if (SphereMesh.Succeeded())
	{
		ManaTopMesh->SetStaticMesh(SphereMesh.Object);
	}
	if (CubeMesh.Succeeded())
	{
		HandleMesh->SetStaticMesh(CubeMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		MugBodyMesh->SetMaterial(0, BasicMaterial.Object);
		ManaTopMesh->SetMaterial(0, BasicMaterial.Object);
		HandleMesh->SetMaterial(0, BasicMaterial.Object);
	}
}

void AWizardStaffManaMugPickup::BeginPlay()
{
	Super::BeginPlay();

	PickupCollision->SetSphereRadius(PickupTuning.PickupRadius);
	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AWizardStaffManaMugPickup::HandlePickupOverlap);

	ApplyVisualColor(MugBodyMesh, MugColor);
	ApplyVisualColor(HandleMesh, MugColor);
	ApplyVisualColor(ManaTopMesh, ManaColor);
	ApplyPickupActiveState();
}

void AWizardStaffManaMugPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	if (PickupCollision)
	{
		PickupCollision->OnComponentBeginOverlap.RemoveDynamic(this, &AWizardStaffManaMugPickup::HandlePickupOverlap);
		PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PickupCollision->SetGenerateOverlapEvents(false);
	}

	bIsPickupActive = false;
	ApplyPickupActiveState();

	Super::EndPlay(EndPlayReason);
}

void AWizardStaffManaMugPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffManaMugPickup, bIsPickupActive);
}

void AWizardStaffManaMugPickup::Collect(AWizardStaffWizardCharacter* CollectingWizard)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsPickupActive || !CollectingWizard)
	{
		return;
	}

	CollectingWizard->ApplyCollectedMugReward();
	if (UWorld* World = GetWorld())
	{
		if (AWizardStaffGameMode* GameMode = World->GetAuthGameMode<AWizardStaffGameMode>())
		{
			const int32 PlayerIndex = GameMode->GetPlayerIndexForWizard(CollectingWizard);
			GameMode->RecordTelemetryMugCollected(CollectingWizard);
			GameMode->PublishReplicatedGameplayEvent(
				EWizardReplicatedGameplayEventType::MugPickup,
				PlayerIndex == INDEX_NONE ? TEXT("A wizard collected a mug") : FString::Printf(TEXT("P%d collected a mug"), PlayerIndex + 1),
				PlayerIndex,
				INDEX_NONE,
				0.0f,
				true,
				1.8f);
			GameMode->TryGrantMugRunBrewReward(CollectingWizard);
		}
	}
	SetPickupActive(false);

	UE_LOG(LogTemp, Log, TEXT("Mug collected by Wizard %d. Respawn %s."),
		CollectingWizard->GetPlayerState() ? CollectingWizard->GetPlayerState()->GetPlayerId() + 1 : 1,
		PickupTuning.bRespawnAfterPickup ? TEXT("scheduled") : TEXT("disabled"));

	if (PickupTuning.bRespawnAfterPickup)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AWizardStaffManaMugPickup::Respawn, PickupTuning.RespawnDelay, false);
	}
	else
	{
		Destroy();
	}
}

void AWizardStaffManaMugPickup::Respawn()
{
	if (!HasAuthority())
	{
		return;
	}

	SetPickupActive(true);
}

void AWizardStaffManaMugPickup::SetPickupActive(bool bNewActive)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsPickupActive = bNewActive;
	if (!bIsPickupActive)
	{
		GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	}

	ApplyPickupActiveState();
	ForceNetUpdate();
}

void AWizardStaffManaMugPickup::HandlePickupOverlap(
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

	AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(OtherActor);
	if (Wizard)
	{
		Collect(Wizard);
	}
}

void AWizardStaffManaMugPickup::OnRep_PickupActive()
{
	ApplyPickupActiveState();
}

void AWizardStaffManaMugPickup::ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const
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

void AWizardStaffManaMugPickup::ApplyPickupActiveState()
{
	SetActorHiddenInGame(!bIsPickupActive);
	SetActorEnableCollision(bIsPickupActive);

	if (PickupCollision)
	{
		PickupCollision->SetSphereRadius(PickupTuning.PickupRadius);
		PickupCollision->SetCollisionEnabled(bIsPickupActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		PickupCollision->SetGenerateOverlapEvents(bIsPickupActive);
	}

	if (MugBodyMesh)
	{
		MugBodyMesh->SetHiddenInGame(!bIsPickupActive);
		MugBodyMesh->SetVisibility(bIsPickupActive, true);
	}
	if (ManaTopMesh)
	{
		ManaTopMesh->SetHiddenInGame(!bIsPickupActive);
		ManaTopMesh->SetVisibility(bIsPickupActive, true);
	}
	if (HandleMesh)
	{
		HandleMesh->SetHiddenInGame(!bIsPickupActive);
		HandleMesh->SetVisibility(bIsPickupActive, true);
	}
}
