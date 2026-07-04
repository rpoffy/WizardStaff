#include "WizardStaffSharedCamera.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"
#include "WizardStaffComponent.h"
#include "WizardStaffWizardCharacter.h"

AWizardStaffSharedCamera::AWizardStaffSharedCamera()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(20.0f);
	SetMinNetUpdateFrequency(8.0f);

	CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
	SetRootComponent(CameraRoot);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CameraRoot);
	SpringArm->SetRelativeRotation(FRotator(CameraPitchDegrees, 0.0f, 0.0f));
	SpringArm->TargetArmLength = MinArmLength;
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void AWizardStaffSharedCamera::BeginPlay()
{
	Super::BeginPlay();

	ApplyToLocalPlayerControllers();
}

void AWizardStaffSharedCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ApplyToLocalPlayerControllers();

	FVector Center;
	float Radius = 0.0f;
	if (!GetLocalPlayerBounds(Center, Radius))
	{
		return;
	}

	const FVector DesiredLocation = Center + TargetOffset;
	const float SafeRadius = FMath::Max(Radius, MinimumTrackedRadius);
	const float DesiredArmLength = FMath::Clamp((SafeRadius + PlayerBoundsPadding) * RadiusToArmLengthScale, MinArmLength, MaxArmLength);

	SetActorLocation(FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaSeconds, FollowLerpSpeed));
	SpringArm->SetRelativeRotation(FRotator(CameraPitchDegrees, 0.0f, 0.0f));
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DesiredArmLength, DeltaSeconds, ZoomLerpSpeed);
}

void AWizardStaffSharedCamera::ApplyToLocalPlayerControllers()
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!PlayerController || !PlayerController->IsLocalController())
		{
			continue;
		}

		PlayerController->bAutoManageActiveCameraTarget = false;
		if (PlayerController->GetViewTarget() != this)
		{
			PlayerController->SetViewTarget(this);
		}
	}
}

bool AWizardStaffSharedCamera::GetLocalPlayerBounds(FVector& OutCenter, float& OutRadius) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	TArray<FVector> TrackingLocations;
	float ExtraZoomRadius = 0.0f;

	if (World->GetNetMode() == NM_Client)
	{
		for (TActorIterator<AWizardStaffWizardCharacter> It(World); It; ++It)
		{
			AddPawnTrackingPoints(*It, TrackingLocations, ExtraZoomRadius);
		}
	}
	else
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PlayerController = It->Get();
			if (!PlayerController)
			{
				continue;
			}

			const APawn* Pawn = PlayerController->GetPawn();
			if (Pawn)
			{
				AddPawnTrackingPoints(Pawn, TrackingLocations, ExtraZoomRadius);
			}
		}
	}

	if (TrackingLocations.Num() == 0)
	{
		return false;
	}

	FVector MinLocation = TrackingLocations[0];
	FVector MaxLocation = TrackingLocations[0];
	for (const FVector& Location : TrackingLocations)
	{
		MinLocation.X = FMath::Min(MinLocation.X, Location.X);
		MinLocation.Y = FMath::Min(MinLocation.Y, Location.Y);
		MinLocation.Z = FMath::Min(MinLocation.Z, Location.Z);
		MaxLocation.X = FMath::Max(MaxLocation.X, Location.X);
		MaxLocation.Y = FMath::Max(MaxLocation.Y, Location.Y);
		MaxLocation.Z = FMath::Max(MaxLocation.Z, Location.Z);
	}
	OutCenter = (MinLocation + MaxLocation) * 0.5f;

	OutRadius = 0.0f;
	for (const FVector& Location : TrackingLocations)
	{
		OutRadius = FMath::Max(OutRadius, FVector::Dist2D(Location, OutCenter));
	}
	OutRadius += ExtraZoomRadius;

	return true;
}

void AWizardStaffSharedCamera::AddPawnTrackingPoints(const APawn* Pawn, TArray<FVector>& OutLocations, float& InOutExtraZoom) const
{
	if (!Pawn)
	{
		return;
	}

	const FVector PawnLocation = Pawn->GetActorLocation();
	OutLocations.Add(PawnLocation);

	if (!bIncludeStaffHeightInZoom)
	{
		return;
	}

	const AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(Pawn);
	if (!Wizard || !Wizard->StaffComponent)
	{
		return;
	}

	InOutExtraZoom = FMath::Max(InOutExtraZoom, Wizard->StaffComponent->GetStaffCollisionLength() * StaffHeightToArmLengthScale);
}
