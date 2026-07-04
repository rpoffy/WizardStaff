#include "WizardStaffArcanePinballProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardStaffComponent.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffGameState.h"
#include "WizardStaffHUD.h"

AWizardStaffArcanePinballProjectile::AWizardStaffArcanePinballProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(60.0f);
	SetMinNetUpdateFrequency(30.0f);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(24.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetCanEverAffectNavigation(false);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.22f));

	TrailMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrailMesh"));
	TrailMesh->SetupAttachment(CollisionComponent);
	TrailMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrailMesh->SetRelativeLocation(TrailRelativeLocation);
	TrailMesh->SetRelativeScale3D(TrailScale);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = Tuning.ProjectileSpeed;
	ProjectileMovement->MaxSpeed = Tuning.ProjectileSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.9f;
	ProjectileMovement->Friction = 0.04f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 40.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (SphereMesh.Succeeded())
	{
		ProjectileMeshAsset = SphereMesh.Object;
		VisualMesh->SetStaticMesh(ProjectileMeshAsset);
	}
	if (CubeMesh.Succeeded())
	{
		TrailMesh->SetStaticMesh(CubeMesh.Object);
	}
	if (BasicMaterial.Succeeded())
	{
		ProjectileMaterial = BasicMaterial.Object;
		VisualMesh->SetMaterial(0, ProjectileMaterial);
		TrailMesh->SetMaterial(0, ProjectileMaterial);
	}
}

void AWizardStaffArcanePinballProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	ConstrainToLaunchHeight();
}

void AWizardStaffArcanePinballProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &AWizardStaffArcanePinballProjectile::HandleProjectileHit);
	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &AWizardStaffArcanePinballProjectile::HandleProjectileBounce);
	SetLifeSpan(FMath::Max(Tuning.Lifetime, 0.1f));
	ApplyReadabilityShellState();

	ApplyVisualColor(VisualMesh, ProjectileColor);
	ApplyVisualColor(TrailMesh, TrailColor);
}

void AWizardStaffArcanePinballProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateArcanePinballGameplay();
	Super::EndPlay(EndPlayReason);
}

void AWizardStaffArcanePinballProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardStaffArcanePinballProjectile, bReadabilityOnlyShell);
}

void AWizardStaffArcanePinballProjectile::InitializeArcanePinball(AWizardStaffWizardCharacter* InCaster, const FWizardArcanePinballTuning& InTuning, const FVector& LaunchDirection)
{
	ConfigureArcanePinball(InCaster, InTuning, LaunchDirection, false);
}

void AWizardStaffArcanePinballProjectile::InitializeArcanePinballReadabilityShell(AWizardStaffWizardCharacter* InCaster, const FWizardArcanePinballTuning& InTuning, const FVector& LaunchDirection)
{
	ConfigureArcanePinball(InCaster, InTuning, LaunchDirection, true);
}

void AWizardStaffArcanePinballProjectile::ConfigureArcanePinball(AWizardStaffWizardCharacter* InCaster, const FWizardArcanePinballTuning& InTuning, const FVector& LaunchDirection, bool bInReadabilityOnlyShell)
{
	CasterWizard = InCaster;
	Tuning = InTuning;
	bReadabilityOnlyShell = bInReadabilityOnlyShell;
	bArcanePinballGameplayEnded = false;
	BounceCount = 0;
	RecentWizardHitTimes.Reset();
	LockedHeightZ = GetActorLocation().Z;
	bHasLockedHeight = true;
	SetLifeSpan(FMath::Max(Tuning.Lifetime, 0.1f));
	ApplyReadabilityShellState();

	const FVector SafeDirection = LaunchDirection.GetSafeNormal();
	const FVector FinalDirection = SafeDirection.IsNearlyZero() ? GetActorForwardVector() : SafeDirection;
	const float SafeSpeed = FMath::Max(Tuning.ProjectileSpeed, 0.0f);

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = SafeSpeed;
		ProjectileMovement->MaxSpeed = SafeSpeed;
		ProjectileMovement->Velocity = FinalDirection * SafeSpeed;
		ProjectileMovement->bShouldBounce = !bReadabilityOnlyShell;
		ProjectileMovement->Activate(true);
	}

	ConstrainToLaunchHeight();
	ForceNetUpdate();
}

void AWizardStaffArcanePinballProjectile::HandleProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority() || bReadabilityOnlyShell || bArcanePinballGameplayEnded)
	{
		return;
	}

	AWizardStaffWizardCharacter* HitWizard = Cast<AWizardStaffWizardCharacter>(OtherActor);
	if (!CanHitWizard(HitWizard))
	{
		return;
	}

	ApplyHitToWizard(HitWizard, Hit);
	if (Tuning.bDestroyOnPlayerHit)
	{
		DeactivateArcanePinballGameplay();
		Destroy();
	}
}

void AWizardStaffArcanePinballProjectile::HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (!HasAuthority() || bReadabilityOnlyShell || bArcanePinballGameplayEnded)
	{
		return;
	}

	++BounceCount;
	if (AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr)
	{
		GameMode->RecordTelemetryArcanePinballBounce(CasterWizard.Get());
	}

	float NewSpeed = ProjectileMovement ? ProjectileMovement->Velocity.Size() : 0.0f;
	if (ProjectileMovement)
	{
		ConstrainToLaunchHeight();

		const float SpeedMultiplier = FMath::Max(Tuning.SpeedMultiplierPerWallBounce, 1.0f);
		const float UncappedSpeed = FMath::Max(Tuning.ProjectileSpeed, 0.0f) * FMath::Pow(SpeedMultiplier, static_cast<float>(BounceCount));
		const float SpeedCap = Tuning.MaxProjectileSpeed > 0.0f ? FMath::Max(Tuning.MaxProjectileSpeed, Tuning.ProjectileSpeed) : UncappedSpeed;
		NewSpeed = FMath::Min(UncappedSpeed, SpeedCap);

		FVector BounceDirection = ProjectileMovement->Velocity.GetSafeNormal();
		BounceDirection.Z = 0.0f;
		if (BounceDirection.IsNearlyZero())
		{
			BounceDirection = ImpactVelocity.GetSafeNormal();
			BounceDirection.Z = 0.0f;
		}
		if (BounceDirection.IsNearlyZero())
		{
			BounceDirection = GetActorForwardVector();
		}

		ProjectileMovement->MaxSpeed = FMath::Max(ProjectileMovement->MaxSpeed, NewSpeed);
		ProjectileMovement->Velocity = BounceDirection * NewSpeed;
		ConstrainToLaunchHeight();
	}

	ForceNetUpdate();

	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()) + 2850ULL,
			0.45f,
			FColor::Magenta,
			FString::Printf(TEXT("Arcane Pinball bounce %d/%d | speed %.0f"), BounceCount, FMath::Max(Tuning.MaxBounces, 0), NewSpeed));
	}

	if (UWorld* World = GetWorld())
	{
		DrawDebugSphere(World, ImpactResult.ImpactPoint, 18.0f, 10, FColor::Magenta, false, 0.35f, 0, 2.0f);
	}

	if (BounceCount >= FMath::Max(Tuning.MaxBounces, 0))
	{
		DeactivateArcanePinballGameplay();
		Destroy();
	}
}

void AWizardStaffArcanePinballProjectile::ApplyHitToWizard(AWizardStaffWizardCharacter* HitWizard, const FHitResult& Hit)
{
	if (!HasAuthority() || bReadabilityOnlyShell || bArcanePinballGameplayEnded)
	{
		return;
	}

	if (!HitWizard)
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const TWeakObjectPtr<AWizardStaffWizardCharacter> HitWizardKey(HitWizard);
	RecentWizardHitTimes.FindOrAdd(HitWizardKey) = Now;

	FVector KnockbackDirection = HitWizard->GetActorLocation() - GetActorLocation();
	KnockbackDirection.Z = 0.0f;
	if (!KnockbackDirection.Normalize())
	{
		KnockbackDirection = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
		KnockbackDirection.Z = 0.0f;
		KnockbackDirection.Normalize();
	}

	const int32 CasterStaffSegments = CasterWizard.IsValid() ? CasterWizard->GetStaffSegmentCount() : 0;
	HitWizard->ApplyBonkReaction(KnockbackDirection, Tuning.HitKnockback, Tuning.HitKnockback * 0.08f, CasterStaffSegments);

	if (Tuning.SloshOnHit > 0.0f)
	{
		HitWizard->AddManaSlosh(Tuning.SloshOnHit);
	}
	float HitStressGained = 0.0f;
	if (Tuning.StressOnHit > 0.0f && HitWizard->StaffComponent)
	{
		HitStressGained = Tuning.StressOnHit;
		HitWizard->StaffComponent->AddStaffStress(Tuning.StressOnHit, TEXT("ArcanePinballHit"));
	}

	const int32 HitPlayerIndex = GetWizardPlayerIndex(HitWizard);
	const bool bSelfHit = CasterWizard.IsValid() && HitWizard == CasterWizard.Get();
	if (AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr)
	{
		GameMode->RecordTelemetryArcanePinballHit(CasterWizard.Get(), HitWizard, bSelfHit, HitStressGained);
		const int32 CasterPlayerIndex = GameMode->GetPlayerIndexForWizard(CasterWizard.Get());
		const int32 ServerHitPlayerIndex = GameMode->GetPlayerIndexForWizard(HitWizard);
		const FString CasterLabel = CasterPlayerIndex == INDEX_NONE ? TEXT("Arcane Pinball") : FString::Printf(TEXT("P%d"), CasterPlayerIndex + 1);
		const FString HitLabel = ServerHitPlayerIndex == INDEX_NONE ? TEXT("a wizard") : FString::Printf(TEXT("P%d"), ServerHitPlayerIndex + 1);
		const FString EventMessage = bSelfHit
			? FString::Printf(TEXT("%s self-hit with Arcane Pinball"), *HitLabel)
			: FString::Printf(TEXT("%s hit %s with Arcane Pinball"), *CasterLabel, *HitLabel);
		GameMode->PublishReplicatedGameplayEvent(
			EWizardReplicatedGameplayEventType::ArcanePinballHit,
			EventMessage,
			CasterPlayerIndex,
			ServerHitPlayerIndex,
			HitStressGained,
			false,
			2.2f);
	}

	const FString HitMessage = bSelfHit
		? FString::Printf(TEXT("P%d self-hit with Arcane Pinball! Stress +%.0f"), HitPlayerIndex + 1, Tuning.StressOnHit)
		: FString::Printf(TEXT("Arcane Pinball hit P%d! Stress +%.0f"), HitPlayerIndex + 1, Tuning.StressOnHit);
	AWizardStaffHUD::PushGameplayMessage(this, HitMessage, bSelfHit ? FColor::Orange : FColor::Magenta, 2.2f, EWizardHudMessageCategory::Powerup);
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(
			2860 + FMath::Max(HitPlayerIndex, 0),
			1.35f,
			bSelfHit ? FColor::Orange : FColor::Magenta,
			HitMessage);
	}

	if (UWorld* World = GetWorld())
	{
		const FVector HitDebugLocation = Hit.ImpactPoint.IsNearlyZero() ? HitWizard->GetActorLocation() : FVector(Hit.ImpactPoint);
		DrawDebugSphere(World, HitDebugLocation, 36.0f, 16, FColor::Magenta, false, 0.8f, 0, 3.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("Arcane Pinball hit Wizard %d%s."), HitPlayerIndex, bSelfHit ? TEXT(" (self-hit)") : TEXT(""));
}

bool AWizardStaffArcanePinballProjectile::CanHitWizard(AWizardStaffWizardCharacter* HitWizard) const
{
	if (!HitWizard)
	{
		return false;
	}
	if (!HitWizard->CanReceiveBonkOrSpellHits())
	{
		return false;
	}
	if (!Tuning.bAllowSelfHit && CasterWizard.IsValid() && HitWizard == CasterWizard.Get())
	{
		return false;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> HitWizardKey(HitWizard);
	if (const float* LastHitTime = RecentWizardHitTimes.Find(HitWizardKey))
	{
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		return Now - *LastHitTime >= PlayerHitCooldown;
	}

	return true;
}

int32 AWizardStaffArcanePinballProjectile::GetWizardPlayerIndex(const AWizardStaffWizardCharacter* Wizard) const
{
	const APlayerState* PlayerState = Wizard ? Wizard->GetPlayerState() : nullptr;
	return PlayerState ? FMath::Max(PlayerState->GetPlayerId(), 0) : 0;
}

void AWizardStaffArcanePinballProjectile::ConstrainToLaunchHeight()
{
	if (!Tuning.bLockHeightToLaunchHeight || !bHasLockedHeight)
	{
		return;
	}

	FVector CurrentLocation = GetActorLocation();
	if (!FMath::IsNearlyEqual(CurrentLocation.Z, LockedHeightZ, 0.25f))
	{
		CurrentLocation.Z = LockedHeightZ;
		SetActorLocation(CurrentLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (!ProjectileMovement)
	{
		return;
	}

	FVector Velocity = ProjectileMovement->Velocity;
	if (FMath::IsNearlyZero(Velocity.Z))
	{
		return;
	}

	const float CurrentSpeed = Velocity.Size();
	Velocity.Z = 0.0f;
	FVector HorizontalDirection = Velocity.GetSafeNormal();
	if (HorizontalDirection.IsNearlyZero())
	{
		HorizontalDirection = GetActorForwardVector();
		HorizontalDirection.Z = 0.0f;
		HorizontalDirection.Normalize();
	}

	ProjectileMovement->Velocity = HorizontalDirection * CurrentSpeed;
}

void AWizardStaffArcanePinballProjectile::ApplyVisualColor(UStaticMeshComponent* MeshComponent, const FLinearColor& Color) const
{
	if (!MeshComponent)
	{
		return;
	}

	if (ProjectileMaterial)
	{
		MeshComponent->SetMaterial(0, ProjectileMaterial);
	}

	UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (!DynamicMaterial)
	{
		return;
	}

	DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
}

void AWizardStaffArcanePinballProjectile::ApplyReadabilityShellState()
{
	if (CollisionComponent)
	{
		const bool bEnableAuthorityCollision = HasAuthority() && !bReadabilityOnlyShell && !bArcanePinballGameplayEnded;
		CollisionComponent->SetCollisionEnabled(bEnableAuthorityCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		CollisionComponent->SetNotifyRigidBodyCollision(bEnableAuthorityCollision);
	}

	if (ProjectileMovement)
	{
		if (!HasAuthority())
		{
			ProjectileMovement->bShouldBounce = false;
			ProjectileMovement->StopMovementImmediately();
			ProjectileMovement->Deactivate();
			return;
		}

		ProjectileMovement->bShouldBounce = !bReadabilityOnlyShell;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
	}
}

void AWizardStaffArcanePinballProjectile::DeactivateArcanePinballGameplay()
{
	if (bArcanePinballGameplayEnded)
	{
		return;
	}

	bArcanePinballGameplayEnded = true;
	RecentWizardHitTimes.Reset();

	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionComponent->SetNotifyRigidBodyCollision(false);
		CollisionComponent->OnComponentHit.RemoveDynamic(this, &AWizardStaffArcanePinballProjectile::HandleProjectileHit);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->OnProjectileBounce.RemoveDynamic(this, &AWizardStaffArcanePinballProjectile::HandleProjectileBounce);
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
}

void AWizardStaffArcanePinballProjectile::OnRep_ReadabilityOnlyShell()
{
	ApplyReadabilityShellState();
}
