#include "WizardStaffGameMode.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameInstance.h"
#include "Engine/CollisionProfile.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/ScopeExit.h"
#include "UObject/UObjectGlobals.h"
#include "WizardStaffArcanePinballProjectile.h"
#include "WizardStaffCauldronArena.h"
#include "WizardStaffCauldronDepositArc.h"
#include "WizardStaffCauldronCurseBomb.h"
#include "WizardStaffCauldronHazard.h"
#include "WizardStaffCauldronIngredient.h"
#include "WizardStaffCauldronVialPickup.h"
#include "WizardStaffComponent.h"
#include "WizardStaffFinalRitualCircle.h"
#include "WizardStaffGameState.h"
#include "WizardStaffManaMugPickup.h"
#include "WizardStaffPartyHall.h"
#include "WizardStaffHUD.h"
#include "WizardStaffPlayerState.h"
#include "WizardStaffPrototypeArena.h"
#include "WizardStaffPrototypeLighting.h"
#include "WizardStaffPlaytestBotComponent.h"
#include "WizardStaffSharedCamera.h"
#include "WizardStaffStaffsAtDawnArena.h"
#include "WizardStaffStaffsAtDawnPowerupPickup.h"
#include "WizardStaffWizardCharacter.h"

namespace
{
FString PrototypeSessionModeToText(EWizardPrototypeSessionMode SessionMode)
{
	switch (SessionMode)
	{
	case EWizardPrototypeSessionMode::LocalPrototype:
		return TEXT("Local Prototype");
	case EWizardPrototypeSessionMode::LocalWithBots:
		return TEXT("Local With Bots");
	case EWizardPrototypeSessionMode::OnlineListenServer:
		return TEXT("Online Listen Server");
	case EWizardPrototypeSessionMode::OnlineClient:
		return TEXT("Online Client");
	default:
		return TEXT("Unknown");
	}
}

bool HasCommandLineToken(const TCHAR* Token)
{
#if !UE_BUILD_SHIPPING
	const FString CommandLine = FCommandLine::Get();
	return Token && CommandLine.Contains(Token, ESearchCase::IgnoreCase, ESearchDir::FromStart);
#else
	return false;
#endif
}

bool IsSteamSmokeHostCommandLineRequested()
{
	return HasCommandLineToken(TEXT("WizardSteamHost")) || HasCommandLineToken(TEXT("DebugSteamHostSession"));
}

bool IsSteamSmokeJoinCommandLineRequested()
{
	return HasCommandLineToken(TEXT("WizardSteamJoinFirstSession")) || HasCommandLineToken(TEXT("DebugSteamFindAndJoinSession"));
}

bool IsDirectConnectJoinCommandLineRequested()
{
#if !UE_BUILD_SHIPPING
	const FString CommandLine = FCommandLine::Get();
	if (!CommandLine.Contains(TEXT("ExecCmds"), ESearchCase::IgnoreCase, ESearchDir::FromStart))
	{
		return false;
	}

	return CommandLine.Contains(TEXT("open 127.0.0.1"), ESearchCase::IgnoreCase, ESearchDir::FromStart)
		|| CommandLine.Contains(TEXT("open localhost"), ESearchCase::IgnoreCase, ESearchDir::FromStart);
#else
	return false;
#endif
}

FWizardPrototypeTuningPresetValues MakeStableWizardPreset()
{
	FWizardPrototypeTuningPresetValues Values;
	Values.CollisionLengthPerSegment = 34.0f;
	Values.CollisionThickness = 14.0f;
	Values.ObstructedControlMultiplier = 0.78f;
	Values.ObstructionRecoverySpeed = 12.0f;
	Values.StuckTimeBeforeStressBoost = 1.10f;
	Values.StuckStressPerSecond = 16.0f;
	Values.StuckTimeBeforeCollisionRelief = 1.55f;
	Values.CollisionReliefDuration = 0.85f;
	Values.CollisionReliefControlMultiplier = 0.95f;
	Values.GentleNudgeDistance = 24.0f;
	Values.MaxStaffStress = 130.0f;
	Values.StressGainedPerBonk = 8.0f;
	Values.StressGainedPerWallImpact = 16.0f;
	Values.StressMultiplierPerSegment = 0.075f;
	Values.StressDecayRate = 2.25f;
	Values.CaughtStressPerSecond = 4.0f;
	Values.SnapImpulseForce = 350.0f;
	Values.SnapUpwardImpulse = 100.0f;
	Values.SnappedSegmentLinearDamping = 1.6f;
	Values.SnappedSegmentAngularDamping = 0.9f;
	Values.bSnapImpulseIgnoresSegmentMass = false;
	Values.OutOfArenaRespawnDelay = 1.2f;
	Values.HorizontalOutOfBoundsPadding = 320.0f;
	Values.FallZThreshold = -450.0f;
	Values.SloshGainedPerStaffSegment = 27.0f;
	Values.SloshReducedOnStaffSnap = 4.0f;
	Values.SloshDecayPerSecond = 4.0f;
	Values.TurnPenaltyPerSlosh = 0.18f;
	Values.MovementPenaltyPerSlosh = 0.08f;
	Values.AccelerationPenaltyPerSlosh = 0.12f;
	Values.BrakingPenaltyPerSlosh = 0.18f;
	Values.MinMovementMultiplier = 0.78f;
	Values.MinTurnMultiplier = 0.78f;
	Values.MinAccelerationMultiplier = 0.72f;
	Values.MinBrakingMultiplier = 0.68f;
	Values.SloshOversteerDegreesPerSecond = 20.0f;
	Values.StumbleChancePerSecondAtMaxSlosh = 0.03f;
	Values.SloshVisualLeanDegrees = 3.0f;
	Values.SloshStaffWobbleDegrees = 2.0f;
	Values.bSetSloshOnApply = true;
	Values.SloshAlphaOnApply = 0.10f;
	Values.SegmentCountBeforeHeftPenalty = 6;
	Values.MovementPenaltyPerHeavySegment = 0.020f;
	Values.TurnPenaltyPerHeavySegment = 0.035f;
	Values.MaxMovementPenalty = 0.20f;
	Values.MaxTurnPenalty = 0.30f;
	Values.BonkCooldownPerHeavySegment = 0.025f;
	Values.BonkVisualDurationPerHeavySegment = 0.012f;
	Values.MaxBonkCooldownBonus = 0.25f;
	Values.MaxBonkVisualDurationBonus = 0.16f;
	Values.BonkKnockbackStrength = 940.0f;
	Values.BonkKnockbackPerStaffSegment = 34.0f;
	Values.BonkMaxKnockbackStrength = 1400.0f;
	Values.BonkUpwardBoost = 145.0f;
	Values.BonkCooldown = 0.60f;
	Values.BonkVisualDuration = 0.30f;
	Values.BonkStrikeEaseExponent = 1.0f;
	Values.StaffContactPadding = 10.0f;
	Values.BonkHitStressMultiplier = 1.05f;
	Values.BonkWhiffStressMultiplier = 0.30f;
	Values.MatchDuration = 60.0f;
	Values.MugSpawnCount = 10;
	Values.MugRespawnDelay = 8.0f;
	Values.BrewRewardChance = 0.35f;
	Values.ArcanePinballProjectileSpeed = 1050.0f;
	Values.ArcanePinballSpeedMultiplierPerWallBounce = 1.10f;
	Values.ArcanePinballMaxProjectileSpeed = 2600.0f;
	Values.ArcanePinballMaxBounces = 5;
	Values.ArcanePinballLifetime = 4.25f;
	Values.ArcanePinballHitKnockback = 360.0f;
	Values.ArcanePinballSloshOnHit = 4.0f;
	Values.ArcanePinballStressOnCast = 10.0f;
	Values.ArcanePinballStressOnHit = 4.0f;
	Values.bArcanePinballAllowSelfHit = true;
	Values.bArcanePinballDestroyOnPlayerHit = false;
	Values.LooseSegmentLifetime = 25.0f;
	Values.MaxLooseSegments = 10;
	Values.LooseSegmentFadeOutDuration = 0.8f;
	return Values;
}

FWizardPrototypeTuningPresetValues MakeChaoticWizardPreset()
{
	return FWizardPrototypeTuningPresetValues();
}

FWizardPrototypeTuningPresetValues MakeAbsurdWizardPreset()
{
	FWizardPrototypeTuningPresetValues Values;
	Values.CollisionLengthPerSegment = 44.0f;
	Values.CollisionThickness = 22.0f;
	Values.ObstructedControlMultiplier = 0.35f;
	Values.ObstructionRecoverySpeed = 5.0f;
	Values.StuckTimeBeforeStressBoost = 0.45f;
	Values.StuckStressPerSecond = 42.0f;
	Values.StuckTimeBeforeCollisionRelief = 3.0f;
	Values.CollisionReliefDuration = 0.45f;
	Values.CollisionReliefControlMultiplier = 0.75f;
	Values.GentleNudgeDistance = 32.0f;
	Values.MaxStaffStress = 75.0f;
	Values.StressGainedPerBonk = 14.0f;
	Values.StressGainedPerWallImpact = 24.0f;
	Values.StressMultiplierPerSegment = 0.14f;
	Values.StressDecayRate = 0.5f;
	Values.CaughtStressPerSecond = 8.0f;
	Values.SnapImpulseForce = 1100.0f;
	Values.SnapUpwardImpulse = 320.0f;
	Values.SnappedSegmentLinearDamping = 0.95f;
	Values.SnappedSegmentAngularDamping = 0.75f;
	Values.bSnapImpulseIgnoresSegmentMass = false;
	Values.OutOfArenaRespawnDelay = 1.0f;
	Values.HorizontalOutOfBoundsPadding = 420.0f;
	Values.FallZThreshold = -600.0f;
	Values.SloshGainedPerStaffSegment = 38.0f;
	Values.SloshReducedOnStaffSnap = 19.0f;
	Values.SloshDecayPerSecond = 1.0f;
	Values.TurnPenaltyPerSlosh = 0.55f;
	Values.MovementPenaltyPerSlosh = 0.30f;
	Values.AccelerationPenaltyPerSlosh = 0.42f;
	Values.BrakingPenaltyPerSlosh = 0.55f;
	Values.MinMovementMultiplier = 0.45f;
	Values.MinTurnMultiplier = 0.35f;
	Values.MinAccelerationMultiplier = 0.35f;
	Values.MinBrakingMultiplier = 0.28f;
	Values.SloshOversteerDegreesPerSecond = 78.0f;
	Values.StumbleChancePerSecondAtMaxSlosh = 0.22f;
	Values.SloshVisualLeanDegrees = 11.0f;
	Values.SloshStaffWobbleDegrees = 9.0f;
	Values.bSetSloshOnApply = true;
	Values.SloshAlphaOnApply = 0.85f;
	Values.SegmentCountBeforeHeftPenalty = 3;
	Values.MovementPenaltyPerHeavySegment = 0.050f;
	Values.TurnPenaltyPerHeavySegment = 0.075f;
	Values.MaxMovementPenalty = 0.40f;
	Values.MaxTurnPenalty = 0.55f;
	Values.BonkCooldownPerHeavySegment = 0.055f;
	Values.BonkVisualDurationPerHeavySegment = 0.025f;
	Values.MaxBonkCooldownBonus = 0.50f;
	Values.MaxBonkVisualDurationBonus = 0.35f;
	Values.BonkKnockbackStrength = 1250.0f;
	Values.BonkKnockbackPerStaffSegment = 55.0f;
	Values.BonkMaxKnockbackStrength = 1900.0f;
	Values.BonkUpwardBoost = 240.0f;
	Values.BonkCooldown = 0.50f;
	Values.BonkVisualDuration = 0.26f;
	Values.BonkStrikeEaseExponent = 0.85f;
	Values.StaffContactPadding = 16.0f;
	Values.BonkHitStressMultiplier = 1.20f;
	Values.BonkWhiffStressMultiplier = 0.60f;
	Values.MatchDuration = 60.0f;
	Values.MugSpawnCount = 12;
	Values.MugRespawnDelay = 4.0f;
	Values.BrewRewardChance = 0.65f;
	Values.ArcanePinballProjectileSpeed = 1750.0f;
	Values.ArcanePinballSpeedMultiplierPerWallBounce = 1.35f;
	Values.ArcanePinballMaxProjectileSpeed = 6500.0f;
	Values.ArcanePinballMaxBounces = 12;
	Values.ArcanePinballLifetime = 6.2f;
	Values.ArcanePinballHitKnockback = 700.0f;
	Values.ArcanePinballSloshOnHit = 16.0f;
	Values.ArcanePinballStressOnCast = 42.0f;
	Values.ArcanePinballStressOnHit = 18.0f;
	Values.bArcanePinballAllowSelfHit = true;
	Values.bArcanePinballDestroyOnPlayerHit = false;
	Values.LooseSegmentLifetime = 45.0f;
	Values.MaxLooseSegments = 28;
	Values.LooseSegmentFadeOutDuration = 0.5f;
	return Values;
}

FString GetStaffsAtDawnPowerupTypeText(EWizardStaffsAtDawnPowerupType PowerupType)
{
	switch (PowerupType)
	{
	case EWizardStaffsAtDawnPowerupType::MegaStaffBrew:
		return TEXT("Mega Staff Brew");
	case EWizardStaffsAtDawnPowerupType::None:
	default:
		return TEXT("None");
	}
}

FColor GetReplicatedGameplayEventColor(EWizardReplicatedGameplayEventType EventType)
{
	switch (EventType)
	{
	case EWizardReplicatedGameplayEventType::BrewRewardGranted:
	case EWizardReplicatedGameplayEventType::BrewRewardUsed:
	case EWizardReplicatedGameplayEventType::ArcanePinballShell:
	case EWizardReplicatedGameplayEventType::ArcanePinballCast:
	case EWizardReplicatedGameplayEventType::ArcanePinballHit:
	case EWizardReplicatedGameplayEventType::StaffsPowerupCollected:
	case EWizardReplicatedGameplayEventType::MegaStaffGranted:
	case EWizardReplicatedGameplayEventType::MegaStaffExpired:
		return FColor::Magenta;
	case EWizardReplicatedGameplayEventType::StaffSegmentSnapped:
	case EWizardReplicatedGameplayEventType::RingOutPending:
		return FColor::Red;
	case EWizardReplicatedGameplayEventType::RespawnComplete:
	case EWizardReplicatedGameplayEventType::StaffClashStarted:
		return FColor::Cyan;
	case EWizardReplicatedGameplayEventType::StaffClashResolved:
	case EWizardReplicatedGameplayEventType::CauldronIngredientDeposited:
	case EWizardReplicatedGameplayEventType::GrandWizardCandidateChanged:
	case EWizardReplicatedGameplayEventType::FinalWinner:
		return FColor::Yellow;
	case EWizardReplicatedGameplayEventType::CauldronCurse:
		return FColor::Red;
	case EWizardReplicatedGameplayEventType::RematchStarted:
	case EWizardReplicatedGameplayEventType::MugPickup:
	default:
		return FColor::Green;
	}
}

EWizardHudMessageCategory GetReplicatedGameplayEventHudCategory(EWizardReplicatedGameplayEventType EventType)
{
	switch (EventType)
	{
	case EWizardReplicatedGameplayEventType::BrewRewardGranted:
	case EWizardReplicatedGameplayEventType::BrewRewardUsed:
	case EWizardReplicatedGameplayEventType::ArcanePinballShell:
	case EWizardReplicatedGameplayEventType::ArcanePinballCast:
	case EWizardReplicatedGameplayEventType::ArcanePinballHit:
	case EWizardReplicatedGameplayEventType::StaffsPowerupCollected:
	case EWizardReplicatedGameplayEventType::MegaStaffGranted:
	case EWizardReplicatedGameplayEventType::MegaStaffExpired:
		return EWizardHudMessageCategory::Powerup;
	case EWizardReplicatedGameplayEventType::StaffSegmentSnapped:
	case EWizardReplicatedGameplayEventType::RingOutPending:
	case EWizardReplicatedGameplayEventType::RespawnComplete:
	case EWizardReplicatedGameplayEventType::StaffClashStarted:
	case EWizardReplicatedGameplayEventType::StaffClashResolved:
	case EWizardReplicatedGameplayEventType::CauldronCurse:
		return EWizardHudMessageCategory::Gameplay;
	case EWizardReplicatedGameplayEventType::CauldronIngredientDeposited:
	case EWizardReplicatedGameplayEventType::GrandWizardCandidateChanged:
	case EWizardReplicatedGameplayEventType::FinalWinner:
		return EWizardHudMessageCategory::Scoring;
	case EWizardReplicatedGameplayEventType::RematchStarted:
	case EWizardReplicatedGameplayEventType::MugPickup:
	default:
		return EWizardHudMessageCategory::Gameplay;
	}
}
}

AWizardStaffGameMode::AWizardStaffGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = AWizardStaffWizardCharacter::StaticClass();
	HUDClass = AWizardStaffHUD::StaticClass();
	GameStateClass = AWizardStaffGameState::StaticClass();
	PlayerStateClass = AWizardStaffPlayerState::StaticClass();
	SharedCameraClass = AWizardStaffSharedCamera::StaticClass();
	RuntimePrototypeArenaClass = AWizardStaffPrototypeArena::StaticClass();
	RuntimeStaffsAtDawnArenaClass = AWizardStaffStaffsAtDawnArena::StaticClass();
	StaffsAtDawnPowerupPickupClass = AWizardStaffStaffsAtDawnPowerupPickup::StaticClass();
	RuntimePartyHallClass = AWizardStaffPartyHall::StaticClass();
	MugRunTuning.ManaMugPickupClass = AWizardStaffManaMugPickup::StaticClass();
	StableTuningPreset = MakeStableWizardPreset();
	ChaoticTuningPreset = MakeChaoticWizardPreset();
	AbsurdTuningPreset = MakeAbsurdWizardPreset();
}

void AWizardStaffGameMode::BeginPlay()
{
	Super::BeginPlay();

	ApplyPlaytestBotPIEDefaults();
	RefreshPrototypeSessionMode(TEXT("BeginPlay"));
	SpawnPrototypeLighting();
	SpawnPrototypeArena();
	SpawnStaffsAtDawnArena();
	SpawnPartyHall();
	SpawnSharedCamera();
	EnsureLocalPlayers();
	AssignSharedCameraToAllPlayers();

	if (bApplyTuningPresetOnBeginPlay)
	{
		SetPrototypeTuningPreset(ActivePrototypeTuningPreset);
	}
	SyncPlaytestBots();

	if (MugRunTuning.bEnableMugRun)
	{
		StartPartyMatch();
	}
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::SpawnPrototypeLighting()
{
	UWorld* World = GetWorld();
	if (!World || ActivePrototypeLighting)
	{
		return;
	}

	for (TActorIterator<AWizardStaffPrototypeLighting> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			ActivePrototypeLighting = *It;
			return;
		}
	}

	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			UE_LOG(LogTemp, Log, TEXT("WizardStaff using authored map lighting from %s."), *GetNameSafe(*It));
			return;
		}
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = MakeUniqueObjectName(World, AWizardStaffPrototypeLighting::StaticClass(), TEXT("RuntimePrototypeLighting"));
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActivePrototypeLighting = World->SpawnActor<AWizardStaffPrototypeLighting>(
		AWizardStaffPrototypeLighting::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);

	if (ActivePrototypeLighting)
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff spawned runtime prototype sun/sky lighting because the startup map has no authored directional light."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff could not spawn runtime prototype lighting; the world may render dark."));
	}
}

void AWizardStaffGameMode::ApplyPlaytestBotPIEDefaults()
{
#if WITH_EDITOR
	const UWorld* World = GetWorld();
	if (!bAutoEnableSoloPlaytestBotInPIE || !World || World->WorldType != EWorldType::PIE || !IsStandaloneLocalPrototypeSession())
	{
		return;
	}

	bEnablePlaytestBots = true;
	bFillMissingPlayersWithBots = true;
	DesiredHumanPlayers = 1;
	DesiredTotalPlayers = FMath::Max(DesiredTotalPlayers, 2);

	UE_LOG(LogTemp, Log, TEXT("WizardStaff PIE playtest bot defaults active: DesiredHumanPlayers=%d DesiredTotalPlayers=%d."),
		DesiredHumanPlayers,
		DesiredTotalPlayers);
#endif
}

EWizardPrototypeSessionMode AWizardStaffGameMode::DetectPrototypeSessionMode() const
{
	const UWorld* World = GetWorld();
	if (World)
	{
		switch (World->GetNetMode())
		{
		case NM_Client:
			return EWizardPrototypeSessionMode::OnlineClient;
		case NM_ListenServer:
		case NM_DedicatedServer:
			return EWizardPrototypeSessionMode::OnlineListenServer;
		case NM_Standalone:
		default:
			break;
		}
	}

	if (IsSteamSmokeHostCommandLineRequested())
	{
		return EWizardPrototypeSessionMode::OnlineListenServer;
	}
	if (IsSteamSmokeJoinCommandLineRequested())
	{
		return EWizardPrototypeSessionMode::OnlineClient;
	}
	if (IsDirectConnectJoinCommandLineRequested())
	{
		return EWizardPrototypeSessionMode::OnlineClient;
	}

	const int32 HumanPlayers = FMath::Clamp(DesiredHumanPlayers, 1, 4);
	const int32 TotalPlayers = FMath::Clamp(DesiredTotalPlayers, 2, 4);
	const bool bLocalBotsRequested = bEnablePlaytestBots && bFillMissingPlayersWithBots && HumanPlayers < TotalPlayers;
	return bLocalBotsRequested
		? EWizardPrototypeSessionMode::LocalWithBots
		: EWizardPrototypeSessionMode::LocalPrototype;
}

void AWizardStaffGameMode::RefreshPrototypeSessionMode(const TCHAR* Reason)
{
	const EWizardPrototypeSessionMode PreviousMode = PrototypeSessionMode;
	PrototypeSessionMode = DetectPrototypeSessionMode();

	if (AWizardStaffGameState* WizardGameState = GetWizardStaffGameState())
	{
		WizardGameState->SetPrototypeSessionModeMirror(PrototypeSessionMode);
	}

#if !UE_BUILD_SHIPPING
	if (!bPrototypeSessionModeLogged || PreviousMode != PrototypeSessionMode)
	{
		const UWorld* World = GetWorld();
		UE_LOG(LogTemp, Log, TEXT("WizardStaff prototype session mode: %s (%s, net mode %d)."),
			*GetPrototypeSessionModeText(),
			Reason ? Reason : TEXT("Refresh"),
			World ? static_cast<int32>(World->GetNetMode()) : INDEX_NONE);
		bPrototypeSessionModeLogged = true;
	}
#endif
}

bool AWizardStaffGameMode::IsStandaloneLocalPrototypeSession() const
{
	const EWizardPrototypeSessionMode DetectedMode = DetectPrototypeSessionMode();
	return DetectedMode == EWizardPrototypeSessionMode::LocalPrototype || DetectedMode == EWizardPrototypeSessionMode::LocalWithBots;
}

bool AWizardStaffGameMode::ShouldHoldOnlineIntermissionForPlayers() const
{
	const EWizardPrototypeSessionMode DetectedMode = DetectPrototypeSessionMode();
	return (DetectedMode == EWizardPrototypeSessionMode::OnlineListenServer || DetectedMode == EWizardPrototypeSessionMode::OnlineClient)
		&& GetConnectedPlayerControllerCount() < 2;
}

FString AWizardStaffGameMode::GetPrototypeSessionModeText() const
{
	return PrototypeSessionModeToText(PrototypeSessionMode);
}

int32 AWizardStaffGameMode::GetConnectedPlayerControllerCount() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	int32 ConnectedControllerCount = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get())
		{
			++ConnectedControllerCount;
		}
	}

	return ConnectedControllerCount;
}

void AWizardStaffGameMode::AssignOnlineScaffoldPlayerSlot(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	AWizardStaffPlayerState* WizardPlayerState = Cast<AWizardStaffPlayerState>(Controller->PlayerState);
	if (!WizardPlayerState)
	{
		return;
	}

	const int32 ExistingPlayerSlot = WizardPlayerState->GetWizardDisplaySlot();
	const bool bAssigningNewSlot = ExistingPlayerSlot < 0;
	const int32 PlayerIndex = bAssigningNewSlot
		? FindFirstAvailablePlayerSlot(WizardPlayerState)
		: ExistingPlayerSlot;
	const AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(Controller->GetPawn());
	const int32 StaffSegmentScore = Wizard ? Wizard->GetStaffSegmentCount() : 0;
	const int32 StaffsAtDawnScore = GetStaffsAtDawnScore(PlayerIndex);
	const int32 CurrentTrialScore = ActiveTrialType == EWizardTrialType::StaffsAtDawn
		? StaffsAtDawnScore
		: (ActiveTrialType == EWizardTrialType::CauldronCatastrophe ? GetCauldronScore(PlayerIndex) : StaffSegmentScore);
	const bool bBot = IsPlayerIndexPlaytestBot(PlayerIndex);
	const FString SummaryText = FString::Printf(
		TEXT("P%d Staff %d Favor %d Wins %d%s"),
		PlayerIndex + 1,
		StaffSegmentScore,
		GetPlayerGrandWizardFavor(PlayerIndex),
		GetPlayerRoundWins(PlayerIndex),
		bBot ? TEXT(" BOT") : TEXT(""));

	const bool bMirrorChanged = WizardPlayerState->SetWizardPlayerMirror(
		PlayerIndex,
		PlayerIndex,
		GetPlayerRoundWins(PlayerIndex),
		GetPlayerGrandWizardFavor(PlayerIndex),
		CurrentTrialScore,
		StaffsAtDawnScore,
		IsPartyHallPlayerReady(PlayerIndex),
		bBot,
		SummaryText);
	if (bMirrorChanged)
	{
		WizardPlayerState->ForceNetUpdate();
	}

	if (bAssigningNewSlot && !IsStandaloneLocalPrototypeSession())
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff online scaffold assigned %s to display slot P%d."),
			*GetNameSafe(Controller),
			PlayerIndex + 1);
	}
}

int32 AWizardStaffGameMode::FindFirstAvailablePlayerSlot(const AWizardStaffPlayerState* PlayerStateToIgnore) const
{
	TSet<int32> UsedSlots;
	if (const AWizardStaffGameState* WizardGameState = GetWizardStaffGameState())
	{
		for (const APlayerState* BasePlayerState : WizardGameState->PlayerArray)
		{
			const AWizardStaffPlayerState* WizardPlayerState = Cast<AWizardStaffPlayerState>(BasePlayerState);
			if (!WizardPlayerState || WizardPlayerState == PlayerStateToIgnore)
			{
				continue;
			}

			const int32 AssignedSlot = WizardPlayerState->GetWizardDisplaySlot();
			if (AssignedSlot >= 0)
			{
				UsedSlots.Add(AssignedSlot);
			}
		}
	}

	int32 CandidateSlot = 0;
	while (UsedSlots.Contains(CandidateSlot))
	{
		++CandidateSlot;
	}
	return CandidateSlot;
}

void AWizardStaffGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ON_SCOPE_EXIT
	{
		SyncReplicatedObservableState();
	};

	UpdateOutOfArenaRespawns(DeltaSeconds);
	UpdateLooseSnappedSegments(DeltaSeconds);
	UpdateKeyboardFallbackControls();

	if ((PartyMatchState == EWizardPartyMatchState::PartyHall || PartyMatchState == EWizardPartyMatchState::Intermission) && ActiveTrialState == EWizardTrialState::WaitingToStart)
	{
		if (ShouldHoldOnlineIntermissionForPlayers())
		{
			IntermissionRemainingTime = FMath::Max(IntermissionRemainingTime, PartyMatchTuning.IntermissionDuration);
			UpdatePartyHallSigns();
			DrawMugRunDebug();
			UpdateLeaderHighlights();

#if !UE_BUILD_SHIPPING
			if (!bLoggedWaitingForOnlinePlayers)
			{
				UE_LOG(LogTemp, Log, TEXT("WizardStaff online smoke startup holding Party Hall until a second player connects."));
				bLoggedWaitingForOnlinePlayers = true;
			}
#endif
			return;
		}

		bLoggedWaitingForOnlinePlayers = false;
		IntermissionRemainingTime = FMath::Max(0.0f, IntermissionRemainingTime - DeltaSeconds);
		UpdatePartyHallSigns();
		DrawMugRunDebug();
		UpdateLeaderHighlights();

		if (IntermissionRemainingTime <= 0.0f)
		{
			StartNextTrial();
		}
		return;
	}

	if (ActiveTrialState == EWizardTrialState::Countdown)
	{
		TrialCountdownRemainingTime = FMath::Max(0.0f, TrialCountdownRemainingTime - DeltaSeconds);
		DrawMugRunDebug();
		DrawStaffsAtDawnArenaDebug();
		UpdateLeaderHighlights();

		if (TrialCountdownRemainingTime <= 0.0f)
		{
			StartActiveTrial();
		}
		return;
	}

	if (PartyMatchState == EWizardPartyMatchState::FinalRound)
	{
		if (ActiveTrialState == EWizardTrialState::Active)
		{
			UpdateGrandWizardFinalRound(DeltaSeconds);
		}
		else if (ActiveTrialState == EWizardTrialState::Results)
		{
			TrialResultsRemainingTime = FMath::Max(0.0f, TrialResultsRemainingTime - DeltaSeconds);
			if (TrialResultsRemainingTime <= 0.0f)
			{
				StartPartyMatch();
				return;
			}
		}
		DrawGrandWizardFinalWorldMarkers();
		DrawMugRunDebug();
		UpdateLeaderHighlights();
		return;
	}

	if (ActiveTrialState == EWizardTrialState::Results)
	{
		TrialResultsRemainingTime = FMath::Max(0.0f, TrialResultsRemainingTime - DeltaSeconds);
		MugRunPostMatchRemainingTime = TrialResultsRemainingTime;
		DrawMugRunDebug();
		DrawStaffsAtDawnArenaDebug();
		UpdateLeaderHighlights();

		if (TrialResultsRemainingTime <= 0.0f)
		{
			FinishActiveTrialResults();
		}
		return;
	}

	if (ActiveTrialType == EWizardTrialType::StaffsAtDawn && ActiveTrialState == EWizardTrialState::Active)
	{
		StaffsAtDawnRemainingTime = FMath::Max(0.0f, StaffsAtDawnRemainingTime - DeltaSeconds);
		UpdateStaffsAtDawnPowerups(DeltaSeconds);
		DrawMugRunDebug();
		DrawStaffsAtDawnArenaDebug();
		UpdateLeaderHighlights();

		if (StaffsAtDawnRemainingTime <= 0.0f)
		{
			EndStaffsAtDawnTrial();
		}
		return;
	}

	if (ActiveTrialType == EWizardTrialType::CauldronCatastrophe && ActiveTrialState == EWizardTrialState::Active)
	{
		UpdateCauldronCatastrophe(DeltaSeconds);
		DrawMugRunDebug();
		UpdateLeaderHighlights();
		return;
	}

	if (MugRunMatchState != EWizardMugRunMatchState::Playing)
	{
		DrawMugRunDebug();
		return;
	}

	MugRunRemainingTime = FMath::Max(0.0f, MugRunRemainingTime - DeltaSeconds);
	DrawMugRunDebug();
	UpdateLeaderHighlights();

	if (MugRunRemainingTime <= 0.0f)
	{
		EndMugRunMatch();
	}
}

void AWizardStaffGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	RefreshPrototypeSessionMode(TEXT("PostLogin"));
	AssignOnlineScaffoldPlayerSlot(NewPlayer);
	AssignSharedCameraToPlayer(NewPlayer);
	SyncPlaytestBots();
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer || NewPlayer->GetPawn())
	{
		return;
	}

	const FTransform SpawnTransform = GetSpawnTransformForController(NewPlayer);
	APawn* NewPawn = SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaffGameMode could not spawn a wizard pawn."));
		return;
	}

	NewPlayer->Possess(NewPawn);
	FinishRestartPlayer(NewPlayer, SpawnTransform.Rotator());

	if (bHasAppliedPrototypeTuningPreset)
	{
		ApplyPrototypeTuningPresetToWizard(Cast<AWizardStaffWizardCharacter>(NewPawn), GetPrototypeTuningPresetValues(ActivePrototypeTuningPreset));
	}
	AssignSharedCameraToPlayer(Cast<APlayerController>(NewPlayer));
	AssignOnlineScaffoldPlayerSlot(NewPlayer);
	SyncPlaytestBots();
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::StartPartyMatch()
{
	if (!MugRunTuning.bEnableMugRun)
	{
		return;
	}

	CleanupCauldronCatastrophe();
	CauldronScores.Reset();
	LastCauldronCursedPlayerIndex = INDEX_NONE;
	BumpMatchSessionGeneration();
	CompletedTrialCount = 0;
	EnsurePlayerRoundWinsSize(GetDesiredLocalPlayerCountForSession());
	for (int32& RoundWins : PlayerRoundWins)
	{
		RoundWins = 0;
	}
	EnsurePlayerGrandWizardFavorSize(GetDesiredLocalPlayerCountForSession());
	for (int32& Favor : PlayerGrandWizardFavor)
	{
		Favor = 0;
	}
	GrandWizardFavorFeedbackMessage.Reset();
	GrandWizardFavorFeedbackExpireTime = 0.0f;
	ResetPlaytestTelemetry();
	CleanupArcanePinballProjectiles();
	PartyMatchState = EWizardPartyMatchState::PartyHall;
	ActiveTrialType = EWizardTrialType::MugRun;
	ActiveTrialState = EWizardTrialState::WaitingToStart;
	bWizardsStagedForActiveTrial = false;
	SetWizardPrototypeInputsLocked(false);
	TrialCountdownRemainingTime = 0.0f;
	TrialResultsRemainingTime = 0.0f;
	IntermissionRemainingTime = 0.0f;
	MugRunWinnerMessage.Reset();
	ResetGrandWizardFinalRoundState();
	ResetStaffsAtDawnForNewTrial();
	ClearPendingOutOfArenaRespawns();
	if (LooseSegmentChaosTuning.bCleanupLooseSegmentsOnRematch)
	{
		CleanupLooseSnappedSegments();
	}
	ResetWizardGameplayStateForMatchSetup();
	ResetMugRunStateForNewTrial();
	SetMugRunPickupsActive(false);
	MugRunWinnerMessage.Reset();
	PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType::RematchStarted,
		TEXT("New match started"),
		INDEX_NONE,
		INDEX_NONE,
		0.0f,
		true,
		2.0f);

	EnterPartyHallIntermission();
}

void AWizardStaffGameMode::StartNextTrial()
{
	if (!MugRunTuning.bEnableMugRun)
	{
		return;
	}

	if (CompletedTrialCount >= FMath::Max(PartyMatchTuning.TrialsBeforeFinalRound, 1))
	{
		EnterGrandWizardFinalRound();
		return;
	}

	StartTrialCountdown(GetTrialTypeForTrialIndex(CompletedTrialCount));
}

void AWizardStaffGameMode::StartTrialCountdown(EWizardTrialType TrialType)
{
	CleanupCauldronCatastrophe();
	CauldronScores.Reset();
	ActiveTrialType = TrialType;
	PartyMatchState = EWizardPartyMatchState::Trial;
	ActiveTrialState = EWizardTrialState::Countdown;
	TrialCountdownRemainingTime = FMath::Max(PartyMatchTuning.TrialCountdownDuration, 0.0f);
	TrialResultsRemainingTime = 0.0f;
	IntermissionRemainingTime = 0.0f;
	PartyHallReadyFeedbackMessage.Reset();
	PartyHallReadyFeedbackExpireTime = 0.0f;
	bPartyHallAllReadyTriggered = false;
	ClearReplicatedGameplayEventFeed();
	ClearPendingOutOfArenaRespawns();

	if (ActiveTrialType == EWizardTrialType::MugRun)
	{
		SetPrototypeArenaPhasePresentationActive(true);
		if (LooseSegmentChaosTuning.bCleanupLooseSegmentsOnRematch)
		{
			CleanupLooseSnappedSegments();
		}
		ResetWizardGameplayStateForMatchSetup();
		ResetMugRunStateForNewTrial();
		SetMugRunPickupsActive(false);
	}
	else if (ActiveTrialType == EWizardTrialType::StaffsAtDawn)
	{
		ResetStaffsAtDawnForNewTrial();
		SetMugRunPickupsActive(false);
	}
	else if (ActiveTrialType == EWizardTrialType::CauldronCatastrophe)
	{
		SetMugRunPickupsActive(false);
		ResetStaffsAtDawnPowerups();
	}

	if (PartyMatchTuning.bResetStaffsAtTrialStart)
	{
		ResetWizardStaffsForTrialStart();
	}
	if (TrialType == EWizardTrialType::CauldronCatastrophe)
	{
		// The Cauldron floor is spawned at activation, so keep players in Party Hall until then.
		bWizardsStagedForActiveTrial = false;
	}
	else
	{
		StageWizardsForCurrentPhase();
		bWizardsStagedForActiveTrial = true;
	}
	SetWizardPrototypeInputsLocked(true);

	AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("Trial Countdown: %s"), *GetActiveTrialName()), FColor::Cyan, 1.8f, EWizardHudMessageCategory::Gameplay);
	if (GEngine && PartyMatchTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4600, 1.5f, FColor::Cyan, FString::Printf(TEXT("Trial Countdown: %s"), *GetActiveTrialName()));
	}

	if (TrialCountdownRemainingTime <= 0.0f)
	{
		StartActiveTrial();
	}
}

void AWizardStaffGameMode::StartActiveTrial()
{
	PartyMatchState = EWizardPartyMatchState::Trial;
	ActiveTrialState = EWizardTrialState::Active;

	switch (ActiveTrialType)
	{
	case EWizardTrialType::MugRun:
		StartMugRunMatch();
		break;
	case EWizardTrialType::StaffsAtDawn:
		StartStaffsAtDawnTrial();
		break;
	case EWizardTrialType::CauldronCatastrophe:
		StartCauldronCatastropheTrial();
		break;
	default:
		StartMugRunMatch();
		break;
	}
}

void AWizardStaffGameMode::StartMugRunMatch()
{
	if (!MugRunTuning.ManaMugPickupClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mug Run cannot start without a ManaMugPickupClass."));
		bWizardsStagedForActiveTrial = false;
		SetWizardPrototypeInputsLocked(false);
		return;
	}

	PartyMatchState = EWizardPartyMatchState::Trial;
	ActiveTrialType = EWizardTrialType::MugRun;
	ActiveTrialState = EWizardTrialState::Active;
	// Countdown staging already revealed the Mug Run arena; keep it active for gameplay.
	SetPrototypeArenaPhasePresentationActive(true);
	if (!bWizardsStagedForActiveTrial)
	{
		StageWizardsForCurrentPhase();
	}
	bWizardsStagedForActiveTrial = false;
	SetWizardPrototypeInputsLocked(false);

	ResetStaffsAtDawnPowerups();
	SpawnMugRunPickups();
	SetMugRunPickupsActive(true);

	MugRunWinnerMessage.Reset();
	MugRunRemainingTime = MugRunTuning.MatchDuration;
	MugRunPostMatchRemainingTime = 0.0f;
	TrialCountdownRemainingTime = 0.0f;
	TrialResultsRemainingTime = 0.0f;
	IntermissionRemainingTime = 0.0f;
	bMugRunMatchActive = true;
	MugRunMatchState = EWizardMugRunMatchState::Playing;
	UpdateLeaderHighlights();

	UE_LOG(LogTemp, Log, TEXT("Mug Run started for %.0f seconds with %d mug pickups."), MugRunTuning.MatchDuration, SpawnedMugs.Num());
}

void AWizardStaffGameMode::EndMugRunMatch()
{
	if (!bMugRunMatchActive)
	{
		return;
	}

	bMugRunMatchActive = false;
	MugRunRemainingTime = 0.0f;
	TrialResultsRemainingTime = FMath::Max(PartyMatchTuning.TrialResultsDisplayDuration, 0.0f);
	MugRunPostMatchRemainingTime = TrialResultsRemainingTime;
	MugRunMatchState = EWizardMugRunMatchState::PostMatch;
	PartyMatchState = EWizardPartyMatchState::Results;
	ActiveTrialState = EWizardTrialState::Results;
	SetMugRunPickupsActive(false);
	CleanupArcanePinballProjectiles();
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			Wizard->ClearCarriedBrewReward();
		}
	}
	AnnounceMugRunWinner();
	UpdateLeaderHighlights();
}

void AWizardStaffGameMode::StartStaffsAtDawnTrial()
{
	PartyMatchState = EWizardPartyMatchState::Trial;
	ActiveTrialType = EWizardTrialType::StaffsAtDawn;
	ActiveTrialState = EWizardTrialState::Active;
	if (!bWizardsStagedForActiveTrial)
	{
		StageWizardsForCurrentPhase();
	}
	bWizardsStagedForActiveTrial = false;
	SetWizardPrototypeInputsLocked(false);

	SetMugRunPickupsActive(false);
	CleanupArcanePinballProjectiles();
	ClearPendingOutOfArenaRespawns();
	RecentBonkAttackerPlayerIndexes.Reset();
	RecentBonkTimes.Reset();
	RecentBonkWasStaffClash.Reset();
	InitializeStaffsAtDawnPowerups();

	MugRunWinnerMessage.Reset();
	StaffsAtDawnRemainingTime = FMath::Max(StaffsAtDawnTuning.TrialDuration, 0.0f);
	TrialCountdownRemainingTime = 0.0f;
	TrialResultsRemainingTime = 0.0f;
	MugRunPostMatchRemainingTime = 0.0f;
	bMugRunMatchActive = false;
	MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;
	bStaffsAtDawnTrialActive = true;
	UpdateLeaderHighlights();

	UE_LOG(LogTemp, Log, TEXT("Staffs at Dawn started for %.0f seconds."), StaffsAtDawnRemainingTime);
	AWizardStaffHUD::PushGameplayMessage(this, TEXT("Staffs at Dawn! Bonk for points."), FColor::Orange, 2.2f, EWizardHudMessageCategory::Gameplay);
	if (GEngine && StaffsAtDawnTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4700, 2.0f, FColor::Orange, TEXT("Staffs at Dawn! Bonk for points."));
	}
	if (GEngine && StaffsAtDawnTuning.bShowArenaDebug)
	{
		const float HalfSize = GetCurrentPlayBoundsHalfSize();
		const float RingOutHalfSize = HalfSize + FMath::Max(OutOfArenaRespawnTuning.HorizontalOutOfBoundsPadding, 0.0f);
		const FString ArenaDebugText = FString::Printf(
			TEXT("Staffs at Dawn arena: %s | Spawns %d | Half %.0f | Ring-out %.0f | FallZ %.0f | Future markers %d"),
			*GetStaffsAtDawnArenaSourceText(),
			GetStaffsAtDawnArenaPlayerSpawnCount(),
			HalfSize,
			RingOutHalfSize,
			GetCurrentOutOfArenaFallZThreshold(),
			GetStaffsAtDawnArenaFuturePowerupSpawnCount());
		GEngine->AddOnScreenDebugMessage(4701, 5.0f, FColor::Cyan, ArenaDebugText);

		const FString PowerupDebugText = FString::Printf(
			TEXT("Staffs at Dawn powerups: %s | Type %s | Active spawn slots %d"),
			StaffsAtDawnTuning.bEnableStaffsAtDawnPowerups ? TEXT("enabled") : TEXT("disabled"),
			*GetStaffsAtDawnPowerupTypeText(StaffsAtDawnTuning.DefaultPowerupType),
			StaffsAtDawnPowerupSpawnLocations.Num());
		GEngine->AddOnScreenDebugMessage(4702, 5.0f, FColor::Purple, PowerupDebugText);
	}

	if (StaffsAtDawnRemainingTime <= 0.0f)
	{
		EndStaffsAtDawnTrial();
	}
}

void AWizardStaffGameMode::EndStaffsAtDawnTrial()
{
	if (!bStaffsAtDawnTrialActive)
	{
		return;
	}

	bStaffsAtDawnTrialActive = false;
	StaffsAtDawnRemainingTime = 0.0f;
	if (bEnablePlaytestTelemetry)
	{
		++StaffsAtDawnTelemetryRoundsCompleted;
	}
	TrialResultsRemainingTime = FMath::Max(PartyMatchTuning.TrialResultsDisplayDuration, 0.0f);
	MugRunPostMatchRemainingTime = TrialResultsRemainingTime;
	PartyMatchState = EWizardPartyMatchState::Results;
	ActiveTrialState = EWizardTrialState::Results;
	SetMugRunPickupsActive(false);
	ResetStaffsAtDawnPowerups();
	RecentBonkAttackerPlayerIndexes.Reset();
	RecentBonkTimes.Reset();
	RecentBonkWasStaffClash.Reset();
	RecentStaffsAtDawnBroomBoostTimes.Reset();
	StaffsAtDawnBroomBoostRingOutThreats.Reset();
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard && Wizard->IsMegaStaffBrewActive())
		{
			Wizard->ClearMegaStaffBrew(true);
		}
	}
	AnnounceStaffsAtDawnWinner();
	UpdateLeaderHighlights();
}

void AWizardStaffGameMode::StartCauldronCatastropheTrial()
{
	if (!HasAuthority())
	{
		return;
	}

	CleanupCauldronCatastrophe();
	PartyMatchState = EWizardPartyMatchState::Trial;
	ActiveTrialType = EWizardTrialType::CauldronCatastrophe;
	ActiveTrialState = EWizardTrialState::Active;
	bWizardsStagedForActiveTrial = false;
	SetMugRunPickupsActive(false);
	CleanupArcanePinballProjectiles();
	ResetStaffsAtDawnPowerups();
	ClearPendingOutOfArenaRespawns();
	SetPrototypeArenaPhasePresentationActive(false);
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard && Wizard->IsMegaStaffBrewActive())
		{
			Wizard->ClearMegaStaffBrew(true);
		}
	}

	EnsureCauldronScoreSize(GetDesiredLocalPlayerCountForSession());
	for (int32& Score : CauldronScores)
	{
		Score = 0;
	}
	MugRunWinnerMessage.Reset();
	CauldronRemainingTime = FMath::Max(CauldronCatastropheTuning.TrialDuration, 0.0f);

	CauldronNextCurseTime = FMath::Max(CauldronCatastropheTuning.FirstCurseDelay, 0.0f);
	bCauldronCatastropheActive = true;
	ResetCauldronSpillState();

	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = MakeUniqueObjectName(World, AWizardStaffCauldronArena::StaticClass(), TEXT("CauldronCatastropheArena"));
		ActiveCauldronArena = World->SpawnActor<AWizardStaffCauldronArena>(AWizardStaffCauldronArena::StaticClass(), RuntimeCauldronArenaLocation, FRotator::ZeroRotator, SpawnParameters);
	}

	// Keep countdown staging in Party Hall, then move wizards only after the Cauldron floor exists.
	StageWizardsForCurrentPhase();
	SetWizardPrototypeInputsLocked(false);
	SetCauldronActiveIntake(ChooseNextCauldronIntake(EWizardCauldronIntakeDirection::None));

	for (int32 VialIndex = 0; VialIndex < FMath::Max(CauldronCatastropheTuning.InitialVialCount, 1); ++VialIndex)
	{
		SpawnCauldronVial();
	}

	AWizardStaffHUD::PushGameplayMessage(this, TEXT("Cauldron Catastrophe! Collect vials to grow your staff."), FColor::Yellow, 3.0f, EWizardHudMessageCategory::Gameplay);
	UE_LOG(LogTemp, Log, TEXT("Cauldron Catastrophe started for %.0f seconds with %d vials."), CauldronRemainingTime, SpawnedCauldronVials.Num());
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::EndCauldronCatastropheTrial()
{
	if (!HasAuthority() || !bCauldronCatastropheActive)
	{
		return;
	}

	bCauldronCatastropheActive = false;
	CauldronRemainingTime = 0.0f;
	TrialResultsRemainingTime = FMath::Max(PartyMatchTuning.TrialResultsDisplayDuration, 0.0f);
	MugRunPostMatchRemainingTime = TrialResultsRemainingTime;
	PartyMatchState = EWizardPartyMatchState::Results;
	ActiveTrialState = EWizardTrialState::Results;
	CleanupCauldronGameplayState();
	AnnounceCauldronWinner();
	// Retain only the Cauldron arena through Results/intermission so its floor remains under
	// transitioning players. Gameplay actors, effects, and callbacks are already neutralized.
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::DebugStartCauldronCatastrophe()
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugStartCauldronCatastrophe rejected on a non-authority instance."));
		return;
	}

	bMugRunMatchActive = false;
	bStaffsAtDawnTrialActive = false;
	StartTrialCountdown(EWizardTrialType::CauldronCatastrophe);
	TrialCountdownRemainingTime = 0.0f;
	StartActiveTrial();
#endif
}

void AWizardStaffGameMode::DebugSpawnCauldronIngredient()
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Warning, TEXT("DebugSpawnCauldronIngredient is legacy; use DebugSpawnCauldronVial Speed or BurdeningPower."));
#endif
}

void AWizardStaffGameMode::DebugSpawnCauldronVial(const FString& VialType)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		const EWizardCauldronVialType ParsedType = ParseCauldronVialType(VialType);
		if (ParsedType != EWizardCauldronVialType::None)
		{
			SpawnCauldronVial(ParsedType);
		}
	}
#endif
}

void AWizardStaffGameMode::DebugGiveCauldronVial(int32 PlayerIndex, const FString& VialType)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		if (AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex))
		{
			GrantCauldronVial(Wizard, ParseCauldronVialType(VialType));
		}
	}
#endif
}

void AWizardStaffGameMode::DebugBreakTopCauldronVialSegment(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		if (AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex))
		{
			if (Wizard->StaffComponent)
			{
				Wizard->StaffComponent->SnapTopStaffSegment();
			}
		}
	}
#endif
}

void AWizardStaffGameMode::DebugPrintCauldronVialStack(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	const AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	const TArray<FCauldronVialStackEntry>* Stack = Wizard ? CauldronVialStacks.Find(Wizard) : nullptr;
	FString StackText;
	if (Stack)
	{
		for (const FCauldronVialStackEntry& Entry : *Stack)
		{
			StackText += (StackText.IsEmpty() ? TEXT("") : TEXT(" -> ")) + GetWizardCauldronVialDisplayName(Entry.Type);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Cauldron vial stack P%d: %s"), PlayerIndex + 1, StackText.IsEmpty() ? TEXT("(empty)") : *StackText);
#endif
}

void AWizardStaffGameMode::DebugPrintCauldronInstability(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	const AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	const TArray<FCauldronVialStackEntry>* Stack = Wizard ? CauldronVialStacks.Find(Wizard) : nullptr;
	const int32 AuthoritativeCount = Stack ? Stack->Num() : 0;
	const int32 SafeCount = FMath::Max(CauldronCatastropheTuning.VialInstabilitySafeCount, 0);
	const int32 ExtraCount = FMath::Max(AuthoritativeCount - SafeCount, 0);
	FString ValidationReport;
	const bool bVialStateValid = ValidateCauldronVialState(Wizard, ValidationReport);
	UE_LOG(LogTemp, Log, TEXT("Cauldron instability P%d: authoritative %d | readable %d | safe %d | extra %d | per-extra %.2f | multiplier %.2fx | max %.2fx | active %s | enabled %s | stress %.1f | vial %s | stack/tag %s (%s)"),
		PlayerIndex + 1, AuthoritativeCount, Wizard ? Wizard->GetReadableCauldronVialCount() : 0, SafeCount, ExtraCount,
		CauldronCatastropheTuning.VialInstabilityStressPerExtraVial, GetCauldronVialInstabilityMultiplier(Wizard),
		CauldronCatastropheTuning.VialInstabilityMaximumMultiplier, bCauldronCatastropheActive ? TEXT("Cauldron") : TEXT("Inactive"),
		CauldronCatastropheTuning.bEnableVialInstability ? TEXT("true") : TEXT("false"), Wizard ? Wizard->GetReadableStaffStress() : 0.0f,
		Wizard ? *GetWizardCauldronVialDisplayName(Wizard->GetReadableActiveCauldronVial()) : TEXT("None"),
		bVialStateValid ? TEXT("valid") : TEXT("MISMATCH"), *ValidationReport);
#endif
}

void AWizardStaffGameMode::DebugApplyCauldronInstabilityTestHit(int32 AttackerIndex, int32 TargetIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority() || !bCauldronCatastropheActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cauldron instability test hit requires the active authoritative Cauldron trial."));
		return;
	}

	AWizardStaffWizardCharacter* Attacker = GetWizardForPlayerIndex(AttackerIndex);
	AWizardStaffWizardCharacter* Target = GetWizardForPlayerIndex(TargetIndex);
	if (!IsValid(Attacker) || !IsValid(Target) || Attacker == Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cauldron instability test hit requires two different valid wizards."));
		return;
	}

	const float Multiplier = GetCauldronVialInstabilityMultiplier(Target);
	float BaseStress = 0.0f;
	float FinalStress = 0.0f;
	if (Multiplier <= 1.0f || !Target->ApplyCauldronVialInstabilityStress(Multiplier, BaseStress, FinalStress))
	{
		UE_LOG(LogTemp, Log, TEXT("Cauldron instability test P%d -> P%d did not apply: multiplier %.2fx."), AttackerIndex + 1, TargetIndex + 1, Multiplier);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Cauldron instability test P%d -> P%d: base %.2f x %.2f = final %.2f."), AttackerIndex + 1, TargetIndex + 1, BaseStress, Multiplier, FinalStress);
#endif
}

void AWizardStaffGameMode::DebugValidateCauldronVialState(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	FString Report;
	const AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	const bool bValid = ValidateCauldronVialState(Wizard, Report);
	if (bValid)
	{
		UE_LOG(LogTemp, Log, TEXT("Cauldron vial validation P%d: %s"), PlayerIndex + 1, *Report);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cauldron vial validation P%d: %s"), PlayerIndex + 1, *Report);
	}
#endif
}

void AWizardStaffGameMode::DebugClearCauldronVials()
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority())
	{
		ClearCauldronVials(true);
	ResetCauldronSpillState();
	}
#endif
}

void AWizardStaffGameMode::DebugDepositCauldronVials(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	DebugStartCauldronBanking(PlayerIndex);
#endif
}

void AWizardStaffGameMode::DebugSpillCauldronVials(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		SpillCauldronVials(GetWizardForPlayerIndex(PlayerIndex), true);
	}
#endif
}

void AWizardStaffGameMode::DebugPrintCauldronSpillState(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	const AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(const_cast<AWizardStaffWizardCharacter*>(Wizard));
	const TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(WizardKey);
	FString SpillTypes;
	if (Stack)
	{
		const int32 SpillCount = FMath::Min(Stack->Num(), FMath::Max(CauldronCatastropheTuning.RingOutVialsSpilled, 0));
		for (int32 SpillIndex = 0; SpillIndex < SpillCount; ++SpillIndex)
		{
			SpillTypes += (SpillTypes.IsEmpty() ? TEXT("") : TEXT(", ")) + GetWizardCauldronVialDisplayName((*Stack)[Stack->Num() - 1 - SpillIndex].Type);
		}
	}
	const FVector LastSafePosition = CauldronLastSafePositions.Contains(WizardKey) ? CauldronLastSafePositions[WizardKey] : FVector::ZeroVector;
	UE_LOG(LogTemp, Log, TEXT("Cauldron spill P%d: stack %d | configured %d | last safe %s | allowed %s | processed %s | would spill [%s] | tracked pickups %d"),
		PlayerIndex + 1,
		Stack ? Stack->Num() : 0,
		CauldronCatastropheTuning.RingOutVialsSpilled,
		*LastSafePosition.ToCompactString(),
		IsValid(Wizard) && bCauldronCatastropheActive ? TEXT("true") : TEXT("false"),
		CauldronProcessedRingOutSpills.Contains(WizardKey) ? TEXT("true") : TEXT("false"),
		*SpillTypes,
		CauldronSpilledVialPickups.Num());
#endif
}

void AWizardStaffGameMode::DebugSetCauldronLastSafePosition(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority() || !bCauldronCatastropheActive)
	{
		return;
	}

	if (AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex))
	{
		if (IsCauldronSafeSpillPosition(Wizard->GetActorLocation()))
		{
			CauldronLastSafePositions.Add(Wizard, Wizard->GetActorLocation());
			UE_LOG(LogTemp, Log, TEXT("Cauldron spill P%d recorded safe position %s."), PlayerIndex + 1, *Wizard->GetActorLocation().ToCompactString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cauldron spill P%d rejected invalid safe position %s."), PlayerIndex + 1, *Wizard->GetActorLocation().ToCompactString());
		}
	}
#endif
}
void AWizardStaffGameMode::DebugAssignCauldronCurse(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		AssignCauldronCurse(PlayerIndex);
	}
#endif
}

void AWizardStaffGameMode::DebugClearCauldronHazards()
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		return;
	}
	for (AWizardStaffCauldronHazard* Hazard : SpawnedCauldronHazards)
	{
		if (IsValid(Hazard))
		{
			Hazard->Destroy();
		}
	}
	SpawnedCauldronHazards.Reset();
	CauldronHazardExpirationTimes.Reset();
	UpdateCauldronHazardEffects();
#endif
}

void AWizardStaffGameMode::DebugEndCauldronCatastrophe()
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority())
	{
		EndCauldronCatastropheTrial();
	}
#endif
}

void AWizardStaffGameMode::DebugAddCauldronScore(int32 PlayerIndex, int32 Amount)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		AddCauldronScore(PlayerIndex, Amount, TEXT("Debug"));
	}
#endif
}

void AWizardStaffGameMode::DebugSetCauldronIntake(const FString& IntakeDirection)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		const EWizardCauldronIntakeDirection ParsedIntake = ParseCauldronIntakeDirection(IntakeDirection);
		if (ParsedIntake != EWizardCauldronIntakeDirection::None)
		{
			SetCauldronActiveIntake(ParsedIntake);
		}
	}
#endif
}

void AWizardStaffGameMode::DebugRotateCauldronIntake()
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		BeginCauldronIntakeRelocation();
	}
#endif
}

void AWizardStaffGameMode::DebugPrintCauldronIntakeState()
{
#if !UE_BUILD_SHIPPING
	auto GetIntakeText = [](EWizardCauldronIntakeDirection IntakeDirection)
	{
		switch (IntakeDirection)
		{
		case EWizardCauldronIntakeDirection::North: return TEXT("North");
		case EWizardCauldronIntakeDirection::East: return TEXT("East");
		case EWizardCauldronIntakeDirection::South: return TEXT("South");
		case EWizardCauldronIntakeDirection::West: return TEXT("West");
		default: return TEXT("None");
		}
	};
	UE_LOG(LogTemp, Log, TEXT("Cauldron intake: active %s | preview %s | relocating %s | remaining %.2fs"),
		GetIntakeText(ActiveCauldronIntake),
		GetIntakeText(PreviewCauldronIntake),
		bCauldronIntakeRelocating ? TEXT("true") : TEXT("false"),
		CauldronIntakeRelocationRemainingTime);
#endif
}

void AWizardStaffGameMode::DebugStartCauldronBanking(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority() && bCauldronCatastropheActive)
	{
		if (AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex))
		{
			StartCauldronBanking(Wizard, NextDebugCauldronDepositSequence--, true);
		}
	}
#endif
}

void AWizardStaffGameMode::DebugInterruptCauldronBanking()
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority())
	{
		EndCauldronBanking(TEXT("Debug interrupt"));
	}
#endif
}

void AWizardStaffGameMode::DebugPrintCauldronBankingState()
{
#if !UE_BUILD_SHIPPING
	const AWizardStaffWizardCharacter* Banker = CauldronBankingWizard.Get();
	const int32 BankerIndex = GetPlayerIndexForWizard(Banker);
	const float Distance = IsValid(Banker) && IsValid(ActiveCauldronArena) && CauldronBankingIntake != EWizardCauldronIntakeDirection::None
		? FVector::Dist2D(Banker->GetActorLocation(), ActiveCauldronArena->GetIntakeWorldLocation(CauldronBankingIntake))
		: 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Cauldron banking: %s | P%d | session %d | intake %d | transferred %d | next %.2fs | distance %.0f/%.0f | last '%s'"),
		IsValid(Banker) ? TEXT("active") : TEXT("inactive"),
		BankerIndex + 1,
		CauldronBankingSessionGeneration,
		static_cast<int32>(CauldronBankingIntake),
		CauldronBankingTransferredCount,
		CauldronBankingNextTransferRemainingTime,
		Distance,
		CauldronCatastropheTuning.BankingMaximumDistanceFromIntake,
		*CauldronBankingLastEndReason);
#endif
}

bool AWizardStaffGameMode::HandleCauldronIngredientBonked(AWizardStaffWizardCharacter* Attacker, AWizardStaffCauldronIngredient* Ingredient)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Attacker) || !IsValid(Ingredient) || Ingredient->IsScored())
	{
		return false;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Attacker);
	if (PlayerIndex == INDEX_NONE)
	{
		return false;
	}

	FVector BonkDirection = Ingredient->GetActorLocation() - Attacker->GetActorLocation();
	BonkDirection.Z = FMath::Max(BonkDirection.Z, 0.18f);
	return Ingredient->ApplyAuthoritativeBonk(PlayerIndex, BonkDirection, CauldronCatastropheTuning.IngredientBonkImpulse);
}

void AWizardStaffGameMode::GatherCauldronIngredientsInBonkBox(const FVector& BoxCenter, const FQuat& BoxRotation, const FVector& BoxExtent, TArray<AWizardStaffCauldronIngredient*>& OutIngredients) const
{
	OutIngredients.Reset();
	if (!bCauldronCatastropheActive)
	{
		return;
	}

	const FVector ExpandedExtent = BoxExtent + FVector(52.0f, 52.0f, 52.0f);
	for (AWizardStaffCauldronIngredient* Ingredient : SpawnedCauldronIngredients)
	{
		if (!IsValid(Ingredient) || Ingredient->IsScored())
		{
			continue;
		}

		const FVector LocalOffset = BoxRotation.UnrotateVector(Ingredient->GetActorLocation() - BoxCenter);
		if (FMath::Abs(LocalOffset.X) <= ExpandedExtent.X
			&& FMath::Abs(LocalOffset.Y) <= ExpandedExtent.Y
			&& FMath::Abs(LocalOffset.Z) <= ExpandedExtent.Z)
		{
			OutIngredients.Add(Ingredient);
		}
	}
}

void AWizardStaffGameMode::GatherCauldronIngredientsInBonkArc(const AWizardStaffWizardCharacter* Attacker, TArray<AWizardStaffCauldronIngredient*>& OutIngredients) const
{
	OutIngredients.Reset();
	if (!bCauldronCatastropheActive || !IsValid(Attacker))
	{
		return;
	}

	FVector Forward = Attacker->GetActorForwardVector();
	Forward.Z = 0.0f;
	if (!Forward.Normalize())
	{
		return;
	}

	const FVector AttackerLocation = Attacker->GetActorLocation();
	const float MaxReach = FMath::Max(Attacker->GetQuickBonkRange() + 140.0f, 260.0f);
	const float MaxReachSquared = FMath::Square(MaxReach);
	constexpr float MinForwardDot = -0.35f;
	constexpr float MaxVerticalOffset = 160.0f;

	for (AWizardStaffCauldronIngredient* Ingredient : SpawnedCauldronIngredients)
	{
		if (!IsValid(Ingredient) || Ingredient->IsScored())
		{
			continue;
		}

		FVector ToIngredient = Ingredient->GetActorLocation() - AttackerLocation;
		if (FMath::Abs(ToIngredient.Z) > MaxVerticalOffset)
		{
			continue;
		}

		ToIngredient.Z = 0.0f;
		if (ToIngredient.SizeSquared() <= MaxReachSquared && ToIngredient.Normalize() && FVector::DotProduct(Forward, ToIngredient) >= MinForwardDot)
		{
			OutIngredients.Add(Ingredient);
		}
	}
}

void AWizardStaffGameMode::NotifyCauldronWizardBonked(AWizardStaffWizardCharacter* Attacker, AWizardStaffWizardCharacter* Target)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Attacker) || !IsValid(Target) || Attacker == Target)
	{
		return;
	}

	ApplyCauldronVialInstabilityForEnemyBonk(Attacker, Target);

	if (Target == CauldronBankingWizard.Get())
	{
		EndCauldronBanking(TEXT("Enemy bonk"));
	}

	if (bCauldronCurseDepositSequenceActive)
	{
		if (IsValid(ActiveCauldronArena))
		{
			ActiveCauldronArena->SetCurseWarningActive(false);
			ActiveCauldronArena->SetCurseDepositWarningActive(true);
		}
	}
	else if (bCauldronCurseGrounded)
	{
		return;
	}

	const int32 AttackerIndex = GetPlayerIndexForWizard(Attacker);
	const int32 TargetIndex = GetPlayerIndexForWizard(Target);
	if (AttackerIndex == CauldronCursedPlayerIndex && TargetIndex != INDEX_NONE)
	{
		CauldronCursedPlayerIndex = TargetIndex;
		LastCauldronCursedPlayerIndex = TargetIndex;
		for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
		{
			if (Wizard)
			{
				Wizard->SetCauldronCurseState(GetPlayerIndexForWizard(Wizard) == TargetIndex, CauldronCurseRemainingTime);
			}
		}
		PublishReplicatedGameplayEvent(EWizardReplicatedGameplayEventType::CauldronCurse, FString::Printf(TEXT("P%d passed the curse to P%d"), AttackerIndex + 1, TargetIndex + 1), AttackerIndex, TargetIndex, CauldronCurseRemainingTime, false);
		SyncReplicatedObservableState();
	}
}

void AWizardStaffGameMode::NotifyCauldronCursedWizardStaffSnapped(AWizardStaffWizardCharacter* Wizard)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || bCauldronCurseGrounded || !IsValid(Wizard)
		|| GetPlayerIndexForWizard(Wizard) != CauldronCursedPlayerIndex)
	{
		return;
	}

	AActor* SnappedSegment = nullptr;
	for (int32 SegmentIndex = LooseSnappedSegments.Num() - 1; SegmentIndex >= 0; --SegmentIndex)
	{
		const FWizardTrackedLooseSegment& TrackedSegment = LooseSnappedSegments[SegmentIndex];
		if (TrackedSegment.SourceWizard.Get() == Wizard && IsValid(TrackedSegment.Actor.Get()))
		{
			SnappedSegment = TrackedSegment.Actor.Get();
			break;
		}
	}

	CauldronGroundCurseSegment = SnappedSegment;
	CauldronGroundCurseLastLocation = SnappedSegment ? SnappedSegment->GetActorLocation() : Wizard->GetCauldronCurseOrbWorldLocation();
	bCauldronCurseGrounded = true;
	CauldronCursedPlayerIndex = INDEX_NONE;
	Wizard->SetCauldronCurseState(false, 0.0f);
	if (SnappedSegment)
	{
		Wizard->AttachCauldronCurseOrbToLooseSegment(SnappedSegment);
	}
	else
	{
		Wizard->DetachCauldronCurseOrbAtWorldLocation(CauldronGroundCurseLastLocation);
	}
	PublishReplicatedGameplayEvent(EWizardReplicatedGameplayEventType::CauldronCurse, TEXT("The cursed staff segment hit the ground"), INDEX_NONE, INDEX_NONE, CauldronCurseRemainingTime, false);
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::UpdateCauldronCatastrophe(float DeltaSeconds)
{
	if (!HasAuthority() || !bCauldronCatastropheActive)
	{
		return;
	}

	CauldronRemainingTime = FMath::Max(CauldronRemainingTime - DeltaSeconds, 0.0f);
	if (CauldronRemainingTime <= 0.0f)
	{
		EndCauldronCatastropheTrial();
		return;
	}

	UpdateCauldronLastSafePositions();
	UpdateCauldronIntakeRelocation(DeltaSeconds);
	UpdateCauldronBanking(DeltaSeconds);
	UpdateCauldronCurseDepositSequence(DeltaSeconds);

	for (int32 VialIndex = SpawnedCauldronVials.Num() - 1; VialIndex >= 0; --VialIndex)
	{
		if (!IsValid(SpawnedCauldronVials[VialIndex]))
		{
			SpawnedCauldronVials.RemoveAtSwap(VialIndex);
		}
	}

	for (int32 RespawnIndex = PendingCauldronVialRespawns.Num() - 1; RespawnIndex >= 0; --RespawnIndex)
	{
		PendingCauldronVialRespawns[RespawnIndex] -= DeltaSeconds;
		if (PendingCauldronVialRespawns[RespawnIndex] <= 0.0f)
		{
			SpawnCauldronVial();
			PendingCauldronVialRespawns.RemoveAtSwap(RespawnIndex);
		}
	}


	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (int32 HazardIndex = SpawnedCauldronHazards.Num() - 1; HazardIndex >= 0; --HazardIndex)
	{
		AWizardStaffCauldronHazard* Hazard = SpawnedCauldronHazards[HazardIndex];
		const TWeakObjectPtr<AWizardStaffCauldronHazard> HazardKey(Hazard);
		const float* ExpirationTime = CauldronHazardExpirationTimes.Find(HazardKey);
		if (!IsValid(Hazard) || (ExpirationTime && Now >= *ExpirationTime))
		{
			CauldronHazardExpirationTimes.Remove(HazardKey);
			if (IsValid(Hazard))
			{
				Hazard->Destroy();
			}
			SpawnedCauldronHazards.RemoveAtSwap(HazardIndex);
		}
	}
	UpdateCauldronHazardEffects();

	if (bCauldronCurseDepositSequenceActive)
	{
		if (IsValid(ActiveCauldronArena))
		{
			ActiveCauldronArena->SetCurseWarningActive(false);
			ActiveCauldronArena->SetCurseDepositWarningActive(true);
		}
	}
	else if (bCauldronCurseGrounded)
	{
		if (IsValid(ActiveCauldronArena))
		{
			ActiveCauldronArena->SetCurseWarningActive(false);
		}
		if (AActor* GroundSegment = CauldronGroundCurseSegment.Get())
		{
			CauldronGroundCurseLastLocation = GroundSegment->GetActorLocation();
		}
		CauldronCurseRemainingTime = FMath::Max(CauldronCurseRemainingTime - DeltaSeconds, 0.0f);
		if (CauldronCurseRemainingTime <= 0.0f)
		{
			ExplodeGroundedCauldronCurse();
		}
	}
	else if (CauldronCursedPlayerIndex != INDEX_NONE)
	{
		if (IsValid(ActiveCauldronArena))
		{
			ActiveCauldronArena->SetCurseWarningActive(false);
		}
		AWizardStaffWizardCharacter* Holder = GetWizardForPlayerIndex(CauldronCursedPlayerIndex);
		if (!IsValid(Holder) || Holder->IsReadableOutOfArenaRespawning())
		{
			ClearCauldronCurse(true);
		}
		else
		{
			const float CountdownRate = Holder->IsInStaffClash() ? CauldronCatastropheTuning.StaffClashCountdownRate : 1.0f;
			CauldronCurseRemainingTime = FMath::Max(CauldronCurseRemainingTime - (DeltaSeconds * FMath::Clamp(CountdownRate, 0.0f, 1.0f)), 0.0f);
			Holder->SetCauldronCurseState(true, CauldronCurseRemainingTime);
			if (CauldronCurseRemainingTime <= 0.0f)
			{
				ExplodeCauldronCurse();
			}
		}
	}
	else
	{
		CauldronNextCurseTime -= DeltaSeconds;
		if (IsValid(ActiveCauldronArena))
		{
			ActiveCauldronArena->SetCurseWarningActive(CauldronNextCurseTime > 0.0f && CauldronNextCurseTime <= 3.5f);
		}
		if (CauldronNextCurseTime <= 0.0f)
		{
			AssignCauldronCurse();
		}
	}
}

void AWizardStaffGameMode::CleanupCauldronGameplayState()
{
	EndCauldronBanking(TEXT("Trial cleanup"), false);
	bCauldronCatastropheActive = false;
	CauldronRemainingTime = 0.0f;
	PendingCauldronIngredientRespawns.Reset();
	PendingCauldronVialRespawns.Reset();

	CauldronNextCurseTime = 0.0f;
	LastCauldronCursedPlayerIndex = INDEX_NONE;
	bCauldronCurseDepositSequenceActive = false;
	CauldronCurseDepositSequenceRemainingTime = 0.0f;
	ResetCauldronIntakeState();
	ClearCauldronCurse(false);

	for (AWizardStaffCauldronIngredient* Ingredient : SpawnedCauldronIngredients)
	{
		if (IsValid(Ingredient))
		{
			Ingredient->Destroy();
		}
	}
	SpawnedCauldronIngredients.Reset();
	ClearCauldronVials(true);
	ResetCauldronSpillState();

	for (AWizardStaffCauldronHazard* Hazard : SpawnedCauldronHazards)
	{
		if (IsValid(Hazard))
		{
			Hazard->Destroy();
		}
	}
	SpawnedCauldronHazards.Reset();
	CauldronHazardExpirationTimes.Reset();
	for (AWizardStaffCauldronCurseBomb* Bomb : SpawnedCauldronCurseDepositBombs)
	{
		if (IsValid(Bomb))
		{
			Bomb->Destroy();
		}
	}
	SpawnedCauldronCurseDepositBombs.Reset();
	ClearCauldronDepositArcs();

	if (IsValid(ActiveCauldronArena))
	{
		ActiveCauldronArena->SetCurseWarningActive(false);
		ActiveCauldronArena->SetCurseDepositWarningActive(false);
	}

	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			Wizard->ClearCauldronSlipperySkid();
			Wizard->SetCauldronHazardMovementMultipliers(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			Wizard->SetCauldronStickyTetherState(false);
			Wizard->SetCauldronCurseState(false, 0.0f);
		}
	}
}

void AWizardStaffGameMode::CleanupCauldronCatastrophe()
{
	CleanupCauldronGameplayState();

	if (IsValid(ActiveCauldronArena))
	{
		ActiveCauldronArena->SetCurseDepositWarningActive(false);
		ActiveCauldronArena->Destroy();
	}
	ActiveCauldronArena = nullptr;
}

void AWizardStaffGameMode::SpawnCauldronIngredient()
{
	if (!HasAuthority() || !bCauldronCatastropheActive || SpawnedCauldronIngredients.Num() >= FMath::Max(CauldronCatastropheTuning.MaximumActiveIngredients, 1))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Angle = FMath::FRandRange(0.0f, 2.0f * UE_PI);
	const FVector SpawnLocation = GetArenaCenter() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * CauldronCatastropheTuning.IngredientSpawnRadius + FVector(0.0f, 0.0f, 90.0f);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AWizardStaffCauldronIngredient* Ingredient = World->SpawnActor<AWizardStaffCauldronIngredient>(AWizardStaffCauldronIngredient::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
	if (Ingredient)
	{
		Ingredient->InitializeIngredient(FMath::RandRange(0, 3));
		SpawnedCauldronIngredients.Add(Ingredient);
	}
}

EWizardCauldronVialType AWizardStaffGameMode::ParseCauldronVialType(const FString& TypeText) const
{
	if (TypeText.Equals(TEXT("Speed"), ESearchCase::IgnoreCase))
	{
		return EWizardCauldronVialType::Speed;
	}
	if (TypeText.Equals(TEXT("BurdeningPower"), ESearchCase::IgnoreCase) || TypeText.Equals(TEXT("Burdening Power"), ESearchCase::IgnoreCase))
	{
		return EWizardCauldronVialType::BurdeningPower;
	}
	UE_LOG(LogTemp, Warning, TEXT("Unknown Cauldron vial type '%s'. Use Speed or BurdeningPower."), *TypeText);
	return EWizardCauldronVialType::None;
}

bool AWizardStaffGameMode::ValidateCauldronVialState(const AWizardStaffWizardCharacter* Wizard, FString& OutReport) const
{
	if (!IsValid(Wizard) || !Wizard->StaffComponent)
	{
		OutReport = TEXT("invalid wizard or staff component");
		return false;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	const TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(Wizard);
	const TArray<FName>& SegmentTags = Wizard->StaffComponent->GetStaffSegmentTags();
	TSet<FName> StackTags;
	TSet<FName> StaffVialTags;
	bool bValid = true;
	FString Failure;

	if (Stack)
	{
		for (const FCauldronVialStackEntry& Entry : *Stack)
		{
			if (Entry.SegmentTag.IsNone() || StackTags.Contains(Entry.SegmentTag))
			{
				bValid = false;
				Failure = FString::Printf(TEXT("duplicate or invalid stack tag %s"), *Entry.SegmentTag.ToString());
				break;
			}
			StackTags.Add(Entry.SegmentTag);
			if (!SegmentTags.Contains(Entry.SegmentTag))
			{
				bValid = false;
				Failure = FString::Printf(TEXT("stack tag %s has no staff segment"), *Entry.SegmentTag.ToString());
				break;
			}
		}
	}

	for (const FName SegmentTag : SegmentTags)
	{
		if (SegmentTag.ToString().StartsWith(TEXT("CauldronVial_")))
		{
			StaffVialTags.Add(SegmentTag);
			if (!StackTags.Contains(SegmentTag))
			{
				bValid = false;
				Failure = FString::Printf(TEXT("staff vial tag %s has no stack entry"), *SegmentTag.ToString());
				break;
			}
		}
	}

	const int32 StackCount = Stack ? Stack->Num() : 0;
	const EWizardCauldronVialType ExpectedActive = StackCount > 0 ? Stack->Last().Type : EWizardCauldronVialType::None;
	if (Wizard->GetReadableCauldronVialCount() != StackCount)
	{
		bValid = false;
		Failure = TEXT("readable vial count differs from stack count");
	}
	else if (Wizard->GetReadableActiveCauldronVial() != ExpectedActive)
	{
		bValid = false;
		Failure = TEXT("readable active vial differs from newest stack entry");
	}
	else if (StackCount > 0 && Wizard->StaffComponent->GetTopStaffSegmentTag() != Stack->Last().SegmentTag)
	{
		// This can be a safe blocked deposit when an unrelated segment is on top.
		Failure = FString::Printf(TEXT("top segment %s blocks newest vial %s"), *Wizard->StaffComponent->GetTopStaffSegmentTag().ToString(), *Stack->Last().SegmentTag.ToString());
	}

	const FString FailureSuffix = Failure.IsEmpty() ? FString() : FString::Printf(TEXT(" | %s"), *Failure);
	OutReport = FString::Printf(
		TEXT("%s | stack %d | staff vial tags %d | readable %d | active %s | top %s%s"),
		bValid ? TEXT("VALID") : TEXT("INVALID"),
		StackCount,
		StaffVialTags.Num(),
		Wizard->GetReadableCauldronVialCount(),
		*GetWizardCauldronVialDisplayName(Wizard->GetReadableActiveCauldronVial()),
		*Wizard->StaffComponent->GetTopStaffSegmentTag().ToString(),
		*FailureSuffix);
	if (!bValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cauldron vial mismatch P%d: %s"), PlayerIndex + 1, *OutReport);
	}
	return bValid;
}

bool AWizardStaffGameMode::IsCauldronVialSpawnLocationValid(const FVector& CandidateLocation) const
{
	const float CandidateRadius = FVector::Dist2D(CandidateLocation, GetArenaCenter());
	if (CandidateRadius < CauldronCatastropheTuning.VialMinimumCauldronDistance
		|| CandidateRadius > FMath::Max(GetCurrentPlayBoundsHalfSize() - CauldronCatastropheTuning.VialMinimumDropOffDistance, 0.0f))
	{
		return false;
	}

	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (IsValid(Wizard) && FVector::DistSquared2D(CandidateLocation, Wizard->GetActorLocation()) < FMath::Square(CauldronCatastropheTuning.VialMinimumPlayerSpacing))
		{
			return false;
		}
	}
	for (const AWizardStaffCauldronVialPickup* Vial : SpawnedCauldronVials)
	{
		if (IsValid(Vial) && FVector::DistSquared2D(CandidateLocation, Vial->GetActorLocation()) < FMath::Square(CauldronCatastropheTuning.VialMinimumVialSpacing))
		{
			return false;
		}
	}
	return true;
}

void AWizardStaffGameMode::SpawnCauldronVial(EWizardCauldronVialType ForcedVialType)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || SpawnedCauldronVials.Num() >= FMath::Max(CauldronCatastropheTuning.MaximumActiveVials, 1))
	{
		return;
	}

	FVector SpawnLocation = GetArenaCenter() + FVector(CauldronCatastropheTuning.VialMinimumCauldronDistance, 0.0f, 0.0f);
	const float MaxRadius = FMath::Max(CauldronCatastropheTuning.VialMinimumCauldronDistance, FMath::Min(CauldronCatastropheTuning.VialSpawnRadius, GetCurrentPlayBoundsHalfSize() - CauldronCatastropheTuning.VialMinimumDropOffDistance));
	for (int32 Attempt = 0; Attempt < 12; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.0f, 2.0f * UE_PI);
		const float Radius = FMath::FRandRange(CauldronCatastropheTuning.VialMinimumCauldronDistance, MaxRadius);
		const FVector Candidate = GetArenaCenter() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Radius;
		if (IsCauldronVialSpawnLocationValid(Candidate))
		{
			SpawnLocation = Candidate;
			break;
		}
	}

	const EWizardCauldronVialType VialType = ForcedVialType == EWizardCauldronVialType::None
		? (FMath::RandBool() ? EWizardCauldronVialType::Speed : EWizardCauldronVialType::BurdeningPower)
		: ForcedVialType;
	SpawnCauldronVialAtLocation(VialType, SpawnLocation, false);
}

bool AWizardStaffGameMode::SpawnCauldronVialAtLocation(EWizardCauldronVialType VialType, const FVector& SpawnLocation, bool bSpilledPickup)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || VialType == EWizardCauldronVialType::None)
	{
		return false;
	}

	if (!bSpilledPickup && SpawnedCauldronVials.Num() >= FMath::Max(CauldronCatastropheTuning.MaximumActiveVials, 1))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AWizardStaffCauldronVialPickup* Vial = World->SpawnActor<AWizardStaffCauldronVialPickup>(
		AWizardStaffCauldronVialPickup::StaticClass(),
		SpawnLocation + FVector(0.0f, 0.0f, 2.0f),
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!Vial)
	{
		return false;
	}

	Vial->InitializeVial(VialType, CauldronCatastropheTuning.VialPickupRadius);
	SpawnedCauldronVials.Add(Vial);
	if (bSpilledPickup)
	{
		CauldronSpilledVialPickups.Add(Vial);
	}
	return true;
}

bool AWizardStaffGameMode::HandleCauldronVialCollected(AWizardStaffWizardCharacter* Collector, EWizardCauldronVialType VialType)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Collector) || VialType == EWizardCauldronVialType::None)
	{
		return false;
	}

	if (!GrantCauldronVial(Collector, VialType))
	{
		return false;
	}
	PendingCauldronVialRespawns.Add(FMath::Max(CauldronCatastropheTuning.VialRespawnDelay, 0.0f));
	const int32 PlayerIndex = GetPlayerIndexForWizard(Collector);
	AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("P%d collected %s vial"), PlayerIndex + 1, *GetWizardCauldronVialDisplayName(VialType)), GetWizardCauldronVialColor(VialType).ToFColor(true), 1.8f, EWizardHudMessageCategory::Gameplay);
	return true;
}

bool AWizardStaffGameMode::GrantCauldronVial(AWizardStaffWizardCharacter* Wizard, EWizardCauldronVialType VialType)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Wizard) || !Wizard->StaffComponent || VialType == EWizardCauldronVialType::None)
	{
		return false;
	}

	const FName SegmentTag(*FString::Printf(TEXT("CauldronVial_%d"), NextCauldronVialPickupOrder));
	const int32 PreviousCount = Wizard->StaffComponent->GetSegmentCount();
	const int32 NewCount = Wizard->StaffComponent->AddStaffSegmentWithTag(SegmentTag, GetWizardCauldronVialColor(VialType));
	if (NewCount <= PreviousCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cauldron vial grant rejected: staff cannot add a segment."));
		return false;
	}

	FCauldronVialStackEntry& NewEntry = CauldronVialStacks.FindOrAdd(Wizard).AddDefaulted_GetRef();
	NewEntry.Type = VialType;
	NewEntry.SegmentTag = SegmentTag;
	NewEntry.PickupOrder = NextCauldronVialPickupOrder++;
	RefreshCauldronVialEffects(Wizard);
	Wizard->SyncReplicatedStaffSegmentCountFromAuthority();
	return true;
}

void AWizardStaffGameMode::NotifyCauldronVialSegmentSnapped(AWizardStaffWizardCharacter* Wizard, FName RemovedSegmentTag)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Wizard) || RemovedSegmentTag.IsNone())
	{
		return;
	}

	TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(Wizard);
	if (!Stack)
	{
		return;
	}
	for (int32 EntryIndex = Stack->Num() - 1; EntryIndex >= 0; --EntryIndex)
	{
		if ((*Stack)[EntryIndex].SegmentTag == RemovedSegmentTag)
		{
			const FString LostVialName = GetWizardCauldronVialDisplayName((*Stack)[EntryIndex].Type);
			Stack->RemoveAt(EntryIndex);
			AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("%s vial shattered"), *LostVialName), FColor::Red, 1.6f, EWizardHudMessageCategory::Gameplay);
			RefreshCauldronVialEffects(Wizard);
			return;
		}
	}
}

bool AWizardStaffGameMode::IsCauldronWithinQuickBonkStrike(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!IsValid(Wizard) || !IsValid(ActiveCauldronArena) || bCauldronIntakeRelocating || ActiveCauldronIntake == EWizardCauldronIntakeDirection::None)
	{
		return false;
	}

	FVector Forward = Wizard->GetActorForwardVector();
	Forward.Z = 0.0f;
	if (!Forward.Normalize())
	{
		return false;
	}

	const FVector IntakeCenter = ActiveCauldronArena->GetIntakeWorldLocation(ActiveCauldronIntake);
	const float BankingMaximumDistance = FMath::Max(CauldronCatastropheTuning.BankingMaximumDistanceFromIntake, 50.0f);
	if (FVector::DistSquared2D(Wizard->GetActorLocation(), IntakeCenter) > FMath::Square(BankingMaximumDistance))
	{
		return false;
	}

	const float IntakeAcceptanceRadius = FMath::Max(CauldronCatastropheTuning.ActiveIntakeAcceptanceRadius, 50.0f);
	const auto IsWithinIntakeAcceptance = [&IntakeCenter, IntakeAcceptanceRadius](const FVector& CandidatePoint)
	{
		return FVector::DistSquared2D(CandidatePoint, IntakeCenter) <= FMath::Square(IntakeAcceptanceRadius)
			&& FMath::Abs(CandidatePoint.Z - IntakeCenter.Z) <= 240.0f;
	};

	const FVector ProjectedStrikePoint = Wizard->GetActorLocation() + (Forward * Wizard->GetQuickBonkRange());
	if (IsWithinIntakeAcceptance(ProjectedStrikePoint))
	{
		return true;
	}

	if (const UWizardStaffComponent* StaffComponent = Wizard->StaffComponent)
	{
		if (const UBoxComponent* StaffCollisionBox = StaffComponent->GetStaffCollisionBox())
		{
			const FTransform CollisionTransform = StaffCollisionBox->GetComponentTransform();
			const FVector CollisionExtent = StaffCollisionBox->GetScaledBoxExtent();
			const FVector LocalIntakeCenter = CollisionTransform.InverseTransformPosition(IntakeCenter);
			const FVector ClosestLocalPoint = LocalIntakeCenter.BoundToBox(-CollisionExtent, CollisionExtent);
			const FVector ClosestStaffPoint = CollisionTransform.TransformPosition(ClosestLocalPoint);
			return IsWithinIntakeAcceptance(ClosestStaffPoint);
		}
	}

	return false;
}

bool AWizardStaffGameMode::HandleCauldronQuickBonk(AWizardStaffWizardCharacter* Attacker, int32 BonkSequence)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Attacker) || BonkSequence <= 0 || !IsCauldronWithinQuickBonkStrike(Attacker))
	{
		return false;
	}

	if (!bCauldronCurseGrounded && GetPlayerIndexForWizard(Attacker) == CauldronCursedPlayerIndex)
	{
		return BeginCauldronCurseDeposit(Attacker, BonkSequence);
	}

	return StartCauldronBanking(Attacker, BonkSequence);
}

bool AWizardStaffGameMode::BeginCauldronCurseDeposit(AWizardStaffWizardCharacter* Wizard, int32 BonkSequence)
{
	const int32 WizardIndex = GetPlayerIndexForWizard(Wizard);
	if (!HasAuthority() || !bCauldronCatastropheActive || bCauldronCurseDepositSequenceActive || !IsValid(Wizard)
		|| WizardIndex == INDEX_NONE || WizardIndex != CauldronCursedPlayerIndex || BonkSequence <= 0 || !IsCauldronWithinQuickBonkStrike(Wizard))
	{
		return false;
	}

	EndCauldronBanking(TEXT("Cursed orb deposit"), false);
	ClearCauldronCurse(false);
	bCauldronCurseDepositSequenceActive = true;
	const int32 ExplosionCount = FMath::Clamp(CauldronCatastropheTuning.CurseDepositExplosionCount, 1, 12);
	CauldronCurseDepositSequenceRemainingTime = FMath::Max(CauldronCatastropheTuning.CurseDepositBoilWarningDuration, 0.1f)
		+ (FMath::Max(ExplosionCount - 1, 0) * FMath::Max(CauldronCatastropheTuning.CurseDepositExplosionLaunchInterval, 0.01f))
		+ FMath::Max(CauldronCatastropheTuning.CurseDepositExplosionFlightDuration, 0.05f)
		+ 0.30f;
	if (IsValid(ActiveCauldronArena))
	{
		ActiveCauldronArena->SetCurseWarningActive(false);
		ActiveCauldronArena->SetCurseDepositWarningActive(true);
		ActiveCauldronArena->TriggerDepositPulse();
	}
	SpawnCauldronCurseDepositBombs();
	PublishReplicatedGameplayEvent(EWizardReplicatedGameplayEventType::CauldronCurse, FString::Printf(TEXT("P%d fed the curse to the cauldron!"), WizardIndex + 1), WizardIndex, INDEX_NONE, static_cast<float>(ExplosionCount), false);
	SyncReplicatedObservableState();
	return true;
}

void AWizardStaffGameMode::SpawnCauldronCurseDepositBombs()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World || !IsValid(ActiveCauldronArena))
	{
		return;
	}

	const int32 ExplosionCount = FMath::Clamp(CauldronCatastropheTuning.CurseDepositExplosionCount, 1, 12);
	const float MinimumDistance = FMath::Max(CauldronCatastropheTuning.CurseDepositExplosionMinimumDistance, 0.0f);
	const float SafeMaximumDistance = FMath::Max(CauldronCatastropheTuning.CurseDepositExplosionMinimumDistance, FMath::Min(CauldronCatastropheTuning.CurseDepositExplosionMaximumDistance, GetCurrentPlayBoundsHalfSize() - 240.0f));
	const float OriginHeight = 150.0f;
	const FVector Origin = ActiveCauldronArena->GetAcceptanceCenter() + FVector(0.0f, 0.0f, OriginHeight);
	const float SectorAngle = (2.0f * UE_PI) / static_cast<float>(ExplosionCount);
	const float AngleOffset = FMath::FRandRange(0.0f, 2.0f * UE_PI);
	const float AngularJitter = SectorAngle * FMath::Clamp(CauldronCatastropheTuning.CurseDepositExplosionAngularJitterFraction, 0.0f, 0.49f);
	TArray<float> TargetAngles;
	TargetAngles.Reserve(ExplosionCount);
	for (int32 SectorIndex = 0; SectorIndex < ExplosionCount; ++SectorIndex)
	{
		const float SectorCenter = AngleOffset + ((static_cast<float>(SectorIndex) + 0.5f) * SectorAngle);
		TargetAngles.Add(SectorCenter + FMath::FRandRange(-AngularJitter, AngularJitter));
	}
	for (int32 ShuffleIndex = TargetAngles.Num() - 1; ShuffleIndex > 0; --ShuffleIndex)
	{
		TargetAngles.Swap(ShuffleIndex, FMath::RandRange(0, ShuffleIndex));
	}

	for (int32 ExplosionIndex = 0; ExplosionIndex < ExplosionCount; ++ExplosionIndex)
	{
		const float Angle = TargetAngles[ExplosionIndex];
		const float Distance = FMath::Sqrt(FMath::FRandRange(FMath::Square(MinimumDistance), FMath::Square(SafeMaximumDistance)));
		const FVector TargetLocation = GetArenaCenter() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Distance + FVector(0.0f, 0.0f, 8.0f);
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AWizardStaffCauldronCurseBomb* Bomb = World->SpawnActor<AWizardStaffCauldronCurseBomb>(AWizardStaffCauldronCurseBomb::StaticClass(), TargetLocation, FRotator::ZeroRotator, SpawnParameters);
		if (Bomb)
		{
			Bomb->InitializeBomb(
				Origin,
				TargetLocation,
				CauldronCatastropheTuning.CurseDepositExplosionRadius,
				FMath::Max(CauldronCatastropheTuning.CurseDepositBoilWarningDuration, 0.1f) + (ExplosionIndex * FMath::Max(CauldronCatastropheTuning.CurseDepositExplosionLaunchInterval, 0.01f)),
				CauldronCatastropheTuning.CurseDepositExplosionFlightDuration,
				CauldronCatastropheTuning.CurseDepositExplosionHorizontalKnockback,
				CauldronCatastropheTuning.CurseDepositExplosionVerticalKnockback);
			SpawnedCauldronCurseDepositBombs.Add(Bomb);
		}
	}
}

void AWizardStaffGameMode::UpdateCauldronCurseDepositSequence(float DeltaSeconds)
{
	for (int32 BombIndex = SpawnedCauldronCurseDepositBombs.Num() - 1; BombIndex >= 0; --BombIndex)
	{
		if (!IsValid(SpawnedCauldronCurseDepositBombs[BombIndex]))
		{
			SpawnedCauldronCurseDepositBombs.RemoveAtSwap(BombIndex);
		}
	}

	if (!bCauldronCurseDepositSequenceActive)
	{
		return;
	}

	CauldronCurseDepositSequenceRemainingTime = FMath::Max(CauldronCurseDepositSequenceRemainingTime - FMath::Max(DeltaSeconds, 0.0f), 0.0f);
	if (CauldronCurseDepositSequenceRemainingTime > 0.0f)
	{
		return;
	}

	bCauldronCurseDepositSequenceActive = false;
	if (IsValid(ActiveCauldronArena))
	{
		ActiveCauldronArena->SetCurseDepositWarningActive(false);
	}
	CauldronNextCurseTime = FMath::Max(CauldronCatastropheTuning.CurseCadence, 1.0f);
	SyncReplicatedObservableState();
}

int32 AWizardStaffGameMode::DepositCauldronVials(AWizardStaffWizardCharacter* Wizard, int32 BonkSequence)
{
	return StartCauldronBanking(Wizard, BonkSequence, true) ? 1 : 0;
}

bool AWizardStaffGameMode::StartCauldronBanking(AWizardStaffWizardCharacter* Wizard, int32 BonkSequence, bool bBypassIntakeStrike)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Wizard) || BonkSequence == 0 || CauldronBankingWizard.IsValid()
		|| bCauldronIntakeRelocating || ActiveCauldronIntake == EWizardCauldronIntakeDirection::None)
	{
		return false;
	}

	if (!bBypassIntakeStrike && !IsCauldronWithinQuickBonkStrike(Wizard))
	{
		return false;
	}

	int32& LastProcessedSequence = LastCauldronDepositBonkSequences.FindOrAdd(Wizard);
	if (LastProcessedSequence == BonkSequence)
	{
		return false;
	}
	LastProcessedSequence = BonkSequence;

	if (!IsCauldronBankingWizardValid(Wizard) || !HasValidTopCauldronVial(Wizard))
	{
		return false;
	}

	CauldronBankingWizard = Wizard;
	++CauldronBankingSessionGeneration;
	CauldronBankingIntake = ActiveCauldronIntake;
	CauldronBankingNextTransferRemainingTime = FMath::Max(CauldronCatastropheTuning.BankingInitialDelay, 0.0f);
	CauldronBankingTransferredCount = 0;
	CauldronBankingLastEndReason = TEXT("Banking");
	Wizard->SetCauldronBankingMovementMultipliers(true, CauldronCatastropheTuning.BankingMovementMultiplier, CauldronCatastropheTuning.BankingAccelerationMultiplier);
	ApplyCauldronBankingReadability();
	SyncReplicatedObservableState();
	return true;
}

void AWizardStaffGameMode::UpdateCauldronBanking(float DeltaSeconds)
{
	AWizardStaffWizardCharacter* Banker = CauldronBankingWizard.Get();
	if (!IsValid(Banker))
	{
		if (CauldronBankingIntake != EWizardCauldronIntakeDirection::None)
		{
			EndCauldronBanking(TEXT("Banker destroyed"));
		}
		return;
	}

	if (!IsCauldronBankingWizardValid(Banker))
	{
		EndCauldronBanking(TEXT("Banker invalid"));
		return;
	}

	if (!IsValid(ActiveCauldronArena) || bCauldronIntakeRelocating || ActiveCauldronIntake != CauldronBankingIntake)
	{
		EndCauldronBanking(TEXT("Intake unavailable"));
		return;
	}

	const float DistanceFromIntake = FVector::Dist2D(Banker->GetActorLocation(), ActiveCauldronArena->GetIntakeWorldLocation(CauldronBankingIntake));
	if (DistanceFromIntake > FMath::Max(CauldronCatastropheTuning.BankingMaximumDistanceFromIntake, 50.0f))
	{
		EndCauldronBanking(TEXT("Left intake range"));
		return;
	}

	CauldronBankingNextTransferRemainingTime = FMath::Max(CauldronBankingNextTransferRemainingTime - FMath::Max(DeltaSeconds, 0.0f), 0.0f);
	ApplyCauldronBankingReadability();
	if (CauldronBankingNextTransferRemainingTime > 0.0f)
	{
		return;
	}

	if (!TransferNextCauldronBankingVial())
	{
		EndCauldronBanking(TEXT("No removable top vial"));
		return;
	}

	if (!HasValidTopCauldronVial(Banker))
	{
		EndCauldronBanking(TEXT("Vial stack complete"));
		return;
	}

	CauldronBankingNextTransferRemainingTime = FMath::Max(CauldronCatastropheTuning.BankingVialTransferInterval, 0.05f);
	ApplyCauldronBankingReadability();
}

bool AWizardStaffGameMode::TransferNextCauldronBankingVial()
{
	AWizardStaffWizardCharacter* Banker = CauldronBankingWizard.Get();
	if (!IsCauldronBankingWizardValid(Banker) || !Banker->StaffComponent)
	{
		return false;
	}

	TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(Banker);
	if (!Stack || Stack->IsEmpty())
	{
		return false;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Banker);
	const FCauldronVialStackEntry& NewestEntry = Stack->Last();
	const EWizardCauldronVialType DepositedVialType = NewestEntry.Type;
	if (PlayerIndex == INDEX_NONE || Banker->StaffComponent->GetTopStaffSegmentTag() != NewestEntry.SegmentTag)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cauldron banking rejected P%d: top segment does not match newest vial."), PlayerIndex + 1);
		return false;
	}

	UStaticMesh* DepositSegmentMesh = nullptr;
	FTransform DepositSegmentTransform = FTransform::Identity;
	if (const UStaticMeshComponent* TopSegmentMesh = Banker->StaffComponent->GetTopStaffSegmentMesh())
	{
		DepositSegmentMesh = TopSegmentMesh->GetStaticMesh();
		DepositSegmentTransform = TopSegmentMesh->GetComponentTransform();
	}

	if (!Banker->StaffComponent->RemoveTopStaffSegment(false))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cauldron banking rejected P%d: expected vial segment could not be removed."), PlayerIndex + 1);
		return false;
	}

	SpawnCauldronDepositArc(DepositSegmentMesh, DepositSegmentTransform, DepositedVialType);
	Stack->Pop();
	++CauldronBankingTransferredCount;
	Banker->SyncReplicatedStaffSegmentCountFromAuthority();
	RefreshCauldronVialEffects(Banker);
	Banker->RefreshCauldronCurseOrbAttachment();
	if (IsValid(ActiveCauldronArena))
	{
		ActiveCauldronArena->TriggerDepositPulse();
	}
	if (FMath::FRand() <= CauldronCatastropheTuning.DepositHazardChance)
	{
		if (DepositedVialType == EWizardCauldronVialType::Speed)
		{
			SpawnCauldronHazard(EWizardCauldronHazardType::Slippery);
		}
		else if (DepositedVialType == EWizardCauldronVialType::BurdeningPower)
		{
			SpawnCauldronHazard(EWizardCauldronHazardType::Sticky);
		}
	}
	AddCauldronScore(PlayerIndex, 1, TEXT("Sequential Vial Deposit"));
	AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("P%d banked 1 vial"), PlayerIndex + 1), FColor::Yellow, 1.2f, EWizardHudMessageCategory::Gameplay);
	SyncReplicatedObservableState();
	return true;
}

void AWizardStaffGameMode::SpawnCauldronDepositArc(UStaticMesh* SegmentMesh, const FTransform& StartTransform, EWizardCauldronVialType VialType)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(ActiveCauldronArena) || !SegmentMesh)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWizardStaffCauldronDepositArc* DepositArc = World->SpawnActor<AWizardStaffCauldronDepositArc>(
		AWizardStaffCauldronDepositArc::StaticClass(),
		StartTransform.GetLocation(),
		StartTransform.Rotator(),
		SpawnParameters);
	if (!DepositArc)
	{
		return;
	}

	const FVector CauldronTarget = ActiveCauldronArena->GetActorLocation() + FVector(0.0f, 0.0f, 142.0f);
	DepositArc->InitializeDepositArc(SegmentMesh, StartTransform, CauldronTarget, GetWizardCauldronVialColor(VialType));
	SpawnedCauldronDepositArcs.Add(DepositArc);
}

void AWizardStaffGameMode::ClearCauldronDepositArcs()
{
	for (AWizardStaffCauldronDepositArc* DepositArc : SpawnedCauldronDepositArcs)
	{
		if (IsValid(DepositArc))
		{
			DepositArc->Destroy();
		}
	}
	SpawnedCauldronDepositArcs.Reset();
}
void AWizardStaffGameMode::EndCauldronBanking(const FString& EndReason, bool bRelocateAfterSuccess)
{
	AWizardStaffWizardCharacter* Banker = CauldronBankingWizard.Get();
	const bool bBankedAnyVials = CauldronBankingTransferredCount > 0;
	if (IsValid(Banker))
	{
		Banker->SetCauldronBankingMovementMultipliers(false);
	}

	CauldronBankingWizard.Reset();
	CauldronBankingIntake = EWizardCauldronIntakeDirection::None;
	CauldronBankingNextTransferRemainingTime = 0.0f;
	CauldronBankingLastEndReason = EndReason;
	CauldronBankingTransferredCount = 0;
	ApplyCauldronBankingReadability();
	SyncReplicatedObservableState();

	if (bRelocateAfterSuccess && bBankedAnyVials && bCauldronCatastropheActive)
	{
		BeginCauldronIntakeRelocation();
	}
}

bool AWizardStaffGameMode::IsCauldronBankingWizardValid(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!IsValid(Wizard) || !Wizard->GetController() || Wizard->IsReadableOutOfArenaRespawning() || Wizard->IsInStaffClash() || Wizard->IsBroomBoostActive())
	{
		return false;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(const_cast<AWizardStaffWizardCharacter*>(Wizard));
	return !PendingOutOfArenaRespawns.Contains(WizardKey) && !IsWizardOutOfArena(Wizard);
}

bool AWizardStaffGameMode::HasValidTopCauldronVial(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!IsValid(Wizard) || !Wizard->StaffComponent)
	{
		return false;
	}

	const TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(Wizard);
	return Stack && !Stack->IsEmpty() && Wizard->StaffComponent->GetTopStaffSegmentTag() == Stack->Last().SegmentTag;
}

void AWizardStaffGameMode::ApplyCauldronBankingReadability()
{
	if (!IsValid(ActiveCauldronArena))
	{
		return;
	}

	const bool bBankingActive = CauldronBankingWizard.IsValid();
	const float FullTransferDuration = CauldronBankingTransferredCount == 0
		? FMath::Max(CauldronCatastropheTuning.BankingInitialDelay, 0.01f)
		: FMath::Max(CauldronCatastropheTuning.BankingVialTransferInterval, 0.01f);
	const float ProgressAlpha = bBankingActive ? FMath::Clamp(1.0f - (CauldronBankingNextTransferRemainingTime / FullTransferDuration), 0.0f, 1.0f) : 0.0f;
	ActiveCauldronArena->SetIntakeBankingState(bBankingActive, ProgressAlpha);
}

void AWizardStaffGameMode::UpdateCauldronLastSafePositions()
{
	if (!HasAuthority() || !bCauldronCatastropheActive)
	{
		return;
	}

	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!IsValid(Wizard) || !Wizard->GetController())
		{
			continue;
		}

		const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
		if (PendingOutOfArenaRespawns.Contains(WizardKey) || !IsCauldronSafeSpillPosition(Wizard->GetActorLocation()))
		{
			continue;
		}

		CauldronLastSafePositions.Add(WizardKey, Wizard->GetActorLocation());
	}

	for (auto It = CauldronSpilledVialPickups.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Get()))
		{
			It.RemoveCurrent();
		}
	}
}

bool AWizardStaffGameMode::IsCauldronSafeSpillPosition(const FVector& Location) const
{
	if (!bCauldronCatastropheActive)
	{
		return false;
	}

	const FVector Center = GetArenaCenter();
	const float EdgeBuffer = FMath::Max(CauldronCatastropheTuning.RingOutSpillEdgeBuffer, CauldronCatastropheTuning.VialMinimumDropOffDistance);
	const float SafeHalfSize = FMath::Max(CauldronCatastropheTuning.ArenaHalfSize - EdgeBuffer, 100.0f);
	return FMath::Abs(Location.X - Center.X) <= SafeHalfSize
		&& FMath::Abs(Location.Y - Center.Y) <= SafeHalfSize
		&& Location.Z >= Center.Z - 90.0f;
}

FVector AWizardStaffGameMode::GetCauldronSpillCenter(const AWizardStaffWizardCharacter* Wizard) const
{
	const FVector ArenaCenter = GetArenaCenter();
	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(const_cast<AWizardStaffWizardCharacter*>(Wizard));
	FVector Candidate = CauldronLastSafePositions.Contains(WizardKey)
		? CauldronLastSafePositions[WizardKey]
		: (IsValid(Wizard) ? Wizard->GetActorLocation() : ArenaCenter);
	if (!IsCauldronSafeSpillPosition(Candidate) && IsValid(Wizard))
	{
		Candidate = GetSpawnTransformForController(Wizard->GetController()).GetLocation();
	}

	FVector Direction = Candidate - ArenaCenter;
	Direction.Z = 0.0f;
	if (!Direction.Normalize())
	{
		Direction = FVector::ForwardVector;
	}

	const float EdgeBuffer = FMath::Max(CauldronCatastropheTuning.RingOutSpillEdgeBuffer, CauldronCatastropheTuning.VialMinimumDropOffDistance);
	const float MaxRadius = FMath::Max(CauldronCatastropheTuning.ArenaHalfSize - EdgeBuffer, CauldronCatastropheTuning.VialMinimumCauldronDistance);
	const float Radius = FMath::Clamp(FVector::Dist2D(Candidate, ArenaCenter), CauldronCatastropheTuning.VialMinimumCauldronDistance, MaxRadius);
	return ArenaCenter + (Direction * Radius) + FVector(0.0f, 0.0f, 2.0f);
}

FVector AWizardStaffGameMode::FindCauldronSpillPickupLocation(const FVector& SpillCenter, int32 SpillIndex, int32 SpillCount) const
{
	const FVector ArenaCenter = GetArenaCenter();
	FVector RadialDirection = SpillCenter - ArenaCenter;
	RadialDirection.Z = 0.0f;
	if (!RadialDirection.Normalize())
	{
		RadialDirection = FVector::ForwardVector;
	}
	const FVector Perpendicular(-RadialDirection.Y, RadialDirection.X, 0.0f);
	const float Spread = FMath::Max(CauldronCatastropheTuning.RingOutSpillRadius, 40.0f);
	const float CenteredIndex = static_cast<float>(SpillIndex) - ((static_cast<float>(FMath::Max(SpillCount, 1)) - 1.0f) * 0.5f);

	for (int32 Attempt = 0; Attempt < 10; ++Attempt)
	{
		const float Ring = Attempt == 0 ? 0.0f : (Spread * (0.55f + (0.18f * Attempt)));
		const float SideOffset = CenteredIndex * Spread;
		const FVector Candidate = SpillCenter + (Perpendicular * SideOffset) + (RadialDirection * Ring);
		if (IsCauldronVialSpawnLocationValid(Candidate))
		{
			return Candidate;
		}
	}

	return GetCauldronSpillCenter(nullptr);
}

int32 AWizardStaffGameMode::SpillCauldronVials(AWizardStaffWizardCharacter* Wizard, bool bInterruptBanking)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Wizard) || !Wizard->StaffComponent)
	{
		return 0;
	}

	if (bInterruptBanking && Wizard == CauldronBankingWizard.Get())
	{
		EndCauldronBanking(TEXT("Ring-out spill"));
	}

	TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(Wizard);
	if (!Stack || Stack->IsEmpty())
	{
		return 0;
	}

	const int32 MaximumSpills = FMath::Min(Stack->Num(), FMath::Max(CauldronCatastropheTuning.RingOutVialsSpilled, 0));
	TArray<EWizardCauldronVialType, TInlineAllocator<2>> SpilledTypes;
	for (int32 SpillIndex = 0; SpillIndex < MaximumSpills; ++SpillIndex)
	{
		const FCauldronVialStackEntry NewestEntry = Stack->Last();
		if (Wizard->StaffComponent->GetTopStaffSegmentTag() != NewestEntry.SegmentTag)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cauldron ring-out spill stopped for P%d: top segment %s blocks vial %s."),
				GetPlayerIndexForWizard(Wizard) + 1,
				*Wizard->StaffComponent->GetTopStaffSegmentTag().ToString(),
				*NewestEntry.SegmentTag.ToString());
			break;
		}

		if (!Wizard->StaffComponent->RemoveTopStaffSegment(false))
		{
			UE_LOG(LogTemp, Warning, TEXT("Cauldron ring-out spill stopped for P%d: could not remove vial segment %s."),
				GetPlayerIndexForWizard(Wizard) + 1,
				*NewestEntry.SegmentTag.ToString());
			break;
		}

		Stack->Pop();
		SpilledTypes.Add(NewestEntry.Type);
	}

	if (SpilledTypes.IsEmpty())
	{
		return 0;
	}

	RefreshCauldronVialEffects(Wizard);
	Wizard->SyncReplicatedStaffSegmentCountFromAuthority();
	Wizard->RefreshCauldronCurseOrbAttachment();

	const FVector SpillCenter = GetCauldronSpillCenter(Wizard);
	int32 SpawnedPickupCount = 0;
	for (int32 SpillIndex = 0; SpillIndex < SpilledTypes.Num(); ++SpillIndex)
	{
		const FVector SpawnLocation = FindCauldronSpillPickupLocation(SpillCenter, SpillIndex, SpilledTypes.Num());
		if (SpawnCauldronVialAtLocation(SpilledTypes[SpillIndex], SpawnLocation, true))
		{
			++SpawnedPickupCount;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cauldron ring-out spill could not spawn P%d's %s vial pickup."),
				GetPlayerIndexForWizard(Wizard) + 1,
				*GetWizardCauldronVialDisplayName(SpilledTypes[SpillIndex]));
		}
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	const FString SpillMessage = FString::Printf(TEXT("P%d spilled %d vial%s!"), PlayerIndex + 1, SpilledTypes.Num(), SpilledTypes.Num() == 1 ? TEXT("") : TEXT("s"));
	PublishReplicatedGameplayEvent(EWizardReplicatedGameplayEventType::CauldronIngredientDeposited, SpillMessage, PlayerIndex, INDEX_NONE, static_cast<float>(SpilledTypes.Num()), false);
	SyncReplicatedObservableState();
	return SpawnedPickupCount;
}

void AWizardStaffGameMode::HandleCauldronRingOutSpill(AWizardStaffWizardCharacter* Wizard)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Wizard))
	{
		return;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
	if (CauldronProcessedRingOutSpills.Contains(WizardKey))
	{
		return;
	}

	CauldronProcessedRingOutSpills.Add(WizardKey);
	SpillCauldronVials(Wizard, true);
}

void AWizardStaffGameMode::ResetCauldronSpillState()
{
	CauldronLastSafePositions.Reset();
	CauldronProcessedRingOutSpills.Reset();
	CauldronSpilledVialPickups.Reset();
}
void AWizardStaffGameMode::ResetCauldronIntakeState()
{
	ActiveCauldronIntake = EWizardCauldronIntakeDirection::None;
	PreviewCauldronIntake = EWizardCauldronIntakeDirection::None;
	bCauldronIntakeRelocating = false;
	CauldronIntakeRelocationRemainingTime = 0.0f;
	ApplyCauldronIntakeReadability();
}

void AWizardStaffGameMode::SetCauldronActiveIntake(EWizardCauldronIntakeDirection NewIntake)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || NewIntake == EWizardCauldronIntakeDirection::None)
	{
		return;
	}

	ActiveCauldronIntake = NewIntake;
	PreviewCauldronIntake = EWizardCauldronIntakeDirection::None;
	bCauldronIntakeRelocating = false;
	CauldronIntakeRelocationRemainingTime = 0.0f;
	ApplyCauldronIntakeReadability();
}

void AWizardStaffGameMode::BeginCauldronIntakeRelocation()
{
	if (!HasAuthority() || !bCauldronCatastropheActive || bCauldronIntakeRelocating || ActiveCauldronIntake == EWizardCauldronIntakeDirection::None)
	{
		return;
	}

	const EWizardCauldronIntakeDirection NextIntake = ChooseNextCauldronIntake(ActiveCauldronIntake);
	if (NextIntake == EWizardCauldronIntakeDirection::None)
	{
		return;
	}

	ActiveCauldronIntake = EWizardCauldronIntakeDirection::None;
	PreviewCauldronIntake = NextIntake;
	bCauldronIntakeRelocating = true;
	CauldronIntakeRelocationRemainingTime = FMath::Max(CauldronCatastropheTuning.ActiveIntakeChangeWarningDuration, 0.0f);
	ApplyCauldronIntakeReadability();

	if (CauldronIntakeRelocationRemainingTime <= KINDA_SMALL_NUMBER)
	{
		UpdateCauldronIntakeRelocation(0.0f);
	}
}

void AWizardStaffGameMode::UpdateCauldronIntakeRelocation(float DeltaSeconds)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !bCauldronIntakeRelocating)
	{
		return;
	}

	CauldronIntakeRelocationRemainingTime = FMath::Max(CauldronIntakeRelocationRemainingTime - FMath::Max(DeltaSeconds, 0.0f), 0.0f);
	if (CauldronIntakeRelocationRemainingTime > 0.0f)
	{
		return;
	}

	const EWizardCauldronIntakeDirection NewActiveIntake = PreviewCauldronIntake != EWizardCauldronIntakeDirection::None
		? PreviewCauldronIntake
		: ChooseNextCauldronIntake(EWizardCauldronIntakeDirection::None);
	ActiveCauldronIntake = NewActiveIntake;
	PreviewCauldronIntake = EWizardCauldronIntakeDirection::None;
	bCauldronIntakeRelocating = false;
	ApplyCauldronIntakeReadability();
}

void AWizardStaffGameMode::ApplyCauldronIntakeReadability()
{
	if (!IsValid(ActiveCauldronArena))
	{
		return;
	}

	ActiveCauldronArena->ConfigureIntakePresentation(
		CauldronCatastropheTuning.ActiveIntakeDistanceFromCauldronCenter,
		CauldronCatastropheTuning.ActiveIntakeVisualScale,
		CauldronCatastropheTuning.ActiveIntakePulseSpeed);
	ActiveCauldronArena->SetIntakeState(ActiveCauldronIntake, PreviewCauldronIntake, bCauldronIntakeRelocating);
}

EWizardCauldronIntakeDirection AWizardStaffGameMode::ChooseNextCauldronIntake(EWizardCauldronIntakeDirection ExcludedIntake) const
{
	static const EWizardCauldronIntakeDirection IntakeDirections[] = {
		EWizardCauldronIntakeDirection::North,
		EWizardCauldronIntakeDirection::East,
		EWizardCauldronIntakeDirection::South,
		EWizardCauldronIntakeDirection::West
	};

	TArray<EWizardCauldronIntakeDirection, TInlineAllocator<4>> Candidates;
	for (const EWizardCauldronIntakeDirection IntakeDirection : IntakeDirections)
	{
		if (IntakeDirection != ExcludedIntake)
		{
			Candidates.Add(IntakeDirection);
		}
	}
	return Candidates.IsEmpty() ? EWizardCauldronIntakeDirection::None : Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

EWizardCauldronIntakeDirection AWizardStaffGameMode::ParseCauldronIntakeDirection(const FString& IntakeDirection) const
{
	if (IntakeDirection.Equals(TEXT("North"), ESearchCase::IgnoreCase))
	{
		return EWizardCauldronIntakeDirection::North;
	}
	if (IntakeDirection.Equals(TEXT("East"), ESearchCase::IgnoreCase))
	{
		return EWizardCauldronIntakeDirection::East;
	}
	if (IntakeDirection.Equals(TEXT("South"), ESearchCase::IgnoreCase))
	{
		return EWizardCauldronIntakeDirection::South;
	}
	if (IntakeDirection.Equals(TEXT("West"), ESearchCase::IgnoreCase))
	{
		return EWizardCauldronIntakeDirection::West;
	}

	UE_LOG(LogTemp, Warning, TEXT("Unknown Cauldron intake '%s'. Use North, East, South, or West."), *IntakeDirection);
	return EWizardCauldronIntakeDirection::None;
}

void AWizardStaffGameMode::RefreshCauldronVialEffects(AWizardStaffWizardCharacter* Wizard)
{
	if (!IsValid(Wizard))
	{
		return;
	}
	const TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(Wizard);
	const int32 Count = Stack ? Stack->Num() : 0;
	const EWizardCauldronVialType ActiveType = Count > 0 ? Stack->Last().Type : EWizardCauldronVialType::None;
	float SpeedMultiplier = 1.0f;
	float AccelerationMultiplier = 1.0f;
	float BonkMultiplier = 1.0f;
	if (ActiveType == EWizardCauldronVialType::Speed)
	{
		SpeedMultiplier = CauldronCatastropheTuning.SpeedVialMovementMultiplier;
		AccelerationMultiplier = CauldronCatastropheTuning.SpeedVialAccelerationMultiplier;
	}
	else if (ActiveType == EWizardCauldronVialType::BurdeningPower)
	{
		SpeedMultiplier = CauldronCatastropheTuning.BurdeningPowerMovementMultiplier;
		BonkMultiplier = CauldronCatastropheTuning.BurdeningPowerBonkMultiplier;
	}
	Wizard->SetCauldronVialEffectState(ActiveType, Count, SpeedMultiplier, AccelerationMultiplier, BonkMultiplier, GetCauldronVialInstabilityMultiplier(Wizard));
}

float AWizardStaffGameMode::GetCauldronVialInstabilityMultiplier(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!bCauldronCatastropheActive || !CauldronCatastropheTuning.bEnableVialInstability || !IsValid(Wizard))
	{
		return 1.0f;
	}

	const TArray<FCauldronVialStackEntry>* Stack = CauldronVialStacks.Find(Wizard);
	const int32 CarriedVialCount = Stack ? Stack->Num() : 0;
	const int32 ExtraVials = FMath::Max(CarriedVialCount - FMath::Max(CauldronCatastropheTuning.VialInstabilitySafeCount, 0), 0);
	const float MaximumMultiplier = FMath::Max(CauldronCatastropheTuning.VialInstabilityMaximumMultiplier, 1.0f);
	return FMath::Clamp(1.0f + (static_cast<float>(ExtraVials) * FMath::Max(CauldronCatastropheTuning.VialInstabilityStressPerExtraVial, 0.0f)), 1.0f, MaximumMultiplier);
}

bool AWizardStaffGameMode::ApplyCauldronVialInstabilityForEnemyBonk(AWizardStaffWizardCharacter* Attacker, AWizardStaffWizardCharacter* Target)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || !IsValid(Attacker) || !IsValid(Target) || Attacker == Target
		|| GetPlayerIndexForWizard(Attacker) == INDEX_NONE || GetPlayerIndexForWizard(Target) == INDEX_NONE)
	{
		return false;
	}

	const float Multiplier = GetCauldronVialInstabilityMultiplier(Target);
	if (Multiplier <= 1.0f)
	{
		return false;
	}

	int32 BonkSequence = Attacker->GetAuthoritativeQuickBonkSequence();
	if (BonkSequence <= 0)
	{
		// A Staff Clash winner may not be the wizard that started the original swing.
		BonkSequence = NextCauldronInstabilityFallbackSequence--;
	}

	const int64 BonkKey = (static_cast<int64>(Attacker->GetUniqueID()) << 32) | static_cast<uint32>(BonkSequence);
	const TWeakObjectPtr<AWizardStaffWizardCharacter> TargetKey(Target);
	if (const int64* LastBonkKey = LastCauldronInstabilityBonkKeys.Find(TargetKey); LastBonkKey && *LastBonkKey == BonkKey)
	{
		return false;
	}

	float BaseStress = 0.0f;
	float FinalStress = 0.0f;
	if (!Target->ApplyCauldronVialInstabilityStress(Multiplier, BaseStress, FinalStress))
	{
		return false;
	}

	LastCauldronInstabilityBonkKeys.Add(TargetKey, BonkKey);
	UE_LOG(LogTemp, Log, TEXT("Cauldron instability P%d -> P%d: base %.2f x %.2f = final %.2f (bonk %d)."),
		GetPlayerIndexForWizard(Attacker) + 1, GetPlayerIndexForWizard(Target) + 1, BaseStress, Multiplier, FinalStress, BonkSequence);
	return true;
}

void AWizardStaffGameMode::ClearCauldronVials(bool bRemoveSegments)
{
	for (AWizardStaffCauldronVialPickup* Vial : SpawnedCauldronVials)
	{
		if (IsValid(Vial))
		{
			Vial->Destroy();
		}
	}
	SpawnedCauldronVials.Reset();
	PendingCauldronVialRespawns.Reset();
	LastCauldronDepositBonkSequences.Reset();
	LastCauldronInstabilityBonkKeys.Reset();
	NextCauldronInstabilityFallbackSequence = -1;
	NextDebugCauldronDepositSequence = -1;

	for (TPair<TWeakObjectPtr<AWizardStaffWizardCharacter>, TArray<FCauldronVialStackEntry>>& Pair : CauldronVialStacks)
	{
		AWizardStaffWizardCharacter* Wizard = Pair.Key.Get();
		if (bRemoveSegments && IsValid(Wizard) && Wizard->StaffComponent)
		{
			for (int32 EntryIndex = Pair.Value.Num() - 1; EntryIndex >= 0; --EntryIndex)
			{
				if (Wizard->StaffComponent->GetTopStaffSegmentTag() != Pair.Value[EntryIndex].SegmentTag)
				{
					UE_LOG(LogTemp, Warning, TEXT("Cauldron vial cleanup left a non-top tagged segment intact."));
					break;
				}
				Wizard->StaffComponent->RemoveTopStaffSegment(false);
			}
			Wizard->SyncReplicatedStaffSegmentCountFromAuthority();
		}
		if (IsValid(Wizard))
		{
			Wizard->SetCauldronVialEffectState(EWizardCauldronVialType::None, 0, 1.0f, 1.0f, 1.0f);
		}
	}
	CauldronVialStacks.Reset();
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (IsValid(Wizard))
		{
			Wizard->SetCauldronVialEffectState(EWizardCauldronVialType::None, 0, 1.0f, 1.0f, 1.0f);
		}
	}
	NextCauldronVialPickupOrder = 1;
}

void AWizardStaffGameMode::SpawnCauldronHazard(EWizardCauldronHazardType HazardType)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || SpawnedCauldronHazards.Num() >= FMath::Max(CauldronCatastropheTuning.MaximumActiveHazards, 1))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Angle = FMath::FRandRange(0.0f, 2.0f * UE_PI);
	const float MaximumRadius = FMath::Min(
		FMath::Max(CauldronCatastropheTuning.IngredientSpawnRadius - 120.0f, 420.0f),
		FMath::Max(CauldronCatastropheTuning.ArenaHalfSize - 240.0f, 420.0f));
	const float Radius = FMath::FRandRange(360.0f, MaximumRadius);
	const FVector SpawnLocation = GetArenaCenter() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Radius + FVector(0.0f, 0.0f, -4.0f);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWizardStaffCauldronHazard* Hazard = World->SpawnActor<AWizardStaffCauldronHazard>(AWizardStaffCauldronHazard::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
	if (Hazard)
	{
		Hazard->InitializeHazard(HazardType, CauldronCatastropheTuning.HazardRadius, IsValid(ActiveCauldronArena) ? ActiveCauldronArena->GetAcceptanceCenter() : GetArenaCenter());
		SpawnedCauldronHazards.Add(Hazard);
		CauldronHazardExpirationTimes.Add(TWeakObjectPtr<AWizardStaffCauldronHazard>(Hazard), World->GetTimeSeconds() + FMath::Max(CauldronCatastropheTuning.HazardLifetime, 1.0f));
	}
}

void AWizardStaffGameMode::UpdateCauldronHazardEffects()
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		float FrictionMultiplier = 1.0f;
		float BrakingMultiplier = 1.0f;
		float SpeedMultiplier = 1.0f;
		float AccelerationMultiplier = 1.0f;
		float TurningMultiplier = 1.0f;
		bool bOnSlipperyHazard = false;
		AWizardStaffCauldronHazard* StickyHazard = nullptr;
		for (AWizardStaffCauldronHazard* Hazard : SpawnedCauldronHazards)
		{
			if (!IsValid(Hazard) || FVector::DistSquared2D(Wizard->GetActorLocation(), Hazard->GetActorLocation()) > FMath::Square(Hazard->GetHazardRadius()))
			{
				continue;
			}
			if (Hazard->GetHazardType() == EWizardCauldronHazardType::Slippery)
			{
				FrictionMultiplier = FMath::Min(FrictionMultiplier, CauldronCatastropheTuning.SlipperyFrictionMultiplier);
				BrakingMultiplier = FMath::Min(BrakingMultiplier, CauldronCatastropheTuning.SlipperyBrakingMultiplier);
				bOnSlipperyHazard = true;
			}
			else
			{
				SpeedMultiplier = FMath::Min(SpeedMultiplier, CauldronCatastropheTuning.StickyMovementMultiplier);
				AccelerationMultiplier = FMath::Min(AccelerationMultiplier, CauldronCatastropheTuning.StickyAccelerationMultiplier);
				TurningMultiplier = FMath::Min(TurningMultiplier, CauldronCatastropheTuning.StickyTurningMultiplier);
				StickyHazard = Hazard;
			}
		}
		const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;
		if (bOnSlipperyHazard)
		{
			Wizard->TriggerCauldronSlipperySkid(
				Wizard->GetManaSloshAlpha(),
				CauldronCatastropheTuning.SlipperySkidDuration,
				CauldronCatastropheTuning.SlipperySkidMinimumImpulse,
				CauldronCatastropheTuning.SlipperySkidMaximumImpulse);
		}

		const bool bSlipperySkidding = Wizard->UpdateCauldronSlipperySkid(DeltaSeconds);
		if (bSlipperySkidding)
		{
			FrictionMultiplier = FMath::Min(FrictionMultiplier, CauldronCatastropheTuning.SlipperyFrictionMultiplier);
			BrakingMultiplier = FMath::Min(BrakingMultiplier, CauldronCatastropheTuning.SlipperyBrakingMultiplier);
			Wizard->ApplyCauldronSlipperyGlide(DeltaSeconds, CauldronCatastropheTuning.SlipperySkidAcceleration);
		}
		Wizard->SetCauldronHazardMovementMultipliers(FrictionMultiplier, BrakingMultiplier, SpeedMultiplier, AccelerationMultiplier, TurningMultiplier);

		if (StickyHazard && !Wizard->IsBroomBoostActive())
		{
			Wizard->SetCauldronStickyTetherState(true, StickyHazard->GetActorLocation());
		}
		else if (Wizard->IsCauldronStickyTethered())
		{
			bool bAnchorStillExists = false;
			for (AWizardStaffCauldronHazard* Hazard : SpawnedCauldronHazards)
			{
				if (IsValid(Hazard) && Hazard->GetHazardType() == EWizardCauldronHazardType::Sticky
					&& FVector::DistSquared2D(Hazard->GetActorLocation(), Wizard->GetCauldronStickyTetherAnchor()) <= 1.0f)
				{
					bAnchorStillExists = true;
					break;
				}
			}
			if (!bAnchorStillExists)
			{
				Wizard->SetCauldronStickyTetherState(false);
			}
		}

		if (Wizard->IsCauldronStickyTethered() && !Wizard->IsBroomBoostActive())
		{
			Wizard->ApplyCauldronStickyTetherReel(GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f);
		}
	}
}

void AWizardStaffGameMode::AssignCauldronCurse(int32 ForcedPlayerIndex)
{
	if (!HasAuthority() || !bCauldronCatastropheActive)
	{
		return;
	}

	TArray<int32> Candidates;
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (Wizard && PlayerIndex != INDEX_NONE && !Wizard->IsReadableOutOfArenaRespawning()
			&& (CauldronCatastropheTuning.bAllowSameWizardConsecutiveCurse || PlayerIndex != LastCauldronCursedPlayerIndex))
		{
			Candidates.Add(PlayerIndex);
		}
	}
	if (ForcedPlayerIndex != INDEX_NONE && IsValid(GetWizardForPlayerIndex(ForcedPlayerIndex)))
	{
		Candidates.Reset();
		Candidates.Add(ForcedPlayerIndex);
	}
	if (Candidates.Num() == 0)
	{
		CauldronNextCurseTime = 1.0f;
		return;
	}

	ClearCauldronCurse(false);
	CauldronCursedPlayerIndex = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
	LastCauldronCursedPlayerIndex = CauldronCursedPlayerIndex;
	CauldronCurseRemainingTime = FMath::Max(CauldronCatastropheTuning.CurseDuration, 0.1f);
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			Wizard->SetCauldronCurseVisualRelativeLocation(CauldronCatastropheTuning.CurseOrbStaffRelativeLocation);
			Wizard->SetCauldronCurseState(GetPlayerIndexForWizard(Wizard) == CauldronCursedPlayerIndex, CauldronCurseRemainingTime);
		}
	}
	PublishReplicatedGameplayEvent(EWizardReplicatedGameplayEventType::CauldronCurse, FString::Printf(TEXT("P%d is cursed: pass it!"), CauldronCursedPlayerIndex + 1), CauldronCursedPlayerIndex, INDEX_NONE, CauldronCurseRemainingTime, false);
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::ClearCauldronCurse(bool bScheduleNext)
{
	CauldronCursedPlayerIndex = INDEX_NONE;
	CauldronCurseRemainingTime = 0.0f;
	bCauldronCurseGrounded = false;
	CauldronGroundCurseSegment.Reset();
	CauldronGroundCurseLastLocation = FVector::ZeroVector;
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			Wizard->ClearCauldronCurseOrbLooseSegmentAttachment();
			Wizard->SetCauldronCurseState(false, 0.0f);
		}
	}
	CauldronNextCurseTime = bScheduleNext ? FMath::Max(CauldronCatastropheTuning.CurseCadence, 1.0f) : 0.0f;
}

void AWizardStaffGameMode::ExplodeCauldronCurse()
{
	const int32 ExplodingPlayerIndex = CauldronCursedPlayerIndex;
	AWizardStaffWizardCharacter* Holder = GetWizardForPlayerIndex(ExplodingPlayerIndex);
	if (!HasAuthority() || !IsValid(Holder))
	{
		ClearCauldronCurse(true);
		return;
	}

	static const FVector CardinalDirections[] = {
		FVector::ForwardVector,
		-FVector::ForwardVector,
		FVector::RightVector,
		-FVector::RightVector
	};
	const FVector KnockbackDirection = CardinalDirections[FMath::RandRange(0, UE_ARRAY_COUNT(CardinalDirections) - 1)];
	Holder->CancelStaffClashStateForReset();
	Holder->ApplyBonkReaction(KnockbackDirection, CauldronCatastropheTuning.ExplosionHorizontalKnockback, CauldronCatastropheTuning.ExplosionVerticalKnockback, 0);
	PublishReplicatedGameplayEvent(EWizardReplicatedGameplayEventType::CauldronCurse, FString::Printf(TEXT("P%d's curse exploded"), ExplodingPlayerIndex + 1), ExplodingPlayerIndex, INDEX_NONE, 0.0f, false);
	ClearCauldronCurse(true);
}

void AWizardStaffGameMode::ExplodeGroundedCauldronCurse()
{
	if (!HasAuthority())
	{
		return;
	}

	const FVector ExplosionLocation = CauldronGroundCurseLastLocation;
	UWorld* World = GetWorld();
	if (!World)
	{
		ClearCauldronCurse(true);
		return;
	}

	const float Radius = FMath::Max(CauldronCatastropheTuning.GroundCurseExplosionRadius, 0.0f);
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	World->OverlapMultiByObjectType(Overlaps, ExplosionLocation, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Radius));
	int32 AffectedWizardCount = 0;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(Overlap.GetActor());
		if (!IsValid(Wizard))
		{
			continue;
		}

		FVector KnockbackDirection = Wizard->GetActorLocation() - ExplosionLocation;
		KnockbackDirection.Z = 0.0f;
		if (!KnockbackDirection.Normalize())
		{
			KnockbackDirection = FVector::ForwardVector;
		}
		Wizard->ApplyBonkReaction(KnockbackDirection, CauldronCatastropheTuning.GroundCurseExplosionKnockback, CauldronCatastropheTuning.GroundCurseExplosionUpwardBoost, 0);
		++AffectedWizardCount;
	}

	PublishReplicatedGameplayEvent(EWizardReplicatedGameplayEventType::CauldronCurse, FString::Printf(TEXT("Cursed segment exploded near %d wizard%s"), AffectedWizardCount, AffectedWizardCount == 1 ? TEXT("") : TEXT("s")), INDEX_NONE, INDEX_NONE, static_cast<float>(AffectedWizardCount), false);
	ClearCauldronCurse(true);
}

void AWizardStaffGameMode::SetPrototypeArenaPhasePresentationActive(bool bActive)
{
	if (!ActivePrototypeArena)
	{
		return;
	}

	ActivePrototypeArena->SetPhasePresentationActive(bActive);
}

void AWizardStaffGameMode::AddCauldronScore(int32 PlayerIndex, int32 Amount, const FString& Reason)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || PlayerIndex == INDEX_NONE || Amount <= 0)
	{
		return;
	}
	EnsureCauldronScoreSize(PlayerIndex + 1);
	if (!CauldronScores.IsValidIndex(PlayerIndex))
	{
		return;
	}
	CauldronScores[PlayerIndex] += Amount;
	PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType::CauldronIngredientDeposited,
		FString::Printf(TEXT("P%d deposited %d vial%s"), PlayerIndex + 1, Amount, Amount == 1 ? TEXT("") : TEXT("s")),
		PlayerIndex,
		INDEX_NONE,
		static_cast<float>(CauldronScores[PlayerIndex]),
		false);
	UE_LOG(LogTemp, Log, TEXT("Cauldron Catastrophe: P%d +%d (%s), score %d."), PlayerIndex + 1, Amount, *Reason, CauldronScores[PlayerIndex]);
	SyncAllReplicatedPlayerStates();
}

void AWizardStaffGameMode::GrantCauldronStaffSegments(int32 PlayerIndex, int32 SegmentCount, const FString& Reason)
{
	if (!HasAuthority() || !bCauldronCatastropheActive || PlayerIndex == INDEX_NONE || SegmentCount <= 0)
	{
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		return;
	}

	int32 GrantedSegments = 0;
	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const int32 PreviousCount = Wizard->StaffComponent->GetSegmentCount();
		if (Wizard->StaffComponent->AddStaffSegment() <= PreviousCount)
		{
			break;
		}
		++GrantedSegments;
	}

	if (GrantedSegments > 0)
	{
		Wizard->AddManaSloshForStaffGrowth(GrantedSegments, FName(*Reason));
		SyncReplicatedPlayerStateForIndex(PlayerIndex);
	}
}

void AWizardStaffGameMode::AnnounceCauldronWinner()
{
	TArray<int32> Winners;
	int32 BestScore = MIN_int32;
	EnsureCauldronScoreSize();
	for (int32 PlayerIndex = 0; PlayerIndex < CauldronScores.Num(); ++PlayerIndex)
	{
		if (!IsValid(GetWizardForPlayerIndex(PlayerIndex)))
		{
			continue;
		}
		const int32 Score = CauldronScores[PlayerIndex];
		if (Score > BestScore)
		{
			BestScore = Score;
			Winners.Reset();
			Winners.Add(PlayerIndex);
		}
		else if (Score == BestScore)
		{
			Winners.Add(PlayerIndex);
		}
	}

	if (Winners.Num() == 0)
	{
		MugRunWinnerMessage = TEXT("Cauldron Catastrophe ended: no winner.");
	}
	else if (Winners.Num() == 1)
	{
		MugRunWinnerMessage = FString::Printf(TEXT("Cauldron Catastrophe Winner: P%d with %d banked vials!"), Winners[0] + 1, BestScore);
	}
	else
	{
		MugRunWinnerMessage = FString::Printf(TEXT("Cauldron Catastrophe Tie at %d:"), BestScore);
		for (const int32 PlayerIndex : Winners)
		{
			MugRunWinnerMessage += FString::Printf(TEXT(" P%d"), PlayerIndex + 1);
		}
	}

	for (const int32 PlayerIndex : Winners)
	{
		EnsurePlayerRoundWinsSize(PlayerIndex + 1);
		if (PlayerRoundWins.IsValidIndex(PlayerIndex))
		{
			++PlayerRoundWins[PlayerIndex];
		}
		const bool bTie = Winners.Num() > 1;
		AddGrandWizardFavor(PlayerIndex, bTie ? GrandWizardFavorTuning.TiedTrialWinnerFavor : GrandWizardFavorTuning.CauldronCatastropheWinnerFavor, bTie ? TEXT("Cauldron Catastrophe Tie") : TEXT("Cauldron Catastrophe Winner"));
	}
	SyncPlaytestTelemetryRoundWins();
	SyncAllReplicatedPlayerStates();
	AWizardStaffHUD::PushGameplayMessage(this, MugRunWinnerMessage, FColor::Yellow, 4.0f, EWizardHudMessageCategory::Scoring);
}

void AWizardStaffGameMode::EnsureCauldronScoreSize(int32 MinimumPlayerCount)
{
	int32 DesiredSize = FMath::Max(MinimumPlayerCount, GetDesiredLocalPlayerCountForSession());
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != INDEX_NONE)
		{
			DesiredSize = FMath::Max(DesiredSize, PlayerIndex + 1);
		}
	}
	const int32 OldSize = CauldronScores.Num();
	CauldronScores.SetNum(FMath::Max(DesiredSize, OldSize));
	for (int32 PlayerIndex = OldSize; PlayerIndex < CauldronScores.Num(); ++PlayerIndex)
	{
		CauldronScores[PlayerIndex] = 0;
	}
}

void AWizardStaffGameMode::RestartMugRunMatch()
{
	if (!MugRunTuning.bEnableMugRun)
	{
		return;
	}

	StartPartyMatch();
}

void AWizardStaffGameMode::FinishActiveTrialResults()
{
	const bool bFinishedCauldronCatastrophe = ActiveTrialType == EWizardTrialType::CauldronCatastrophe;
	TrialResultsRemainingTime = 0.0f;
	MugRunPostMatchRemainingTime = 0.0f;
	IntermissionRemainingTime = 0.0f;
	ActiveTrialState = EWizardTrialState::Finished;
	MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;
	bStaffsAtDawnTrialActive = false;
	++CompletedTrialCount;
	if (bFinishedCauldronCatastrophe)
	{
		CauldronScores.Reset();
		SyncAllReplicatedPlayerStates();
	}

	EnterPartyHallIntermission();
}

void AWizardStaffGameMode::EnterGrandWizardFinalRound()
{
	// The Final reuses the prototype arena hidden during Party Hall. Restore its floor first,
	// then retire the retained Cauldron arena before spawning Final presentation and players.
	SetPrototypeArenaPhasePresentationActive(true);
	CleanupCauldronCatastrophe();
	CauldronScores.Reset();
	PartyMatchState = EWizardPartyMatchState::FinalRound;
	ActiveTrialState = EWizardTrialState::Active;
	bWizardsStagedForActiveTrial = false;
	SetWizardPrototypeInputsLocked(false);
	ClearReplicatedGameplayEventFeed();
	IntermissionRemainingTime = 0.0f;
	TrialCountdownRemainingTime = 0.0f;
	TrialResultsRemainingTime = 0.0f;
	MugRunRemainingTime = 0.0f;
	MugRunPostMatchRemainingTime = 0.0f;
	bMugRunMatchActive = false;
	MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;
	SetMugRunPickupsActive(false);
	CleanupArcanePinballProjectiles();
	ClearPendingOutOfArenaRespawns();

	GrandWizardWinnerPlayerIndex = INDEX_NONE;
	GrandWizardWinnerMessage.Reset();
	MugRunWinnerMessage.Reset();
	GrandWizardCandidatePlayerIndex = SelectInitialGrandWizardCandidate();
	GrandWizardCandidateIntroMessage = BuildGrandWizardCandidateIntroMessage(GrandWizardCandidatePlayerIndex);
	ApplyFavorBasedFinalStaffSetup();
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			Wizard->SetManaSloshLocked(FinalRoundTuning.bLockManaSloshAtFinalStart);
		}
	}
	GrandWizardStealPlayerIndex = INDEX_NONE;
	GrandWizardStealHoldTime = 0.0f;
	GrandWizardStartBonusRemainingTime = FinalRoundTuning.bLeaderStartsWithBonus ? FMath::Max(FinalRoundTuning.LeaderStartBonusDuration, 0.0f) : 0.0f;
	FinalRoundRemainingTime = FMath::Max(FinalRoundTuning.FinalRoundDuration, 0.0f);
	const FString CandidateStaffText = GrandWizardCandidatePlayerIndex == INDEX_NONE
		? FString()
		: FString::Printf(TEXT("P%d Final Staff: %d max Candidate segments"), GrandWizardCandidatePlayerIndex + 1, CalculateFinalStartingSegments(GrandWizardCandidatePlayerIndex, true));
	SetGrandWizardFinalFeedbackMessage(CandidateStaffText.IsEmpty() ? GrandWizardCandidateIntroMessage : FString::Printf(TEXT("%s  |  %s"), *GrandWizardCandidateIntroMessage, *CandidateStaffText), FColor::Yellow);
	if (GrandWizardCandidatePlayerIndex != INDEX_NONE)
	{
		PublishReplicatedGameplayEvent(
			EWizardReplicatedGameplayEventType::GrandWizardCandidateChanged,
			FString::Printf(TEXT("P%d starts as Candidate"), GrandWizardCandidatePlayerIndex + 1),
			GrandWizardCandidatePlayerIndex,
			INDEX_NONE,
			0.0f,
			false);
	}

	SpawnOrUpdateFinalRoundCircleVisual();
	SetFinalRoundCircleVisible(true);
	RespawnWizardsForFinalRound();
	UpdateLeaderHighlights();

	UE_LOG(LogTemp, Log, TEXT("Grand Wizard Final started. %s Duration %.0fs."), *GrandWizardCandidateIntroMessage, FinalRoundRemainingTime);
	if (GEngine && FinalRoundTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4610, 5.0f, FColor::Yellow, GrandWizardCandidateIntroMessage);
	}

	if (FinalRoundRemainingTime <= 0.0f)
	{
		FinishGrandWizardFinalRound();
	}
}

void AWizardStaffGameMode::UpdateGrandWizardFinalRound(float DeltaSeconds)
{
	if (PartyMatchState != EWizardPartyMatchState::FinalRound || ActiveTrialState != EWizardTrialState::Active)
	{
		return;
	}

	AddGrandWizardCandidateTime(FMath::Min(DeltaSeconds, FinalRoundRemainingTime));
	FinalRoundRemainingTime = FMath::Max(0.0f, FinalRoundRemainingTime - DeltaSeconds);
	GrandWizardStartBonusRemainingTime = FMath::Max(0.0f, GrandWizardStartBonusRemainingTime - DeltaSeconds);
	SpawnOrUpdateFinalRoundCircleVisual();

	const float ControlRadius = FMath::Max(FinalRoundTuning.CircleRadius + FinalRoundTuning.CandidateNearCirclePadding, FinalRoundTuning.CircleRadius);
	const AWizardStaffWizardCharacter* CandidateWizard = GetWizardForPlayerIndex(GrandWizardCandidatePlayerIndex);
	const bool bCandidateHasControl = CandidateWizard && IsWizardInsideFinalRoundCircle(CandidateWizard, ControlRadius);
	const int32 ChallengerPlayerIndex = GetBestGrandWizardCircleChallenger();
	bool bCanSteal = ChallengerPlayerIndex != INDEX_NONE && ChallengerPlayerIndex != GrandWizardCandidatePlayerIndex;
	const AWizardStaffWizardCharacter* ChallengerWizard = GetWizardForPlayerIndex(ChallengerPlayerIndex);

	if (FinalRoundTuning.bRequireCandidateOutsideCircleToSteal && bCandidateHasControl)
	{
		bCanSteal = false;
	}

	if (FinalRoundTuning.bLeaderStartsWithBonus && GrandWizardStartBonusRemainingTime > 0.0f && bCandidateHasControl)
	{
		bCanSteal = false;
	}

	if (!IsWizardEligibleForGrandWizardSteal(ChallengerWizard))
	{
		bCanSteal = false;
	}

	if (bCanSteal)
	{
		if (GrandWizardStealPlayerIndex != ChallengerPlayerIndex)
		{
			GrandWizardStealPlayerIndex = ChallengerPlayerIndex;
			GrandWizardStealHoldTime = 0.0f;
		}

		GrandWizardStealHoldTime += DeltaSeconds;
		if (GrandWizardStealHoldTime >= FMath::Max(FinalRoundTuning.CandidateSwapHoldDuration, 0.0f))
		{
			SetGrandWizardCandidate(ChallengerPlayerIndex, TEXT("CircleSteal"));
		}
	}
	else
	{
		GrandWizardStealPlayerIndex = INDEX_NONE;
		GrandWizardStealHoldTime = 0.0f;
	}

	if (FinalRoundRemainingTime <= 0.0f)
	{
		FinishGrandWizardFinalRound();
	}
}

void AWizardStaffGameMode::FinishGrandWizardFinalRound()
{
	if (PartyMatchState != EWizardPartyMatchState::FinalRound)
	{
		return;
	}

	FinalRoundRemainingTime = 0.0f;
	ActiveTrialState = EWizardTrialState::Results;
	TrialResultsRemainingTime = FMath::Max(RematchTuning.PostMatchDuration, 0.0f);
	GrandWizardWinnerPlayerIndex = GrandWizardCandidatePlayerIndex;
	if (GrandWizardWinnerPlayerIndex == INDEX_NONE)
	{
		GrandWizardWinnerPlayerIndex = SelectInitialGrandWizardCandidate();
	}

	if (GrandWizardWinnerPlayerIndex == INDEX_NONE)
	{
		GrandWizardWinnerMessage = TEXT("Grand Wizard Final ended: no winner.");
	}
	else
	{
		GrandWizardWinnerMessage = FString::Printf(TEXT("Grand Wizard: P%d wins the match!"), GrandWizardWinnerPlayerIndex + 1);
	}

	MugRunWinnerMessage = GrandWizardWinnerMessage;
	GrandWizardStealPlayerIndex = INDEX_NONE;
	GrandWizardStealHoldTime = 0.0f;
	GrandWizardStartBonusRemainingTime = 0.0f;
	CaptureFinalPlaytestTelemetrySnapshot();
	DispatchAuthoritativeSteamMatchResults();
	UpdateLeaderHighlights();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *GrandWizardWinnerMessage);
	AWizardStaffHUD::PushGameplayMessage(this, GrandWizardWinnerMessage, FColor::Yellow, 5.0f, EWizardHudMessageCategory::Scoring);
	PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType::FinalWinner,
		GrandWizardWinnerMessage,
		GrandWizardWinnerPlayerIndex,
		INDEX_NONE,
		0.0f,
		false);
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4611, 10.0f, FColor::Yellow, GrandWizardWinnerMessage);
	}
	if (bShowPlaytestTelemetryDebug)
	{
		LogPlaytestTelemetrySummary();
	}
}

void AWizardStaffGameMode::DispatchAuthoritativeSteamMatchResults()
{
	if (!HasAuthority() || PrototypeSessionMode != EWizardPrototypeSessionMode::OnlineListenServer)
	{
		return;
	}

	AWizardStaffGameState* WizardGameState = GetGameState<AWizardStaffGameState>();
	if (!WizardGameState)
	{
		return;
	}

	const int32 MatchGeneration = WizardGameState->GetReplicatedMatchSessionGeneration();
	if (MatchGeneration <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WizardStaff Steam result dispatch skipped: invalid match generation %d."), MatchGeneration);
		return;
	}

	int32 DispatchedPlayerCount = 0;
	for (APlayerState* BasePlayerState : WizardGameState->PlayerArray)
	{
		AWizardStaffPlayerState* WizardPlayerState = Cast<AWizardStaffPlayerState>(BasePlayerState);
		if (!WizardPlayerState || WizardPlayerState->IsPlaytestBotSlot())
		{
			continue;
		}

		const int32 PlayerSlot = WizardPlayerState->GetWizardDisplaySlot();
		if (PlayerSlot < 0)
		{
			continue;
		}

		WizardPlayerState->SendAuthoritativeSteamMatchResultToOwner(
			MatchGeneration,
			PlayerSlot,
			GrandWizardWinnerPlayerIndex,
			GetPlayerGrandWizardFavor(PlayerSlot),
			GetPlayerRoundWins(PlayerSlot));
		++DispatchedPlayerCount;
	}

	UE_LOG(LogTemp, Log, TEXT("WizardStaff dispatched authoritative Steam match result generation %d to %d online player owner(s)."),
		MatchGeneration,
		DispatchedPlayerCount);
}

void AWizardStaffGameMode::ResetGrandWizardFinalRoundState()
{
	FinalRoundRemainingTime = 0.0f;
	GrandWizardCandidatePlayerIndex = INDEX_NONE;
	GrandWizardWinnerPlayerIndex = INDEX_NONE;
	GrandWizardWinnerMessage.Reset();
	GrandWizardCandidateIntroMessage.Reset();
	GrandWizardFinalStaffSetupMessage.Reset();
	GrandWizardFinalFeedbackMessage.Reset();
	GrandWizardFinalFeedbackExpireTime = 0.0f;
	GrandWizardStealPlayerIndex = INDEX_NONE;
	GrandWizardStealHoldTime = 0.0f;
	GrandWizardStartBonusRemainingTime = 0.0f;
	SetFinalRoundCircleVisible(false);
}

void AWizardStaffGameMode::SpawnOrUpdateFinalRoundCircleVisual()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!IsValid(FinalRoundCircleActor))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = MakeUniqueObjectName(World, AWizardStaffFinalRitualCircle::StaticClass(), TEXT("GrandWizardRitualCircle"));
		FinalRoundCircleActor = World->SpawnActor<AWizardStaffFinalRitualCircle>(AWizardStaffFinalRitualCircle::StaticClass(), GetFinalRoundCircleCenter(), FRotator::ZeroRotator, SpawnParameters);
	}

	if (!FinalRoundCircleActor)
	{
		return;
	}

	FLinearColor CircleColor(0.25f, 0.95f, 1.0f, 1.0f);
	if (GrandWizardWinnerPlayerIndex != INDEX_NONE || !GrandWizardWinnerMessage.IsEmpty())
	{
		CircleColor = FLinearColor(1.0f, 0.82f, 0.08f, 1.0f);
	}
	else if (GrandWizardStealPlayerIndex != INDEX_NONE)
	{
		CircleColor = FLinearColor(1.0f, 0.45f, 0.05f, 1.0f);
	}
	else if (IsGrandWizardCandidateVulnerable())
	{
		CircleColor = FLinearColor(1.0f, 0.16f, 0.06f, 1.0f);
	}

	FinalRoundCircleActor->SetCircleState(GetFinalRoundCircleCenter(), FinalRoundTuning.CircleRadius, CircleColor, true);
}

void AWizardStaffGameMode::SetFinalRoundCircleVisible(bool bNewVisible)
{
	if (IsValid(FinalRoundCircleActor))
	{
		FinalRoundCircleActor->SetCircleVisible(bNewVisible);
	}
}

void AWizardStaffGameMode::DrawGrandWizardFinalWorldMarkers() const
{
	if (!FinalRoundTuning.bShowDebug)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || PartyMatchState != EWizardPartyMatchState::FinalRound)
	{
		return;
	}

	const FVector CircleCenter = GetFinalRoundCircleCenter();
	const float CircleRadius = FMath::Max(FinalRoundTuning.CircleRadius, 0.0f);
	const float ControlRadius = FMath::Max(CircleRadius + FinalRoundTuning.CandidateNearCirclePadding, CircleRadius);
	const bool bFinalHasWinner = GrandWizardWinnerPlayerIndex != INDEX_NONE || !GrandWizardWinnerMessage.IsEmpty();
	const bool bCandidateVulnerable = IsGrandWizardCandidateVulnerable();
	const bool bStealActive = GrandWizardStealPlayerIndex != INDEX_NONE;
	const FColor CircleColor = bFinalHasWinner ? FColor::Yellow : (bStealActive ? FColor::Orange : (bCandidateVulnerable ? FColor::Red : FColor::Cyan));
	DrawDebugCylinder(World, CircleCenter, CircleCenter + FVector(0.0f, 0.0f, 10.0f), CircleRadius, 64, CircleColor, false, 0.0f, 0, 5.5f);
	DrawDebugCylinder(World, CircleCenter, CircleCenter + FVector(0.0f, 0.0f, 12.0f), ControlRadius, 64, FColor::Cyan, false, 0.0f, 0, 1.5f);
	DrawDebugString(World, CircleCenter + FVector(0.0f, 0.0f, 72.0f), bFinalHasWinner ? TEXT("FINAL WINNER LOCKED") : (bCandidateVulnerable ? TEXT("RITUAL CIRCLE - VULNERABLE") : TEXT("RITUAL CIRCLE - CANDIDATE SAFE")), nullptr, CircleColor, 0.0f, true);

	const int32 MarkedPlayerIndex = GrandWizardWinnerPlayerIndex != INDEX_NONE ? GrandWizardWinnerPlayerIndex : GrandWizardCandidatePlayerIndex;
	const AWizardStaffWizardCharacter* MarkedWizard = GetWizardForPlayerIndex(MarkedPlayerIndex);
	if (MarkedWizard)
	{
		const FVector MarkerLocation = MarkedWizard->GetActorLocation() + FVector(0.0f, 0.0f, 160.0f);
		const FColor MarkerColor = bFinalHasWinner ? FColor::Yellow : (bCandidateVulnerable ? FColor::Orange : FColor::Cyan);
		DrawDebugSphere(World, MarkerLocation, 44.0f, 20, MarkerColor, false, 0.0f, 0, 5.5f);
		const FString MarkerText = bFinalHasWinner
			? FString::Printf(TEXT("GRAND WIZARD P%d"), MarkedPlayerIndex + 1)
			: FString::Printf(TEXT("CANDIDATE P%d - %s"), MarkedPlayerIndex + 1, bCandidateVulnerable ? TEXT("VULNERABLE") : TEXT("SAFE"));
		DrawDebugString(World, MarkerLocation + FVector(0.0f, 0.0f, 42.0f), MarkerText, nullptr, MarkerColor, 0.0f, true);
	}

	if (ActiveTrialState != EWizardTrialState::Active)
	{
		return;
	}

	if (GrandWizardStealPlayerIndex != INDEX_NONE)
	{
		if (const AWizardStaffWizardCharacter* StealingWizard = GetWizardForPlayerIndex(GrandWizardStealPlayerIndex))
		{
			const FVector StealLocation = StealingWizard->GetActorLocation() + FVector(0.0f, 0.0f, 132.0f);
			DrawDebugSphere(World, StealLocation, 24.0f, 12, FColor::Orange, false, 0.0f, 0, 3.0f);
			DrawDebugString(
				World,
				StealLocation + FVector(0.0f, 0.0f, 30.0f),
				FString::Printf(TEXT("STEALING %.0f%%"), GetGrandWizardStealProgressAlpha() * 100.0f),
				nullptr,
				FColor::Orange,
				0.0f,
				true);
		}
		return;
	}

	if (bCandidateVulnerable)
	{
		DrawDebugString(World, CircleCenter + FVector(0.0f, 0.0f, 110.0f), TEXT("CANDIDATE VULNERABLE - HOLD THE CIRCLE"), nullptr, FColor::Orange, 0.0f, true);
	}
	else if (GetBestGrandWizardCircleChallenger() != INDEX_NONE)
	{
		DrawDebugString(World, CircleCenter + FVector(0.0f, 0.0f, 110.0f), TEXT("CANNOT STEAL - CANDIDATE IS STILL SAFE"), nullptr, FColor::Cyan, 0.0f, true);
	}
}

FVector AWizardStaffGameMode::GetFinalRoundCircleCenter() const
{
	return GetArenaCenter();
}

bool AWizardStaffGameMode::IsWizardInsideFinalRoundCircle(const AWizardStaffWizardCharacter* Wizard, float Radius) const
{
	if (!Wizard)
	{
		return false;
	}

	return FVector::Dist2D(Wizard->GetActorLocation(), GetFinalRoundCircleCenter()) <= FMath::Max(Radius, 0.0f);
}

bool AWizardStaffGameMode::IsWizardEligibleForGrandWizardSteal(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!IsValid(Wizard)
		|| PartyMatchState != EWizardPartyMatchState::FinalRound
		|| ActiveTrialState != EWizardTrialState::Active)
	{
		return false;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (PlayerIndex == INDEX_NONE || PlayerIndex == GrandWizardCandidatePlayerIndex)
	{
		return false;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(const_cast<AWizardStaffWizardCharacter*>(Wizard));
	return !Wizard->IsReadableOutOfArenaRespawning()
		&& !PendingOutOfArenaRespawns.Contains(WizardKey)
		&& !IsWizardOutOfArena(Wizard);
}

int32 AWizardStaffGameMode::SelectInitialGrandWizardCandidate() const
{
	int32 BestPlayerIndex = INDEX_NONE;
	int32 BestFavor = INDEX_NONE;
	int32 BestWins = INDEX_NONE;
	int32 BestSegments = INDEX_NONE;

	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		const int32 Favor = GetPlayerGrandWizardFavor(PlayerIndex);
		const int32 RoundWins = GetPlayerRoundWins(PlayerIndex);
		const int32 SegmentCount = Wizard->GetStaffSegmentCount();
		if (Favor > BestFavor
			|| (Favor == BestFavor && RoundWins > BestWins)
			|| (Favor == BestFavor && RoundWins == BestWins && SegmentCount > BestSegments)
			|| (Favor == BestFavor && RoundWins == BestWins && SegmentCount == BestSegments && (BestPlayerIndex == INDEX_NONE || PlayerIndex < BestPlayerIndex)))
		{
			BestFavor = Favor;
			BestWins = RoundWins;
			BestSegments = SegmentCount;
			BestPlayerIndex = PlayerIndex;
		}
	}

	return BestPlayerIndex;
}

FString AWizardStaffGameMode::BuildGrandWizardCandidateIntroMessage(int32 PlayerIndex) const
{
	if (PlayerIndex == INDEX_NONE)
	{
		return TEXT("Grand Wizard Final: no starting Candidate.");
	}

	const AWizardStaffWizardCharacter* CandidateWizard = GetWizardForPlayerIndex(PlayerIndex);
	const int32 CandidateSegments = CandidateWizard ? CandidateWizard->GetStaffSegmentCount() : 0;
	const int32 CandidateRoundWins = GetPlayerRoundWins(PlayerIndex);
	const int32 CandidateFavor = GetPlayerGrandWizardFavor(PlayerIndex);
	bool bTiedOnFavor = false;
	bool bTiedOnFavorAndWins = false;
	bool bTiedOnFavorWinsAndStaff = false;

	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 OtherPlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (OtherPlayerIndex == INDEX_NONE || OtherPlayerIndex == PlayerIndex)
		{
			continue;
		}

		const int32 OtherFavor = GetPlayerGrandWizardFavor(OtherPlayerIndex);
		const int32 OtherRoundWins = GetPlayerRoundWins(OtherPlayerIndex);
		const int32 OtherSegments = Wizard->GetStaffSegmentCount();
		if (OtherFavor == CandidateFavor)
		{
			bTiedOnFavor = true;
			if (OtherRoundWins == CandidateRoundWins)
			{
				bTiedOnFavorAndWins = true;
				if (OtherSegments == CandidateSegments)
				{
					bTiedOnFavorWinsAndStaff = true;
				}
			}
		}
	}

	if (bTiedOnFavorWinsAndStaff)
	{
		return FString::Printf(
			TEXT("P%d starts as Candidate: tied at %d Favor, %d wins, and %d staff; lowest player number."),
			PlayerIndex + 1,
			CandidateFavor,
			CandidateRoundWins,
			CandidateSegments);
	}

	if (bTiedOnFavorAndWins)
	{
		return FString::Printf(
			TEXT("P%d starts as Candidate: tied at %d Favor and %d wins, led with %d staff."),
			PlayerIndex + 1,
			CandidateFavor,
			CandidateRoundWins,
			CandidateSegments);
	}

	if (bTiedOnFavor)
	{
		return FString::Printf(
			TEXT("P%d starts as Candidate: tied at %d Favor, led with %d round wins."),
			PlayerIndex + 1,
			CandidateFavor,
			CandidateRoundWins);
	}

	return FString::Printf(
		TEXT("P%d starts as Candidate: highest Favor (%d)."),
		PlayerIndex + 1,
		CandidateFavor);
}

int32 AWizardStaffGameMode::CalculateFinalStartingSegments(int32 PlayerIndex, bool bCandidate) const
{
	const int32 Favor = GetPlayerGrandWizardFavor(PlayerIndex);
	const int32 BaseSegments = bCandidate ? FinalRoundTuning.CandidateStartingBaseSegments : FinalRoundTuning.ChallengerStartingBaseSegments;
	const float SegmentsPerFavor = bCandidate ? FinalRoundTuning.CandidateSegmentsPerFavor : FinalRoundTuning.ChallengerSegmentsPerFavor;
	const int32 MaxSegments = bCandidate ? FinalRoundTuning.CandidateMaxStartingSegments : FinalRoundTuning.ChallengerMaxStartingSegments;
	if (bCandidate)
	{
		return FMath::Max(MaxSegments, 0);
	}

	const int32 TargetSegments = FMath::RoundToInt(static_cast<float>(FMath::Max(BaseSegments, 0)) + (static_cast<float>(Favor) * FMath::Max(SegmentsPerFavor, 0.0f)));
	return FMath::Clamp(TargetSegments, 0, FMath::Max(MaxSegments, 0));
}

void AWizardStaffGameMode::ApplyFavorBasedFinalStaffSetup()
{
	GrandWizardFinalStaffSetupMessage.Reset();
	if (!FinalRoundTuning.bUseFavorBasedFinalStaffSetup)
	{
		return;
	}

	FString StaffSetupText = TEXT("Final Staff setup:");
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (!Wizard || PlayerIndex == INDEX_NONE)
		{
			continue;
		}

		Wizard->ResetForNewMatch(true, true);
		const bool bCandidate = PlayerIndex == GrandWizardCandidatePlayerIndex;
		const int32 TargetSegments = CalculateFinalStartingSegments(PlayerIndex, bCandidate);
		SetWizardStaffSegmentCount(Wizard, TargetSegments, true);
		StaffSetupText += FString::Printf(TEXT(" P%d=%d%s"), PlayerIndex + 1, TargetSegments, bCandidate ? TEXT(" Candidate") : TEXT(""));
	}
	GrandWizardFinalStaffSetupMessage = StaffSetupText;
	UE_LOG(LogTemp, Log, TEXT("%s"), *GrandWizardFinalStaffSetupMessage);
}

void AWizardStaffGameMode::SetWizardStaffSegmentCount(AWizardStaffWizardCharacter* Wizard, int32 TargetSegmentCount, bool bApplyManaSloshForSegments) const
{
	if (!Wizard || !Wizard->StaffComponent)
	{
		return;
	}

	const int32 SafeTargetSegmentCount = FMath::Max(TargetSegmentCount, 0);
	Wizard->StaffComponent->ClearStaffSegments();
	Wizard->StaffComponent->RebuildStaffSegmentsForCount(SafeTargetSegmentCount);
	if (bApplyManaSloshForSegments && SafeTargetSegmentCount > 0)
	{
		Wizard->AddManaSloshForStaffGrowth(SafeTargetSegmentCount, TEXT("FinalStaffSetup"));
	}
}

int32 AWizardStaffGameMode::GetBestGrandWizardCircleChallenger() const
{
	int32 BestPlayerIndex = INDEX_NONE;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FVector CircleCenter = GetFinalRoundCircleCenter();
	const float CircleRadius = FMath::Max(FinalRoundTuning.CircleRadius, 0.0f);
	const float CircleRadiusSquared = FMath::Square(CircleRadius);

	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!IsWizardEligibleForGrandWizardSteal(Wizard))
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		const float DistanceSquared = FMath::Square(FVector::Dist2D(Wizard->GetActorLocation(), CircleCenter));
		if (DistanceSquared <= CircleRadiusSquared && DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestPlayerIndex = PlayerIndex;
		}
	}

	return BestPlayerIndex;
}

void AWizardStaffGameMode::SetGrandWizardCandidate(int32 PlayerIndex, const FString& Reason)
{
	if (PlayerIndex == INDEX_NONE || PlayerIndex == GrandWizardCandidatePlayerIndex)
	{
		return;
	}

	GrandWizardCandidatePlayerIndex = PlayerIndex;
	GrandWizardStealPlayerIndex = INDEX_NONE;
	GrandWizardStealHoldTime = 0.0f;
	GrandWizardStartBonusRemainingTime = 0.0f;
	UpdateLeaderHighlights();

	const FString CandidateText = GetGrandWizardCandidateText();
	UE_LOG(LogTemp, Log, TEXT("Grand Wizard Candidate changed to %s (%s)."), *CandidateText, *Reason);
	SetGrandWizardFinalFeedbackMessage(FString::Printf(TEXT("%s stole the Grand Wizard title!"), *CandidateText), FColor::Yellow);
	PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType::GrandWizardCandidateChanged,
		FString::Printf(TEXT("%s stole the Grand Wizard title"), *CandidateText),
		PlayerIndex,
		INDEX_NONE,
		0.0f,
		false);

	if (FinalRoundTuning.bShowDebug)
	{
		if (UWorld* World = GetWorld())
		{
			if (const AWizardStaffWizardCharacter* CandidateWizard = GetWizardForPlayerIndex(PlayerIndex))
			{
				const FVector FeedbackLocation = CandidateWizard->GetActorLocation() + FVector(0.0f, 0.0f, 150.0f);
				DrawDebugSphere(World, FeedbackLocation, 48.0f, 20, FColor::Yellow, false, 1.2f, 0, 5.0f);
				DrawDebugString(World, FeedbackLocation + FVector(0.0f, 0.0f, 42.0f), TEXT("TITLE STOLEN!"), nullptr, FColor::Yellow, 1.2f, true);
			}
		}
	}
}

void AWizardStaffGameMode::SetGrandWizardFinalFeedbackMessage(const FString& Message, const FColor& MessageColor)
{
	GrandWizardFinalFeedbackMessage = Message;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	GrandWizardFinalFeedbackExpireTime = Now + FMath::Max(FinalRoundTuning.FeedbackMessageDuration, 0.0f);
	AWizardStaffHUD::PushGameplayMessage(this, Message, MessageColor, FinalRoundTuning.FeedbackMessageDuration, EWizardHudMessageCategory::Scoring);

	if (GEngine && FinalRoundTuning.bShowDebug && !Message.IsEmpty())
	{
		if (AWizardStaffHUD::IsFullDebugMode(this))
		{
			GEngine->AddOnScreenDebugMessage(4612, FMath::Max(FinalRoundTuning.FeedbackMessageDuration, 0.1f), MessageColor, Message);
		}
	}
}

void AWizardStaffGameMode::RespawnWizardsForFinalRound()
{
	const FVector CircleCenter = GetFinalRoundCircleCenter();
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != GrandWizardCandidatePlayerIndex)
		{
			RespawnWizardInArena(Wizard);
			continue;
		}

		if (UCharacterMovementComponent* MovementComponent = Wizard->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
		Wizard->CancelMovementStateForRespawn();
		Wizard->SyncReplicatedOutOfArenaRespawnStateFromAuthority(false, 0.0f, true);

		Wizard->SetActorLocationAndRotation(
			CircleCenter + FVector(0.0f, 0.0f, 120.0f),
			FRotator::ZeroRotator,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		Wizard->ForceNetUpdate();
	}
}

FString AWizardStaffGameMode::GetMugRunMatchStateText() const
{
	if (ActiveTrialState == EWizardTrialState::Countdown)
	{
		return TEXT("Mug Run Countdown");
	}
	if (ActiveTrialState == EWizardTrialState::Results)
	{
		return TEXT("Mug Run Results");
	}
	if (PartyMatchState == EWizardPartyMatchState::FinalRound)
	{
		return TEXT("Final Round");
	}

	switch (MugRunMatchState)
	{
	case EWizardMugRunMatchState::Playing:
		return TEXT("Mug Run");
	case EWizardMugRunMatchState::PostMatch:
		return TEXT("Post-Match");
	case EWizardMugRunMatchState::WaitingToStart:
	default:
		return TEXT("Waiting");
	}
}

FString AWizardStaffGameMode::GetPartyMatchStateText() const
{
	return GetPartyMatchStateText(PartyMatchState);
}

FString AWizardStaffGameMode::GetActiveTrialStateText() const
{
	return GetTrialStateText(ActiveTrialState);
}

int32 AWizardStaffGameMode::GetPlayerRoundWins(int32 PlayerIndex) const
{
	return PlayerRoundWins.IsValidIndex(PlayerIndex) ? PlayerRoundWins[PlayerIndex] : 0;
}

int32 AWizardStaffGameMode::GetPlayerGrandWizardFavor(int32 PlayerIndex) const
{
	return PlayerGrandWizardFavor.IsValidIndex(PlayerIndex) ? PlayerGrandWizardFavor[PlayerIndex] : 0;
}

FString AWizardStaffGameMode::GetGrandWizardFavorFeedbackMessage() const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return Now <= GrandWizardFavorFeedbackExpireTime ? GrandWizardFavorFeedbackMessage : FString();
}

void AWizardStaffGameMode::ResetPlaytestTelemetry()
{
	PlayerPlaytestStats.Reset();
	StaffsAtDawnTelemetryRingOuts = 0;
	StaffsAtDawnTelemetryRespawns = 0;
	StaffsAtDawnTelemetryRoundsCompleted = 0;
	RecentStaffsAtDawnBroomBoostTimes.Reset();
	StaffsAtDawnBroomBoostRingOutThreats.Reset();
	EnsurePlayerPlaytestStatsSize(GetDesiredLocalPlayerCountForSession());
	SyncPlaytestTelemetryRoundWins();
}

void AWizardStaffGameMode::RecordTelemetryMugCollected(AWizardStaffWizardCharacter* Wizard)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(GetPlayerIndexForWizard(Wizard)))
	{
		++Stats->MugsCollected;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d mugs collected %d."), GetPlayerIndexForWizard(Wizard) + 1, Stats->MugsCollected);
		}
	}
}

bool AWizardStaffGameMode::TryGrantMugRunBrewReward(AWizardStaffWizardCharacter* Wizard)
{
	if (!Wizard
		|| !MugRunTuning.bEnableBrewRewardsInMugRun
		|| ActiveTrialType != EWizardTrialType::MugRun
		|| ActiveTrialState != EWizardTrialState::Active
		|| MugRunMatchState != EWizardMugRunMatchState::Playing)
	{
		return false;
	}

	const float RewardChance = FMath::Clamp(MugRunTuning.BrewRewardChance, 0.0f, 1.0f);
	if (RewardChance <= 0.0f || FMath::FRand() > RewardChance)
	{
		return false;
	}

	return Wizard->TryGrantBrewReward(EWizardBrewRewardType::ArcanePinball, MugRunTuning.bReplaceExistingRewardOnPickup);
}

void AWizardStaffGameMode::RecordTelemetryStaffSegmentGained(AWizardStaffWizardCharacter* Wizard)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(GetPlayerIndexForWizard(Wizard)))
	{
		++Stats->StaffSegmentsGained;
	}
}

void AWizardStaffGameMode::RecordTelemetryStaffSegmentSnapped(AWizardStaffWizardCharacter* Wizard)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(GetPlayerIndexForWizard(Wizard)))
	{
		++Stats->StaffSegmentsSnappedOff;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d snapped segments %d."), GetPlayerIndexForWizard(Wizard) + 1, Stats->StaffSegmentsSnappedOff);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryBonkAttempt(AWizardStaffWizardCharacter* Attacker)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(GetPlayerIndexForWizard(Attacker)))
	{
		++Stats->BonksAttempted;
	}
}

void AWizardStaffGameMode::RecordTelemetryBonkLanded(AWizardStaffWizardCharacter* Attacker, AWizardStaffWizardCharacter* Victim)
{
	const int32 AttackerIndex = GetPlayerIndexForWizard(Attacker);
	const int32 VictimIndex = GetPlayerIndexForWizard(Victim);
	if (IsStaffsAtDawnActive() && AttackerIndex != INDEX_NONE && VictimIndex != INDEX_NONE && AttackerIndex != VictimIndex)
	{
		const TWeakObjectPtr<AWizardStaffWizardCharacter> VictimKey(Victim);
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		RecentBonkAttackerPlayerIndexes.Add(VictimKey, AttackerIndex);
		RecentBonkTimes.Add(VictimKey, Now);
		RecentBonkWasStaffClash.Add(VictimKey, false);
		AddStaffsAtDawnScore(AttackerIndex, StaffsAtDawnTuning.PointsPerBonk, FString::Printf(TEXT("Bonked P%d"), VictimIndex + 1));
		AddStaffsAtDawnBonkSegmentProgress(AttackerIndex);
	}

	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	FWizardPlayerPlaytestStats* AttackerStats = FindOrAddPlayerPlaytestStats(AttackerIndex);
	FWizardPlayerPlaytestStats* VictimStats = FindOrAddPlayerPlaytestStats(VictimIndex);
	if (AttackerStats)
	{
		++AttackerStats->BonksLanded;
	}
	if (VictimStats)
	{
		++VictimStats->TimesBonked;
	}
}

void AWizardStaffGameMode::RecordTelemetryStaffClashStarted(AWizardStaffWizardCharacter* WizardA, AWizardStaffWizardCharacter* WizardB)
{
	const int32 PlayerAIndex = GetPlayerIndexForWizard(WizardA);
	const int32 PlayerBIndex = GetPlayerIndexForWizard(WizardB);
	PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType::StaffClashStarted,
		PlayerAIndex != INDEX_NONE && PlayerBIndex != INDEX_NONE
			? FString::Printf(TEXT("STAFF CLASH! P%d vs P%d"), PlayerAIndex + 1, PlayerBIndex + 1)
			: TEXT("STAFF CLASH!"),
		PlayerAIndex,
		PlayerBIndex,
		0.0f,
		false);

	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* StatsA = FindOrAddPlayerPlaytestStats(PlayerAIndex))
	{
		++StatsA->StaffClashesStarted;
	}
	if (FWizardPlayerPlaytestStats* StatsB = FindOrAddPlayerPlaytestStats(PlayerBIndex))
	{
		++StatsB->StaffClashesStarted;
	}

	if (bShowPlaytestTelemetryDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Telemetry: Staff Clash started P%d vs P%d."), PlayerAIndex + 1, PlayerBIndex + 1);
	}
}

void AWizardStaffGameMode::RecordTelemetryStaffClashWon(AWizardStaffWizardCharacter* Winner, AWizardStaffWizardCharacter* Loser)
{
	RecordTelemetryBonkLanded(Winner, Loser);

	const int32 WinnerIndex = GetPlayerIndexForWizard(Winner);
	const int32 LoserIndex = GetPlayerIndexForWizard(Loser);
	PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType::StaffClashResolved,
		WinnerIndex != INDEX_NONE
			? FString::Printf(TEXT("P%d won Staff Clash"), WinnerIndex + 1)
			: TEXT("Staff Clash resolved"),
		WinnerIndex,
		LoserIndex,
		0.0f,
		false);

	if (bEnablePlaytestTelemetry)
	{
		if (FWizardPlayerPlaytestStats* WinnerStats = FindOrAddPlayerPlaytestStats(WinnerIndex))
		{
			++WinnerStats->StaffClashesWon;
		}
	}

	if (IsStaffsAtDawnActive() && WinnerIndex != INDEX_NONE && Loser)
	{
		const TWeakObjectPtr<AWizardStaffWizardCharacter> LoserKey(Loser);
		RecentBonkWasStaffClash.Add(LoserKey, true);
	}

	if (bEnablePlaytestTelemetry && bShowPlaytestTelemetryDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d won Staff Clash."), WinnerIndex + 1);
	}
}

void AWizardStaffGameMode::RecordTelemetryStaffClashTied(AWizardStaffWizardCharacter* WizardA, AWizardStaffWizardCharacter* WizardB)
{
	const int32 PlayerAIndex = GetPlayerIndexForWizard(WizardA);
	const int32 PlayerBIndex = GetPlayerIndexForWizard(WizardB);
	PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType::StaffClashResolved,
		PlayerAIndex != INDEX_NONE && PlayerBIndex != INDEX_NONE
			? FString::Printf(TEXT("Staff Clash tied P%d-P%d"), PlayerAIndex + 1, PlayerBIndex + 1)
			: TEXT("Staff Clash tied"),
		PlayerAIndex,
		PlayerBIndex,
		0.0f,
		false);

	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* StatsA = FindOrAddPlayerPlaytestStats(PlayerAIndex))
	{
		++StatsA->StaffClashTies;
	}
	if (FWizardPlayerPlaytestStats* StatsB = FindOrAddPlayerPlaytestStats(PlayerBIndex))
	{
		++StatsB->StaffClashTies;
	}

	if (bShowPlaytestTelemetryDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Telemetry: Staff Clash tied P%d vs P%d."), PlayerAIndex + 1, PlayerBIndex + 1);
	}
}

void AWizardStaffGameMode::RecordTelemetryOutOfArenaRespawn(AWizardStaffWizardCharacter* Wizard)
{
	const int32 VictimIndex = GetPlayerIndexForWizard(Wizard);
	const bool bStaffsAtDawnRespawn = IsStaffsAtDawnActive() && Wizard;
	bool bCreditedStaffsAtDawnRingOut = false;
	bool bCreditedStaffClashRingOut = false;
	int32 CreditedRingOutAttackerIndex = INDEX_NONE;
	if (bStaffsAtDawnRespawn)
	{
		const TWeakObjectPtr<AWizardStaffWizardCharacter> VictimKey(Wizard);
		const int32* AttackerIndex = RecentBonkAttackerPlayerIndexes.Find(VictimKey);
		const float* HitTime = RecentBonkTimes.Find(VictimKey);
		const bool* bLastBonkWasStaffClash = RecentBonkWasStaffClash.Find(VictimKey);
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		if (AttackerIndex && HitTime
			&& *AttackerIndex != INDEX_NONE
			&& *AttackerIndex != VictimIndex
			&& Now - *HitTime <= FMath::Max(StaffsAtDawnTuning.OutOfArenaCreditWindow, 0.0f))
		{
			CreditedRingOutAttackerIndex = *AttackerIndex;
			bCreditedStaffsAtDawnRingOut = true;
			bCreditedStaffClashRingOut = bLastBonkWasStaffClash && *bLastBonkWasStaffClash;
			AddStaffsAtDawnScore(CreditedRingOutAttackerIndex, StaffsAtDawnTuning.PointsPerOutOfArena, FString::Printf(TEXT("Ring-out P%d"), VictimIndex + 1));
			if (GrandWizardFavorTuning.bGrantFavorForStaffsAtDawnRingOuts)
			{
				AddGrandWizardFavor(CreditedRingOutAttackerIndex, GrandWizardFavorTuning.FavorPerStaffsAtDawnRingOut, TEXT("Ring-Out"));
			}
			GrantStaffsAtDawnCombatSegments(CreditedRingOutAttackerIndex, StaffsAtDawnTuning.StaffSegmentsPerOutOfArena, TEXT("Ring-Out"));
		}

		RecentBonkAttackerPlayerIndexes.Remove(VictimKey);
		RecentBonkTimes.Remove(VictimKey);
		RecentBonkWasStaffClash.Remove(VictimKey);
	}

	if (bStaffsAtDawnRespawn)
	{
		RecordTelemetryBroomBoostFailedRingOutRecovery(Wizard);

		if (Wizard->StaffComponent)
		{
			Wizard->ClearMegaStaffBrew(false);
			Wizard->StaffComponent->ClearStaffSegments();
			if (StaffsAtDawnBonksTowardSegment.IsValidIndex(VictimIndex))
			{
				StaffsAtDawnBonksTowardSegment[VictimIndex] = 0;
			}
			if (GEngine && StaffsAtDawnTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
			{
				const FString VictimLabel = VictimIndex != INDEX_NONE ? FString::Printf(TEXT("P%d"), VictimIndex + 1) : FString(TEXT("Wizard"));
				GEngine->AddOnScreenDebugMessage(
					4820 + FMath::Max(VictimIndex, 0),
					1.25f,
					FColor::Cyan,
					FString::Printf(TEXT("%s ring-out: staff reset"), *VictimLabel));
			}
		}
	}

	if (VictimIndex != INDEX_NONE)
	{
		PublishReplicatedGameplayEvent(
			EWizardReplicatedGameplayEventType::RespawnComplete,
			FString::Printf(TEXT("P%d respawned"), VictimIndex + 1),
			VictimIndex,
			CreditedRingOutAttackerIndex,
			0.0f,
			true,
			1.8f);
	}

	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (bStaffsAtDawnRespawn)
	{
		++StaffsAtDawnTelemetryRespawns;
		if (bCreditedStaffsAtDawnRingOut)
		{
			++StaffsAtDawnTelemetryRingOuts;
			if (FWizardPlayerPlaytestStats* AttackerStats = FindOrAddPlayerPlaytestStats(CreditedRingOutAttackerIndex))
			{
				++AttackerStats->StaffsAtDawnRingOutsCaused;
				if (bCreditedStaffClashRingOut)
				{
					++AttackerStats->StaffClashRingOutsCaused;
				}
			}
			if (AWizardStaffWizardCharacter* AttackerWizard = GetWizardForPlayerIndex(CreditedRingOutAttackerIndex))
			{
				if (AttackerWizard->IsMegaStaffBrewActive())
				{
					RecordTelemetryMegaStaffRingOut(AttackerWizard);
				}
			}
			if (bShowPlaytestTelemetryDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("Telemetry: Staffs at Dawn credited ring-out P%d over P%d."), CreditedRingOutAttackerIndex + 1, VictimIndex + 1);
			}
		}
	}

	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(VictimIndex))
	{
		++Stats->OutOfArenaRespawns;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d out-of-arena respawns %d."), VictimIndex + 1, Stats->OutOfArenaRespawns);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryBroomBoostUsed(AWizardStaffWizardCharacter* Wizard)
{
	if (!Wizard || !IsStaffsAtDawnActive())
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	RecentStaffsAtDawnBroomBoostTimes.Add(WizardKey, Now);
	if (PendingOutOfArenaRespawns.Contains(WizardKey))
	{
		MarkStaffsAtDawnBroomBoostRingOutThreat(Wizard);
	}

	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->BroomBoostsUsed;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Staffs at Dawn broom boosts used %d."), PlayerIndex + 1, Stats->BroomBoostsUsed);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryArcanePinballRewardReceived(AWizardStaffWizardCharacter* Wizard)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->ArcanePinballRewardsReceived;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Arcane Pinball rewards received %d."), PlayerIndex + 1, Stats->ArcanePinballRewardsReceived);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryArcanePinballCast(AWizardStaffWizardCharacter* Caster, float CastStressGained)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 CasterIndex = GetPlayerIndexForWizard(Caster);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(CasterIndex))
	{
		++Stats->ArcanePinballsCast;
		Stats->ArcanePinballCastStressGained += FMath::Max(CastStressGained, 0.0f);
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Arcane Pinballs cast %d, cast stress %.1f."),
				CasterIndex + 1,
				Stats->ArcanePinballsCast,
				Stats->ArcanePinballCastStressGained);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryArcanePinballHit(
	AWizardStaffWizardCharacter* Caster,
	AWizardStaffWizardCharacter* HitWizard,
	bool bSelfHit,
	float HitStressGained)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 CasterIndex = GetPlayerIndexForWizard(Caster);
	const int32 HitPlayerIndex = GetPlayerIndexForWizard(HitWizard);
	const float SafeHitStress = FMath::Max(HitStressGained, 0.0f);

	if (bSelfHit)
	{
		if (FWizardPlayerPlaytestStats* HitStats = FindOrAddPlayerPlaytestStats(HitPlayerIndex))
		{
			++HitStats->ArcanePinballSelfHits;
			HitStats->ArcanePinballHitStressGained += SafeHitStress;
			if (bShowPlaytestTelemetryDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Arcane Pinball self-hits %d, hit stress %.1f."),
					HitPlayerIndex + 1,
					HitStats->ArcanePinballSelfHits,
					HitStats->ArcanePinballHitStressGained);
			}
		}
		return;
	}

	if (FWizardPlayerPlaytestStats* CasterStats = FindOrAddPlayerPlaytestStats(CasterIndex))
	{
		++CasterStats->ArcanePinballHitsOnPlayers;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Arcane Pinball hits on other players %d."),
				CasterIndex + 1,
				CasterStats->ArcanePinballHitsOnPlayers);
		}
	}

	if (FWizardPlayerPlaytestStats* HitStats = FindOrAddPlayerPlaytestStats(HitPlayerIndex))
	{
		HitStats->ArcanePinballHitStressGained += SafeHitStress;
	}
}

void AWizardStaffGameMode::RecordTelemetryArcanePinballBounce(AWizardStaffWizardCharacter* Caster)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 CasterIndex = GetPlayerIndexForWizard(Caster);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(CasterIndex))
	{
		++Stats->ArcanePinballTotalBounces;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Telemetry: P%d Arcane Pinball total bounces %d."), CasterIndex + 1, Stats->ArcanePinballTotalBounces);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryMegaStaffPickup(AWizardStaffWizardCharacter* Wizard, int32 SegmentsGranted)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->MegaStaffPickupsCollected;
		Stats->MegaStaffSegmentsGranted += FMath::Max(SegmentsGranted, 0);
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Mega Staff pickups %d, segments granted %d."),
				PlayerIndex + 1,
				Stats->MegaStaffPickupsCollected,
				Stats->MegaStaffSegmentsGranted);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryMegaStaffSegmentSnapped(AWizardStaffWizardCharacter* Wizard)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->MegaStaffSegmentsSnappedDuringEffect;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Mega Staff snapped temp segments %d."),
				PlayerIndex + 1,
				Stats->MegaStaffSegmentsSnappedDuringEffect);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryMegaStaffRingOut(AWizardStaffWizardCharacter* Wizard)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->MegaStaffRingOutsCaused;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d Mega Staff ring-outs %d."),
				PlayerIndex + 1,
				Stats->MegaStaffRingOutsCaused);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryLooseSegmentChaosHit(AWizardStaffWizardCharacter* Wizard, EWizardLooseSegmentChaosEffectType EffectType, float ManaSloshAdded)
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->LooseSegmentChaosHits;
		Stats->LooseSegmentManaSloshAdded += FMath::Max(ManaSloshAdded, 0.0f);

		if (EffectType == EWizardLooseSegmentChaosEffectType::TripBonk)
		{
			++Stats->LooseSegmentTripBonks;
		}
		else if (EffectType == EWizardLooseSegmentChaosEffectType::ArcanePop)
		{
			++Stats->LooseSegmentArcanePops;
		}

		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d loose segment chaos hits %d, slosh %.1f, trip bonks %d, arcane pops %d."),
				PlayerIndex + 1,
				Stats->LooseSegmentChaosHits,
				Stats->LooseSegmentManaSloshAdded,
				Stats->LooseSegmentTripBonks,
				Stats->LooseSegmentArcanePops);
		}
	}
}

FWizardPlayerPlaytestStats AWizardStaffGameMode::GetPlayerPlaytestStats(int32 PlayerIndex) const
{
	return PlayerPlaytestStats.IsValidIndex(PlayerIndex) ? PlayerPlaytestStats[PlayerIndex] : FWizardPlayerPlaytestStats();
}

void AWizardStaffGameMode::EnsurePlayerPlaytestStatsSize(int32 MinimumPlayerCount)
{
	int32 DesiredSize = FMath::Max(MinimumPlayerCount, GetDesiredLocalPlayerCountForSession());
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != INDEX_NONE)
		{
			DesiredSize = FMath::Max(DesiredSize, PlayerIndex + 1);
		}
	}

	if (PlayerPlaytestStats.Num() < DesiredSize)
	{
		PlayerPlaytestStats.SetNum(DesiredSize);
	}
}

int32 AWizardStaffGameMode::GetPlayerIndexForWizard(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!Wizard)
	{
		return INDEX_NONE;
	}

	if (const AWizardStaffPlayerState* WizardPlayerState = Wizard->GetPlayerState<AWizardStaffPlayerState>())
	{
		const int32 AssignedSlot = WizardPlayerState->GetWizardDisplaySlot();
		if (AssignedSlot >= 0)
		{
			return AssignedSlot;
		}
	}

	const UWorld* World = GetWorld();
	if (World)
	{
		int32 ControllerIndex = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PlayerController = It->Get();
			if (PlayerController && PlayerController->GetPawn() == Wizard)
			{
				return ControllerIndex;
			}
			++ControllerIndex;
		}
	}

	const APlayerState* PlayerState = Wizard->GetPlayerState();
	return PlayerState && PlayerState->GetPlayerId() >= 0 ? PlayerState->GetPlayerId() : INDEX_NONE;
}

AWizardStaffGameState* AWizardStaffGameMode::GetWizardStaffGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AWizardStaffGameState>() : nullptr;
}

AWizardStaffPlayerState* AWizardStaffGameMode::GetWizardStaffPlayerStateForIndex(int32 PlayerIndex) const
{
	if (PlayerIndex == INDEX_NONE)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (const AWizardStaffGameState* WizardGameState = GetWizardStaffGameState())
	{
		for (APlayerState* BasePlayerState : WizardGameState->PlayerArray)
		{
			AWizardStaffPlayerState* WizardPlayerState = Cast<AWizardStaffPlayerState>(BasePlayerState);
			if (WizardPlayerState && WizardPlayerState->GetWizardDisplaySlot() == PlayerIndex)
			{
				return WizardPlayerState;
			}
		}
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
		const AWizardStaffPlayerState* WizardPlayerState = Cast<AWizardStaffPlayerState>(PlayerState);
		if (WizardPlayerState && WizardPlayerState->GetWizardDisplaySlot() >= 0)
		{
			continue;
		}
		if (PlayerState && PlayerState->GetPlayerId() >= 0 && PlayerState->GetPlayerId() == PlayerIndex)
		{
			return Cast<AWizardStaffPlayerState>(PlayerController->PlayerState);
		}
	}

	int32 ControllerIndex = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const AWizardStaffPlayerState* WizardPlayerState = PlayerController
			? Cast<AWizardStaffPlayerState>(PlayerController->PlayerState)
			: nullptr;
		const bool bHasAssignedSlot = WizardPlayerState && WizardPlayerState->GetWizardDisplaySlot() >= 0;
		if (PlayerController && !bHasAssignedSlot && ControllerIndex == PlayerIndex)
		{
			return Cast<AWizardStaffPlayerState>(PlayerController->PlayerState);
		}
		++ControllerIndex;
	}

	return nullptr;
}

void AWizardStaffGameMode::SyncReplicatedObservableState()
{
	if (AWizardStaffGameState* WizardGameState = GetWizardStaffGameState())
	{
		PrototypeSessionMode = DetectPrototypeSessionMode();
		WizardGameState->SetPrototypeSessionModeMirror(PrototypeSessionMode);

		const FString ResultMessage = !GrandWizardWinnerMessage.IsEmpty() ? GrandWizardWinnerMessage : MugRunWinnerMessage;
		WizardGameState->SetMatchStateMirror(
			PartyMatchState,
			ActiveTrialState,
			ActiveTrialType,
			ActivePrototypeTuningPreset,
			CompletedTrialCount,
			GrandWizardCandidatePlayerIndex,
			GrandWizardWinnerPlayerIndex,
			IsGrandWizardCandidateVulnerable(),
			GrandWizardStealPlayerIndex,
			GetGrandWizardStealProgressAlpha(),
			ResultMessage);
		WizardGameState->SetTimerMirror(
			MugRunRemainingTime,
			StaffsAtDawnRemainingTime,
			TrialCountdownRemainingTime,
			TrialResultsRemainingTime,
			IntermissionRemainingTime,
			FinalRoundRemainingTime);
		WizardGameState->SetCauldronMirror(
			CauldronRemainingTime,
			CauldronCursedPlayerIndex,
			CauldronCurseRemainingTime,
			GetCauldronBankingPlayerIndex(),
			CauldronBankingTransferredCount,
			GetCauldronBankingProgressAlpha());
	}

	SyncAllReplicatedPlayerStates();
}

void AWizardStaffGameMode::PublishReplicatedGameplayEvent(
	EWizardReplicatedGameplayEventType EventType,
	const FString& DisplayText,
	int32 PrimaryPlayerIndex,
	int32 SecondaryPlayerIndex,
	float NumericValue,
	bool bAlsoPushLocalHudMessage,
	float LocalHudLifetime)
{
	if (!HasAuthority() || DisplayText.IsEmpty())
	{
		return;
	}

	if (AWizardStaffGameState* WizardGameState = GetWizardStaffGameState())
	{
		WizardGameState->AddReplicatedGameplayEvent(EventType, DisplayText, PrimaryPlayerIndex, SecondaryPlayerIndex, NumericValue);
	}

	if (bAlsoPushLocalHudMessage)
	{
		AWizardStaffHUD::PushGameplayMessage(
			this,
			DisplayText,
			GetReplicatedGameplayEventColor(EventType),
			LocalHudLifetime,
			GetReplicatedGameplayEventHudCategory(EventType));
	}
}

void AWizardStaffGameMode::ClearReplicatedGameplayEventFeed()
{
	if (!HasAuthority())
	{
		return;
	}

	if (AWizardStaffGameState* WizardGameState = GetWizardStaffGameState())
	{
		WizardGameState->ClearReplicatedGameplayEvents();
	}
}

void AWizardStaffGameMode::SyncReplicatedPlayerStateForIndex(int32 PlayerIndex)
{
	AWizardStaffPlayerState* WizardPlayerState = GetWizardStaffPlayerStateForIndex(PlayerIndex);
	if (!WizardPlayerState)
	{
		return;
	}

	const AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	const int32 StaffSegmentScore = Wizard ? Wizard->GetStaffSegmentCount() : 0;
	const int32 StaffsAtDawnScore = GetStaffsAtDawnScore(PlayerIndex);
	const int32 CauldronScore = GetCauldronScore(PlayerIndex);
	const int32 CurrentTrialScore = ActiveTrialType == EWizardTrialType::StaffsAtDawn
		? StaffsAtDawnScore
		: (ActiveTrialType == EWizardTrialType::CauldronCatastrophe ? CauldronScore : StaffSegmentScore);
	const bool bReady = IsPartyHallPlayerReady(PlayerIndex);
	const bool bBot = IsPlayerIndexPlaytestBot(PlayerIndex);
	const FString SummaryText = FString::Printf(
		TEXT("P%d Staff %d Favor %d Wins %d Dawn %d Cauldron %d%s"),
		PlayerIndex + 1,
		StaffSegmentScore,
		GetPlayerGrandWizardFavor(PlayerIndex),
		GetPlayerRoundWins(PlayerIndex),
		StaffsAtDawnScore,
		CauldronScore,
		bBot ? TEXT(" BOT") : TEXT(""));

	const bool bMirrorChanged = WizardPlayerState->SetWizardPlayerMirror(
		PlayerIndex,
		PlayerIndex,
		GetPlayerRoundWins(PlayerIndex),
		GetPlayerGrandWizardFavor(PlayerIndex),
		CurrentTrialScore,
		StaffsAtDawnScore,
		bReady,
		bBot,
		SummaryText);
	if (bMirrorChanged)
	{
		WizardPlayerState->ForceNetUpdate();
	}
}

void AWizardStaffGameMode::SyncAllReplicatedPlayerStates()
{
	int32 DesiredSize = FMath::Max(GetDesiredLocalPlayerCountForSession(), PlayerRoundWins.Num());
	DesiredSize = FMath::Max(DesiredSize, PlayerGrandWizardFavor.Num());
	DesiredSize = FMath::Max(DesiredSize, StaffsAtDawnScores.Num());
	DesiredSize = FMath::Max(DesiredSize, CauldronScores.Num());
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != INDEX_NONE)
		{
			DesiredSize = FMath::Max(DesiredSize, PlayerIndex + 1);
		}
	}

	for (int32 PlayerIndex = 0; PlayerIndex < DesiredSize; ++PlayerIndex)
	{
		SyncReplicatedPlayerStateForIndex(PlayerIndex);
	}
}

void AWizardStaffGameMode::DebugServerAddStaffSegmentToPlayer(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerAddStaffSegmentToPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerAddStaffSegmentToPlayer could not find P%d wizard/staff."), PlayerIndex + 1);
		return;
	}

	const int32 PreviousCount = Wizard->StaffComponent->GetSegmentCount();
	const int32 NewCount = Wizard->StaffComponent->AddStaffSegment();
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server added staff segment to P%d (%d -> %d)."), PlayerIndex + 1, PreviousCount, NewCount);
	SyncReplicatedPlayerStateForIndex(PlayerIndex);
#endif
}

void AWizardStaffGameMode::DebugServerRemoveStaffSegmentFromPlayer(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerRemoveStaffSegmentFromPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerRemoveStaffSegmentFromPlayer could not find P%d wizard/staff."), PlayerIndex + 1);
		return;
	}

	const int32 PreviousCount = Wizard->StaffComponent->GetSegmentCount();
	const bool bRemoved = Wizard->StaffComponent->RemoveTopStaffSegment(false);
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server removed staff segment from P%d (%d -> %d, removed=%s)."),
		PlayerIndex + 1,
		PreviousCount,
		Wizard->StaffComponent->GetSegmentCount(),
		bRemoved ? TEXT("true") : TEXT("false"));
	SyncReplicatedPlayerStateForIndex(PlayerIndex);
#endif
}

void AWizardStaffGameMode::DebugServerClearStaffSegmentsForPlayer(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerClearStaffSegmentsForPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerClearStaffSegmentsForPlayer could not find P%d wizard/staff."), PlayerIndex + 1);
		return;
	}

	const int32 PreviousCount = Wizard->StaffComponent->GetSegmentCount();
	Wizard->StaffComponent->ClearStaffSegments();
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server cleared staff segments for P%d (%d -> 0)."), PlayerIndex + 1, PreviousCount);
	SyncReplicatedPlayerStateForIndex(PlayerIndex);
#endif
}

void AWizardStaffGameMode::DebugServerSnapTopStaffSegmentForPlayer(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerSnapTopStaffSegmentForPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerSnapTopStaffSegmentForPlayer could not find P%d wizard/staff."), PlayerIndex + 1);
		return;
	}

	const int32 PreviousCount = Wizard->StaffComponent->GetSegmentCount();
	const bool bSnapped = Wizard->StaffComponent->SnapTopStaffSegment();
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server snapped staff segment for P%d (%d -> %d, snapped=%s)."),
		PlayerIndex + 1,
		PreviousCount,
		Wizard->StaffComponent->GetSegmentCount(),
		bSnapped ? TEXT("true") : TEXT("false"));
	SyncReplicatedPlayerStateForIndex(PlayerIndex);
#endif
}

void AWizardStaffGameMode::DebugServerAddManaSloshToPlayer(int32 PlayerIndex, float SloshAmount)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerAddManaSloshToPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerAddManaSloshToPlayer could not find P%d wizard."), PlayerIndex + 1);
		return;
	}

	const float PreviousSlosh = Wizard->GetReadableManaSlosh();
	Wizard->AddManaSlosh(FMath::Max(SloshAmount, 0.0f));
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server added Mana Slosh to P%d (%.1f -> %.1f)."), PlayerIndex + 1, PreviousSlosh, Wizard->GetReadableManaSlosh());
#endif
}

void AWizardStaffGameMode::DebugServerClearManaSloshForPlayer(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerClearManaSloshForPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerClearManaSloshForPlayer could not find P%d wizard."), PlayerIndex + 1);
		return;
	}

	Wizard->ManaSlosh = 0.0f;
	Wizard->SyncReplicatedManaSloshFromAuthority(true);
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server cleared Mana Slosh for P%d."), PlayerIndex + 1);
#endif
}

void AWizardStaffGameMode::DebugServerAddStaffStressToPlayer(int32 PlayerIndex, float StressAmount)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerAddStaffStressToPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerAddStaffStressToPlayer could not find P%d wizard/staff."), PlayerIndex + 1);
		return;
	}

	const float PreviousStress = Wizard->GetReadableStaffStress();
	const float SafeStressAmount = FMath::Max(StressAmount, 0.0f);
	if (Wizard->GetStaffSegmentCount() > 0)
	{
		Wizard->StaffComponent->AddStaffStress(SafeStressAmount, TEXT("OnlineScaffoldDebugStress"));
	}
	else
	{
		const float SafeMaxStress = FMath::Max(Wizard->StaffComponent->StressTuning.MaxStaffStress, 1.0f);
		Wizard->StaffComponent->StaffStress = FMath::Clamp(Wizard->StaffComponent->StaffStress + SafeStressAmount, 0.0f, SafeMaxStress);
		Wizard->SyncReplicatedStaffStressFromAuthority(true);
	}
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server added Staff Stress to P%d (%.1f -> %.1f)."), PlayerIndex + 1, PreviousStress, Wizard->GetReadableStaffStress());
#endif
}

void AWizardStaffGameMode::DebugServerClearStaffStressForPlayer(int32 PlayerIndex)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerClearStaffStressForPlayer ignored outside server authority."));
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugServerClearStaffStressForPlayer could not find P%d wizard/staff."), PlayerIndex + 1);
		return;
	}

	Wizard->StaffComponent->StaffStress = 0.0f;
	Wizard->SyncReplicatedStaffStressFromAuthority(true);
	UE_LOG(LogTemp, Log, TEXT("Online scaffold: server cleared Staff Stress for P%d."), PlayerIndex + 1);
#endif
}

void AWizardStaffGameMode::BumpMatchSessionGeneration()
{
	if (AWizardStaffGameState* WizardGameState = GetWizardStaffGameState())
	{
		WizardGameState->IncrementMatchSessionGeneration();
	}
}

FWizardPlayerPlaytestStats* AWizardStaffGameMode::FindOrAddPlayerPlaytestStats(int32 PlayerIndex)
{
	if (PlayerIndex == INDEX_NONE)
	{
		return nullptr;
	}

	EnsurePlayerPlaytestStatsSize(PlayerIndex + 1);
	return PlayerPlaytestStats.IsValidIndex(PlayerIndex) ? &PlayerPlaytestStats[PlayerIndex] : nullptr;
}

bool AWizardStaffGameMode::HasRecentStaffsAtDawnBroomBoost(AWizardStaffWizardCharacter* Wizard, float CurrentTime) const
{
	if (!Wizard || !IsStaffsAtDawnActive())
	{
		return false;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
	const float* BoostTime = RecentStaffsAtDawnBroomBoostTimes.Find(WizardKey);
	return BoostTime
		&& CurrentTime - *BoostTime <= FMath::Max(StaffsAtDawnTuning.BroomBoostRecoveryTelemetryWindow, 0.0f);
}

void AWizardStaffGameMode::MarkStaffsAtDawnBroomBoostRingOutThreat(AWizardStaffWizardCharacter* Wizard)
{
	if (!Wizard || !IsStaffsAtDawnActive())
	{
		return;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
	StaffsAtDawnBroomBoostRingOutThreats.Add(WizardKey);
	if (bShowPlaytestTelemetryDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d broom boost recovery attempt entered ring-out threat."), GetPlayerIndexForWizard(Wizard) + 1);
	}
}

void AWizardStaffGameMode::RecordTelemetryBroomBoostRingOutSave(AWizardStaffWizardCharacter* Wizard)
{
	if (!Wizard)
	{
		return;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
	if (!StaffsAtDawnBroomBoostRingOutThreats.Contains(WizardKey))
	{
		return;
	}

	StaffsAtDawnBroomBoostRingOutThreats.Remove(WizardKey);
	RecentStaffsAtDawnBroomBoostTimes.Remove(WizardKey);
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->BroomBoostRingOutSaves;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d broom boost ring-out saves %d."), PlayerIndex + 1, Stats->BroomBoostRingOutSaves);
		}
	}
}

void AWizardStaffGameMode::RecordTelemetryBroomBoostFailedRingOutRecovery(AWizardStaffWizardCharacter* Wizard)
{
	if (!Wizard)
	{
		return;
	}

	const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
	if (!StaffsAtDawnBroomBoostRingOutThreats.Contains(WizardKey))
	{
		return;
	}

	StaffsAtDawnBroomBoostRingOutThreats.Remove(WizardKey);
	RecentStaffsAtDawnBroomBoostTimes.Remove(WizardKey);
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
	{
		++Stats->BroomBoostFailedRingOutRecoveries;
		if (bShowPlaytestTelemetryDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("Telemetry: P%d broom boost failed recoveries %d."), PlayerIndex + 1, Stats->BroomBoostFailedRingOutRecoveries);
		}
	}
}

void AWizardStaffGameMode::SyncPlaytestTelemetryRoundWins()
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	EnsurePlayerPlaytestStatsSize(GetDesiredLocalPlayerCountForSession());
	for (int32 PlayerIndex = 0; PlayerIndex < PlayerPlaytestStats.Num(); ++PlayerIndex)
	{
		PlayerPlaytestStats[PlayerIndex].RoundWins = GetPlayerRoundWins(PlayerIndex);
		PlayerPlaytestStats[PlayerIndex].GrandWizardFavorEarned = GetPlayerGrandWizardFavor(PlayerIndex);
	}
}

void AWizardStaffGameMode::AddGrandWizardCandidateTime(float DeltaSeconds)
{
	if (!bEnablePlaytestTelemetry || DeltaSeconds <= 0.0f || GrandWizardCandidatePlayerIndex == INDEX_NONE)
	{
		return;
	}

	if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(GrandWizardCandidatePlayerIndex))
	{
		Stats->TimeAsGrandWizardCandidate += DeltaSeconds;
	}
}

void AWizardStaffGameMode::CaptureFinalPlaytestTelemetrySnapshot()
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	EnsurePlayerPlaytestStatsSize(GetDesiredLocalPlayerCountForSession());
	SyncPlaytestTelemetryRoundWins();
	for (int32 PlayerIndex = 0; PlayerIndex < PlayerPlaytestStats.Num(); ++PlayerIndex)
	{
		const AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
		PlayerPlaytestStats[PlayerIndex].FinalStaffSegmentCount = Wizard ? Wizard->GetStaffSegmentCount() : 0;
		PlayerPlaytestStats[PlayerIndex].bFinalWinner = PlayerIndex == GrandWizardWinnerPlayerIndex;
	}
}

void AWizardStaffGameMode::LogPlaytestTelemetrySummary() const
{
	if (!bEnablePlaytestTelemetry)
	{
		return;
	}

	const FString FinalWinnerText = GrandWizardWinnerPlayerIndex == INDEX_NONE
		? FString(TEXT("no winner"))
		: FString::Printf(TEXT("P%d"), GrandWizardWinnerPlayerIndex + 1);
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff playtest summary: final winner %s."), *FinalWinnerText);
	UE_LOG(LogTemp, Log, TEXT("Staffs at Dawn telemetry: ring-outs %d, rounds %d, average respawns per round %.1f."),
		StaffsAtDawnTelemetryRingOuts,
		StaffsAtDawnTelemetryRoundsCompleted,
		GetAverageStaffsAtDawnRespawnsPerRound());
	for (int32 PlayerIndex = 0; PlayerIndex < PlayerPlaytestStats.Num(); ++PlayerIndex)
	{
		const FWizardPlayerPlaytestStats& Stats = PlayerPlaytestStats[PlayerIndex];
		UE_LOG(LogTemp, Log, TEXT("P%d%s: Favor %d, Mugs %d, Dawn %d, DawnRO %d, DawnSeg+ %d, Broom U/S/F %d/%d/%d, Mega P/Seg/Snap/RO %d/%d/%d/%d, Seg+ %d, Snaps %d, Bonks %d/%d, Clash S/W/T/RO %d/%d/%d/%d, Bonked %d, Respawns %d, Candidate %.1fs, Final Staff %d, Round Wins %d, Pinball reward/cast %d/%d, hit/self %d/%d, bounces %d, stress cast/hit %.1f/%.1f, Loose H/S/T/P %d/%.1f/%d/%d."),
			PlayerIndex + 1,
			Stats.bFinalWinner ? TEXT(" WINNER") : TEXT(""),
			Stats.GrandWizardFavorEarned,
			Stats.MugsCollected,
			Stats.StaffsAtDawnScore,
			Stats.StaffsAtDawnRingOutsCaused,
			Stats.StaffsAtDawnCombatSegmentsGained,
			Stats.BroomBoostsUsed,
			Stats.BroomBoostRingOutSaves,
			Stats.BroomBoostFailedRingOutRecoveries,
			Stats.MegaStaffPickupsCollected,
			Stats.MegaStaffSegmentsGranted,
			Stats.MegaStaffSegmentsSnappedDuringEffect,
			Stats.MegaStaffRingOutsCaused,
			Stats.StaffSegmentsGained,
			Stats.StaffSegmentsSnappedOff,
			Stats.BonksLanded,
			Stats.BonksAttempted,
			Stats.StaffClashesStarted,
			Stats.StaffClashesWon,
			Stats.StaffClashTies,
			Stats.StaffClashRingOutsCaused,
			Stats.TimesBonked,
			Stats.OutOfArenaRespawns,
			Stats.TimeAsGrandWizardCandidate,
			Stats.FinalStaffSegmentCount,
			Stats.RoundWins,
			Stats.ArcanePinballRewardsReceived,
			Stats.ArcanePinballsCast,
			Stats.ArcanePinballHitsOnPlayers,
			Stats.ArcanePinballSelfHits,
			Stats.ArcanePinballTotalBounces,
			Stats.ArcanePinballCastStressGained,
			Stats.ArcanePinballHitStressGained,
			Stats.LooseSegmentChaosHits,
			Stats.LooseSegmentManaSloshAdded,
			Stats.LooseSegmentTripBonks,
			Stats.LooseSegmentArcanePops);
	}
}

int32 AWizardStaffGameMode::GetCurrentStandingLeaderPlayerIndex() const
{
	const TArray<AWizardStaffWizardCharacter*> Wizards = GetCurrentWizards();
	int32 BestPlayerIndex = INDEX_NONE;
	int32 BestFavor = INDEX_NONE;
	int32 BestWins = INDEX_NONE;
	int32 BestSegments = INDEX_NONE;
	bool bTie = false;

	for (const AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		const int32 Favor = GetPlayerGrandWizardFavor(PlayerIndex);
		const int32 Wins = GetPlayerRoundWins(PlayerIndex);
		const int32 Segments = Wizard->GetStaffSegmentCount();
		if (Favor > BestFavor
			|| (Favor == BestFavor && Wins > BestWins)
			|| (Favor == BestFavor && Wins == BestWins && Segments > BestSegments))
		{
			BestFavor = Favor;
			BestWins = Wins;
			BestSegments = Segments;
			BestPlayerIndex = PlayerIndex;
			bTie = false;
		}
		else if (Favor == BestFavor && Wins == BestWins && Segments == BestSegments)
		{
			bTie = true;
		}
	}

	return bTie ? INDEX_NONE : BestPlayerIndex;
}

FString AWizardStaffGameMode::GetActiveTrialName() const
{
	if (PartyMatchState == EWizardPartyMatchState::FinalRound)
	{
		return TEXT("Grand Wizard Final");
	}

	return GetTrialTypeText(ActiveTrialType);
}

int32 AWizardStaffGameMode::GetStaffsAtDawnScore(int32 PlayerIndex) const
{
	return StaffsAtDawnScores.IsValidIndex(PlayerIndex) ? StaffsAtDawnScores[PlayerIndex] : 0;
}

int32 AWizardStaffGameMode::GetCauldronScore(int32 PlayerIndex) const
{
	return CauldronScores.IsValidIndex(PlayerIndex) ? CauldronScores[PlayerIndex] : 0;
}

int32 AWizardStaffGameMode::GetCauldronBankingPlayerIndex() const
{
	return GetPlayerIndexForWizard(CauldronBankingWizard.Get());
}

float AWizardStaffGameMode::GetCauldronBankingProgressAlpha() const
{
	if (!CauldronBankingWizard.IsValid())
	{
		return 0.0f;
	}

	const float FullTransferDuration = CauldronBankingTransferredCount == 0
		? FMath::Max(CauldronCatastropheTuning.BankingInitialDelay, 0.01f)
		: FMath::Max(CauldronCatastropheTuning.BankingVialTransferInterval, 0.01f);
	return FMath::Clamp(1.0f - (CauldronBankingNextTransferRemainingTime / FullTransferDuration), 0.0f, 1.0f);
}

FString AWizardStaffGameMode::GetStaffsAtDawnFeedbackMessage() const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return Now <= StaffsAtDawnFeedbackExpireTime ? StaffsAtDawnFeedbackMessage : FString();
}

bool AWizardStaffGameMode::IsStaffsAtDawnActive() const
{
	return PartyMatchState == EWizardPartyMatchState::Trial
		&& ActiveTrialState == EWizardTrialState::Active
		&& ActiveTrialType == EWizardTrialType::StaffsAtDawn;
}

float AWizardStaffGameMode::GetAverageStaffsAtDawnRespawnsPerRound() const
{
	return StaffsAtDawnTelemetryRoundsCompleted > 0
		? static_cast<float>(StaffsAtDawnTelemetryRespawns) / static_cast<float>(StaffsAtDawnTelemetryRoundsCompleted)
		: 0.0f;
}

bool AWizardStaffGameMode::IsPartyHallIntermissionActive() const
{
	return IsPartyHallActive() && ActiveTrialState == EWizardTrialState::WaitingToStart;
}

float AWizardStaffGameMode::GetPartyHallBonkKnockbackMultiplier() const
{
	return IsPartyHallIntermissionActive() && PartyHallTuning.bUseGentleBonks
		? FMath::Clamp(PartyHallTuning.BonkKnockbackMultiplier, 0.0f, 1.0f)
		: 1.0f;
}

bool AWizardStaffGameMode::ShouldDisablePartyHallBonkStress() const
{
	return IsPartyHallIntermissionActive() && PartyHallTuning.bDisableBonkStaffStress;
}

void AWizardStaffGameMode::NotifyPartyHallReadyBellBonked(AWizardStaffWizardCharacter* Wizard)
{
	if (!PartyHallTuning.bEnableReadyBell || !Wizard || !IsPartyHallIntermissionActive())
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (PlayerIndex == INDEX_NONE)
	{
		return;
	}

	EnsurePartyHallReadyStateSize(PlayerIndex + 1);
	const bool bWasAlreadyReady = IsPartyHallPlayerReady(PlayerIndex);
	PartyHallReadyPlayers[PlayerIndex] = 1;

	if (ActivePartyHall)
	{
		ActivePartyHall->PlayReadyBellFeedback(PlayerIndex);
	}

	const FString PlayerReadyMessage = bWasAlreadyReady
		? FString::Printf(TEXT("P%d rang the Ready Bell again."), PlayerIndex + 1)
		: FString::Printf(TEXT("P%d rang the Ready Bell: READY"), PlayerIndex + 1);
	SetPartyHallReadyFeedbackMessage(PlayerReadyMessage, bWasAlreadyReady ? FColor::Cyan : FColor::Green);

	if (!bPartyHallAllReadyTriggered && AreAllActivePartyHallPlayersReady())
	{
		bPartyHallAllReadyTriggered = true;
		const float AllReadyCountdown = FMath::Max(PartyHallTuning.ReadyBellAllReadyCountdownDuration, 0.0f);
		IntermissionRemainingTime = FMath::Min(IntermissionRemainingTime, AllReadyCountdown);
		SetPartyHallReadyFeedbackMessage(
			AllReadyCountdown <= 0.0f
				? TEXT("All players ready! Starting next Trial.")
				: FString::Printf(TEXT("All players ready! Next Trial in %.1fs"), AllReadyCountdown),
			FColor::Yellow);
	}

	UpdatePartyHallSigns();
	SyncReplicatedObservableState();
}

bool AWizardStaffGameMode::IsPartyHallPlayerReady(int32 PlayerIndex) const
{
	return PartyHallReadyPlayers.IsValidIndex(PlayerIndex) && PartyHallReadyPlayers[PlayerIndex] != 0;
}

int32 AWizardStaffGameMode::GetPartyHallReadyPlayerCount() const
{
	int32 ReadyCount = 0;
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (IsPartyHallPlayerReady(PlayerIndex))
		{
			++ReadyCount;
		}
	}
	return ReadyCount;
}

FString AWizardStaffGameMode::GetPartyHallReadyFeedbackMessage() const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return Now <= PartyHallReadyFeedbackExpireTime ? PartyHallReadyFeedbackMessage : FString();
}

bool AWizardStaffGameMode::GetPartyHallReadyBellLocation(FVector& OutLocation) const
{
	return ActivePartyHall && ActivePartyHall->GetReadyBellWorldLocation(OutLocation);
}

FVector AWizardStaffGameMode::GetPlaytestBoundsCenter() const
{
	return GetCurrentPlayBoundsCenter();
}

float AWizardStaffGameMode::GetPlaytestBoundsHalfSize() const
{
	return GetCurrentPlayBoundsHalfSize();
}

FVector AWizardStaffGameMode::GetPlaytestFinalCircleCenter() const
{
	return GetFinalRoundCircleCenter();
}

int32 AWizardStaffGameMode::GetGrandWizardCircleChallengerPlayerIndex() const
{
	return GetBestGrandWizardCircleChallenger();
}

float AWizardStaffGameMode::GetGrandWizardStealProgressAlpha() const
{
	const float SafeHoldDuration = FMath::Max(FinalRoundTuning.CandidateSwapHoldDuration, 0.0f);
	return SafeHoldDuration <= 0.0f ? 0.0f : FMath::Clamp(GrandWizardStealHoldTime / SafeHoldDuration, 0.0f, 1.0f);
}

bool AWizardStaffGameMode::IsGrandWizardCandidateVulnerable() const
{
	if (PartyMatchState != EWizardPartyMatchState::FinalRound
		|| ActiveTrialState != EWizardTrialState::Active
		|| GrandWizardCandidatePlayerIndex == INDEX_NONE)
	{
		return false;
	}

	if (FinalRoundTuning.bLeaderStartsWithBonus && GrandWizardStartBonusRemainingTime > 0.0f)
	{
		return false;
	}

	if (!FinalRoundTuning.bRequireCandidateOutsideCircleToSteal)
	{
		return true;
	}

	const AWizardStaffWizardCharacter* CandidateWizard = GetWizardForPlayerIndex(GrandWizardCandidatePlayerIndex);
	const float ControlRadius = FMath::Max(FinalRoundTuning.CircleRadius + FinalRoundTuning.CandidateNearCirclePadding, FinalRoundTuning.CircleRadius);
	return !CandidateWizard || !IsWizardInsideFinalRoundCircle(CandidateWizard, ControlRadius);
}

FString AWizardStaffGameMode::GetGrandWizardCandidateText() const
{
	return GrandWizardCandidatePlayerIndex == INDEX_NONE
		? TEXT("None")
		: FString::Printf(TEXT("P%d"), GrandWizardCandidatePlayerIndex + 1);
}

FString AWizardStaffGameMode::GetGrandWizardFinalFeedbackMessage() const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	return Now <= GrandWizardFinalFeedbackExpireTime ? GrandWizardFinalFeedbackMessage : FString();
}

void AWizardStaffGameMode::SetPrototypeTuningPreset(EWizardPrototypeTuningPreset NewPreset)
{
	ActivePrototypeTuningPreset = NewPreset;
	ApplyPrototypeTuningPresetValues(GetPrototypeTuningPresetValues(ActivePrototypeTuningPreset));
	bHasAppliedPrototypeTuningPreset = true;
	AnnouncePrototypeTuningPreset();
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::CyclePrototypeTuningPreset()
{
	switch (ActivePrototypeTuningPreset)
	{
	case EWizardPrototypeTuningPreset::Stable:
		SetPrototypeTuningPreset(EWizardPrototypeTuningPreset::Chaotic);
		break;
	case EWizardPrototypeTuningPreset::Chaotic:
		SetPrototypeTuningPreset(EWizardPrototypeTuningPreset::Absurd);
		break;
	case EWizardPrototypeTuningPreset::Absurd:
	default:
		SetPrototypeTuningPreset(EWizardPrototypeTuningPreset::Stable);
		break;
	}
}

void AWizardStaffGameMode::SetPrototypeTuningPresetByName(const FString& PresetName)
{
	const FString NormalizedName = PresetName.TrimStartAndEnd().ToLower();
	if (NormalizedName == TEXT("stable"))
	{
		SetPrototypeTuningPreset(EWizardPrototypeTuningPreset::Stable);
	}
	else if (NormalizedName == TEXT("chaotic") || NormalizedName == TEXT("chaos"))
	{
		SetPrototypeTuningPreset(EWizardPrototypeTuningPreset::Chaotic);
	}
	else if (NormalizedName == TEXT("absurd") || NormalizedName == TEXT("arcane catastrophe") || NormalizedName == TEXT("catastrophe"))
	{
		SetPrototypeTuningPreset(EWizardPrototypeTuningPreset::Absurd);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown Wizard Staff tuning preset '%s'. Use Stable, Chaotic, or Arcane Catastrophe."), *PresetName);
	}
}

FString AWizardStaffGameMode::GetActivePrototypeTuningPresetText() const
{
	return GetPrototypeTuningPresetText(ActivePrototypeTuningPreset);
}

void AWizardStaffGameMode::SetPlaytestBotsEnabled(bool bNewEnabled)
{
	bEnablePlaytestBots = bNewEnabled;
	EnsureLocalPlayers();
	SyncPlaytestBots();

	const FString Message = bEnablePlaytestBots ? TEXT("Playtest bots enabled") : TEXT("Playtest bots disabled");
	AWizardStaffHUD::PushGameplayMessage(this, Message, FColor::Cyan, 1.8f, EWizardHudMessageCategory::Debug);
	UE_LOG(LogTemp, Log, TEXT("%s."), *Message);
	SyncReplicatedObservableState();
}

void AWizardStaffGameMode::TogglePlaytestBots()
{
	SetPlaytestBotsEnabled(!bEnablePlaytestBots);
}

bool AWizardStaffGameMode::IsPlayerIndexPlaytestBot(int32 PlayerIndex) const
{
	if (!IsStandaloneLocalPrototypeSession() || !bEnablePlaytestBots || PlayerIndex == INDEX_NONE)
	{
		return false;
	}

	const int32 HumanPlayers = FMath::Clamp(DesiredHumanPlayers, 1, 4);
	const int32 TotalPlayers = GetDesiredLocalPlayerCountForSession();
	return PlayerIndex >= HumanPlayers && PlayerIndex < TotalPlayers;
}

int32 AWizardStaffGameMode::GetDesiredLocalPlayerCountForSession() const
{
	if (!IsStandaloneLocalPrototypeSession())
	{
		return FMath::Clamp(FMath::Max(GetConnectedPlayerControllerCount(), 2), 2, 4);
	}

	if (!bEnablePlaytestBots || !bFillMissingPlayersWithBots)
	{
		return FMath::Clamp(DesiredLocalPlayerCount, 2, 4);
	}

	const int32 HumanPlayers = FMath::Clamp(DesiredHumanPlayers, 1, 4);
	const int32 TotalPlayers = FMath::Clamp(DesiredTotalPlayers, 2, 4);
	return FMath::Clamp(FMath::Max(HumanPlayers, TotalPlayers), 2, 4);
}

FName AWizardStaffGameMode::GetLooseSnappedSegmentTag()
{
	return FName(TEXT("WizardStaffLooseSegment"));
}

void AWizardStaffGameMode::RegisterLooseSnappedSegment(AActor* LooseSegment)
{
	RegisterLooseSnappedSegmentFromWizard(LooseSegment, nullptr);
}

void AWizardStaffGameMode::RegisterLooseSnappedSegmentFromWizard(AActor* LooseSegment, AWizardStaffWizardCharacter* SourceWizard)
{
	if (!LooseSegment)
	{
		return;
	}

	LooseSegment->Tags.AddUnique(GetLooseSnappedSegmentTag());
	PruneLooseSnappedSegments();

	for (const FWizardTrackedLooseSegment& TrackedSegment : LooseSnappedSegments)
	{
		if (TrackedSegment.Actor.Get() == LooseSegment)
		{
			return;
		}
	}

	FWizardTrackedLooseSegment NewTrackedSegment;
	NewTrackedSegment.Actor = LooseSegment;
	NewTrackedSegment.SourceWizard = SourceWizard;
	NewTrackedSegment.InitialScale = LooseSegment->GetActorScale3D();
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(LooseSegment->GetRootComponent()))
	{
		RootPrimitive->SetNotifyRigidBodyCollision(true);
		RootPrimitive->OnComponentHit.AddUniqueDynamic(this, &AWizardStaffGameMode::HandleLooseSegmentHit);
		NewTrackedSegment.CollisionComponent = RootPrimitive;

		if (LooseSegmentChaosTuning.bEnableLooseSegmentChaosEffects)
		{
			if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(RootPrimitive))
			{
				if (UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
				{
					DynamicMaterial->SetVectorParameterValue(TEXT("Color"), LooseSegmentChaosTuning.ChaosActiveColor);
					DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), LooseSegmentChaosTuning.ChaosActiveColor);
				}
			}
		}
	}
	LooseSnappedSegments.Add(NewTrackedSegment);
	EnforceLooseSnappedSegmentBudget();
}

void AWizardStaffGameMode::CleanupLooseSnappedSegments()
{
	for (FWizardTrackedLooseSegment& TrackedSegment : LooseSnappedSegments)
	{
		if (AActor* LooseSegment = TrackedSegment.Actor.Get())
		{
			LooseSegment->Destroy();
		}
	}
	LooseSnappedSegments.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FName LooseSegmentTag = GetLooseSnappedSegmentTag();
	int32 DestroyedTaggedCount = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) && Actor->ActorHasTag(LooseSegmentTag))
		{
			Actor->Destroy();
			++DestroyedTaggedCount;
		}
	}

	if (DestroyedTaggedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Destroyed %d tagged loose staff segment actors."), DestroyedTaggedCount);
	}
}

void AWizardStaffGameMode::EnsureLocalPlayers()
{
	if (!IsStandaloneLocalPrototypeSession())
	{
		return;
	}

	const int32 TargetPlayerCount = GetDesiredLocalPlayerCountForSession();
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	for (int32 PlayerIndex = GameInstance->GetNumLocalPlayers(); PlayerIndex < TargetPlayerCount; ++PlayerIndex)
	{
		FString Error;
		UGameplayStatics::CreatePlayer(this, PlayerIndex, true);
	}

	SyncPlaytestBots();
}

void AWizardStaffGameMode::SyncPlaytestBots()
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		const bool bShouldBeBot = IsPlayerIndexPlaytestBot(PlayerIndex);
		const bool bWasBot = Wizard->IsPlaytestBot();
		Wizard->SetPlaytestBot(bShouldBeBot);
		if (bShouldBeBot && !bWasBot)
		{
			UE_LOG(LogTemp, Log, TEXT("WizardStaff assigned P%d as a playtest bot."), PlayerIndex + 1);
		}
	}
}

void AWizardStaffGameMode::UpdateKeyboardFallbackControls()
{
	if (!IsStandaloneLocalPrototypeSession() || !KeyboardFallbackControls.bEnabled)
	{
		return;
	}

	APlayerController* PrimaryPlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PrimaryPlayerController)
	{
		return;
	}

	AWizardStaffWizardCharacter* TargetWizard = GetWizardForPlayerIndex(KeyboardFallbackControls.TargetPlayerIndex);
	if (!TargetWizard || TargetWizard->IsPlaytestBot())
	{
		return;
	}

	auto IsKeyDown = [PrimaryPlayerController](const FKey& Key)
	{
		return Key.IsValid() && PrimaryPlayerController->IsInputKeyDown(Key);
	};

	auto WasKeyJustPressed = [PrimaryPlayerController](const FKey& Key)
	{
		return Key.IsValid() && PrimaryPlayerController->WasInputKeyJustPressed(Key);
	};

	auto WasKeyJustReleased = [PrimaryPlayerController](const FKey& Key)
	{
		return Key.IsValid() && PrimaryPlayerController->WasInputKeyJustReleased(Key);
	};

	const float ForwardValue = (IsKeyDown(KeyboardFallbackControls.MoveForwardKey) ? 1.0f : 0.0f)
		+ (IsKeyDown(KeyboardFallbackControls.MoveBackwardKey) ? -1.0f : 0.0f);
	const float RightValue = (IsKeyDown(KeyboardFallbackControls.MoveRightKey) ? 1.0f : 0.0f)
		+ (IsKeyDown(KeyboardFallbackControls.MoveLeftKey) ? -1.0f : 0.0f);
	const float TurnValue = (IsKeyDown(KeyboardFallbackControls.TurnRightKey) ? 1.0f : 0.0f)
		+ (IsKeyDown(KeyboardFallbackControls.TurnLeftKey) ? -1.0f : 0.0f);

	TargetWizard->ApplyPrototypeLocalInput(ForwardValue, RightValue, TurnValue);

	if (WasKeyJustPressed(KeyboardFallbackControls.HopKey))
	{
		TargetWizard->Jump();
	}
	if (WasKeyJustReleased(KeyboardFallbackControls.HopKey))
	{
		TargetWizard->StopJumping();
	}
	if (WasKeyJustPressed(KeyboardFallbackControls.QuickBonkKey))
	{
		TargetWizard->QuickBonk();
	}
	if (WasKeyJustPressed(KeyboardFallbackControls.DrinkMugKey))
	{
		TargetWizard->DrinkMug();
	}
	if (WasKeyJustPressed(KeyboardFallbackControls.UseRewardKey))
	{
		TargetWizard->UseReward();
	}

	if (GEngine && KeyboardFallbackControls.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this) && (!FMath::IsNearlyZero(ForwardValue) || !FMath::IsNearlyZero(RightValue) || !FMath::IsNearlyZero(TurnValue)))
	{
		const FString DebugText = FString::Printf(TEXT("Keyboard fallback driving P%d"), KeyboardFallbackControls.TargetPlayerIndex + 1);
		GEngine->AddOnScreenDebugMessage(4400, 0.0f, FColor::Silver, DebugText);
	}
}

void AWizardStaffGameMode::SpawnSharedCamera()
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer || !bUseSharedCamera || SharedCamera || !SharedCameraClass)
	{
		return;
	}

	SharedCamera = World->SpawnActor<AWizardStaffSharedCamera>(SharedCameraClass, GetCurrentPlayBoundsCenter() + FVector(0.0f, 0.0f, 120.0f), FRotator::ZeroRotator);
}

void AWizardStaffGameMode::AssignSharedCameraToAllPlayers()
{
	if (!bUseSharedCamera || !SharedCamera)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AssignSharedCameraToPlayer(It->Get());
	}
}

void AWizardStaffGameMode::AssignSharedCameraToPlayer(APlayerController* PlayerController)
{
	if (bUseSharedCamera && SharedCamera && PlayerController)
	{
		PlayerController->bAutoManageActiveCameraTarget = false;
		PlayerController->SetViewTarget(SharedCamera);
		if (!PlayerController->IsLocalController())
		{
			FViewTargetTransitionParams TransitionParams;
			PlayerController->ClientSetViewTarget(SharedCamera, TransitionParams);
		}
	}
}

FTransform AWizardStaffGameMode::GetSpawnTransformForController(const AController* Controller) const
{
	if (IsPartyHallActive())
	{
		return GetPartyHallSpawnTransformForController(Controller);
	}

	const int32 TargetPlayerCount = GetDesiredLocalPlayerCountForSession();
	const int32 ControllerIndex = GetControllerIndex(Controller);
	if (bCauldronCatastropheActive && IsValid(ActiveCauldronArena))
	{
		const float AngleRadians = (2.0f * UE_PI * static_cast<float>(ControllerIndex)) / static_cast<float>(TargetPlayerCount);
		const float SpawnDistance = FMath::Min(SpawnRadius, FMath::Max(CauldronCatastropheTuning.ArenaHalfSize - 260.0f, 100.0f));
		const AWizardStaffWizardCharacter* Wizard = Controller ? Cast<AWizardStaffWizardCharacter>(Controller->GetPawn()) : nullptr;
		const float CapsuleHalfHeight = Wizard && Wizard->GetCapsuleComponent()
			? Wizard->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
			: 88.0f;
		const float SpawnZ = ActiveCauldronArena->GetFloorSurfaceZ()
			- ActiveCauldronArena->GetActorLocation().Z
			+ CapsuleHalfHeight + 2.0f;
		const FVector SpawnLocation = ActiveCauldronArena->GetActorLocation()
			+ FVector(FMath::Cos(AngleRadians) * SpawnDistance, FMath::Sin(AngleRadians) * SpawnDistance, SpawnZ);
		FRotator SpawnRotation = (ActiveCauldronArena->GetActorLocation() - SpawnLocation).Rotation();
		SpawnRotation.Pitch = 0.0f;
		SpawnRotation.Roll = 0.0f;
		return FTransform(SpawnRotation, SpawnLocation);
	}

	if (ShouldUseStaffsAtDawnArena())
	{
		FTransform StaffsAtDawnSpawnTransform;
		if (ActiveStaffsAtDawnArena->GetPlayerSpawnTransform(ControllerIndex, TargetPlayerCount, StaffsAtDawnSpawnTransform))
		{
			return StaffsAtDawnSpawnTransform;
		}
	}

	if (!bCauldronCatastropheActive && ActivePrototypeArena)
	{
		FTransform AuthoredSpawnTransform;
		if (ActivePrototypeArena->GetPlayerSpawnTransform(ControllerIndex, TargetPlayerCount, AuthoredSpawnTransform))
		{
			return AuthoredSpawnTransform;
		}
	}

	const FVector ArenaCenter = GetArenaCenter();
	const float AngleRadians = (2.0f * UE_PI * static_cast<float>(ControllerIndex)) / static_cast<float>(TargetPlayerCount);
	const FVector SpawnLocation = ArenaCenter + FVector(FMath::Cos(AngleRadians) * SpawnRadius, FMath::Sin(AngleRadians) * SpawnRadius, 120.0f);
	FRotator SpawnRotation = (ArenaCenter - SpawnLocation).Rotation();
	SpawnRotation.Pitch = 0.0f;
	SpawnRotation.Roll = 0.0f;

	return FTransform(SpawnRotation, SpawnLocation);
}

FTransform AWizardStaffGameMode::GetPartyHallSpawnTransformForController(const AController* Controller) const
{
	const int32 TargetPlayerCount = GetDesiredLocalPlayerCountForSession();
	const int32 ControllerIndex = GetControllerIndex(Controller);
	if (ActivePartyHall)
	{
		FTransform AuthoredSpawnTransform;
		if (ActivePartyHall->GetPlayerSpawnTransform(ControllerIndex, TargetPlayerCount, AuthoredSpawnTransform))
		{
			return AuthoredSpawnTransform;
		}
	}

	const FVector HallCenter = ActivePartyHall ? ActivePartyHall->GetHallBoundsCenter() : PartyHallSpawnLocation;
	const float AngleRadians = (2.0f * UE_PI * static_cast<float>(ControllerIndex)) / static_cast<float>(TargetPlayerCount);
	const FVector SpawnLocation = HallCenter + FVector(FMath::Cos(AngleRadians) * SpawnRadius, FMath::Sin(AngleRadians) * SpawnRadius, 120.0f);
	FRotator SpawnRotation = (HallCenter - SpawnLocation).Rotation();
	SpawnRotation.Pitch = 0.0f;
	SpawnRotation.Roll = 0.0f;

	return FTransform(SpawnRotation, SpawnLocation);
}

int32 AWizardStaffGameMode::GetControllerIndex(const AController* Controller) const
{
	if (!Controller)
	{
		return 0;
	}

	if (const AWizardStaffPlayerState* WizardPlayerState = Cast<AWizardStaffPlayerState>(Controller->PlayerState))
	{
		const int32 AssignedSlot = WizardPlayerState->GetWizardDisplaySlot();
		if (AssignedSlot >= 0)
		{
			return AssignedSlot;
		}
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		if (const APlayerState* PlayerState = Controller->PlayerState)
		{
			return FMath::Max(PlayerState->GetPlayerId(), 0);
		}
		return 0;
	}

	int32 Index = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get() == Controller)
		{
			return Index;
		}
		++Index;
	}

	if (const APlayerState* PlayerState = Controller->PlayerState)
	{
		return FMath::Max(PlayerState->GetPlayerId(), 0);
	}

	return 0;
}

void AWizardStaffGameMode::SpawnPrototypeArena()
{
	ActivePrototypeArena = nullptr;

	if (bUseAuthoredPrototypeArena)
	{
		ActivePrototypeArena = FindAuthoredPrototypeArena();
		if (ActivePrototypeArena)
		{
			ArenaHalfSize = ActivePrototypeArena->GetArenaHalfSize();
			UE_LOG(LogTemp, Log, TEXT("Using authored Wizard Staff prototype arena: %s"), *ActivePrototypeArena->GetName());
			return;
		}
	}

	if (!bSpawnPrototypeArena)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && RuntimePrototypeArenaClass)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = MakeUniqueObjectName(World, RuntimePrototypeArenaClass, TEXT("RuntimePrototypeArena"));
		ActivePrototypeArena = World->SpawnActor<AWizardStaffPrototypeArena>(RuntimePrototypeArenaClass, FTransform::Identity, SpawnParameters);
		if (ActivePrototypeArena)
		{
			ArenaHalfSize = ActivePrototypeArena->GetArenaHalfSize();
			UE_LOG(LogTemp, Log, TEXT("Spawned runtime Wizard Staff prototype arena fallback."));
			return;
		}
	}

	SpawnLegacyRuntimeArenaBlocks();
}

AWizardStaffPrototypeArena* AWizardStaffGameMode::FindAuthoredPrototypeArena() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AWizardStaffPrototypeArena> It(World); It; ++It)
	{
		AWizardStaffPrototypeArena* Arena = *It;
		if (IsValid(Arena))
		{
			return Arena;
		}
	}

	return nullptr;
}

void AWizardStaffGameMode::SpawnStaffsAtDawnArena()
{
	ActiveStaffsAtDawnArena = nullptr;

	if (bUseAuthoredStaffsAtDawnArena)
	{
		ActiveStaffsAtDawnArena = FindAuthoredStaffsAtDawnArena();
		if (ActiveStaffsAtDawnArena)
		{
			UE_LOG(LogTemp, Log, TEXT("Using authored Wizard Staff Staffs at Dawn arena: %s"), *ActiveStaffsAtDawnArena->GetName());
			return;
		}
	}

	if (!bSpawnStaffsAtDawnArena)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && RuntimeStaffsAtDawnArenaClass)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = MakeUniqueObjectName(World, RuntimeStaffsAtDawnArenaClass, TEXT("RuntimeStaffsAtDawnArena"));
		ActiveStaffsAtDawnArena = World->SpawnActor<AWizardStaffStaffsAtDawnArena>(
			RuntimeStaffsAtDawnArenaClass,
			FTransform(FRotator::ZeroRotator, RuntimeStaffsAtDawnArenaLocation),
			SpawnParameters);
		if (ActiveStaffsAtDawnArena)
		{
			UE_LOG(LogTemp, Log, TEXT("Spawned runtime Wizard Staff Staffs at Dawn arena fallback."));
		}
	}
}

AWizardStaffStaffsAtDawnArena* AWizardStaffGameMode::FindAuthoredStaffsAtDawnArena() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AWizardStaffStaffsAtDawnArena> It(World); It; ++It)
	{
		AWizardStaffStaffsAtDawnArena* Arena = *It;
		if (IsValid(Arena))
		{
			return Arena;
		}
	}

	return nullptr;
}

bool AWizardStaffGameMode::ShouldUseStaffsAtDawnArena() const
{
	return ActiveStaffsAtDawnArena
		&& !IsPartyHallActive()
		&& PartyMatchState != EWizardPartyMatchState::FinalRound
		&& ActiveTrialType == EWizardTrialType::StaffsAtDawn;
}

FString AWizardStaffGameMode::GetStaffsAtDawnArenaSourceText() const
{
	if (ActiveStaffsAtDawnArena)
	{
		return FString::Printf(TEXT("StaffsAtDawnArena %s"), *ActiveStaffsAtDawnArena->GetName());
	}

	if (ActivePrototypeArena)
	{
		return FString::Printf(TEXT("Prototype fallback %s"), *ActivePrototypeArena->GetName());
	}

	return TEXT("Radial fallback spawns");
}

int32 AWizardStaffGameMode::GetStaffsAtDawnArenaPlayerSpawnCount() const
{
	if (ActiveStaffsAtDawnArena)
	{
		return ActiveStaffsAtDawnArena->GetPlayerSpawnCount();
	}

	if (ActivePrototypeArena)
	{
		return ActivePrototypeArena->PlayerSpawnMarkers.Num();
	}

	return 0;
}

int32 AWizardStaffGameMode::GetStaffsAtDawnArenaFuturePowerupSpawnCount() const
{
	return ActiveStaffsAtDawnArena ? ActiveStaffsAtDawnArena->GetFuturePowerupSpawnCount() : 0;
}

void AWizardStaffGameMode::DrawStaffsAtDawnArenaDebug() const
{
	if (!StaffsAtDawnTuning.bShowArenaDebug
		|| ActiveTrialType != EWizardTrialType::StaffsAtDawn
		|| PartyMatchState == EWizardPartyMatchState::PartyHall
		|| PartyMatchState == EWizardPartyMatchState::Intermission
		|| PartyMatchState == EWizardPartyMatchState::FinalRound)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector BoundsCenter = GetCurrentPlayBoundsCenter();
	const float ArenaDebugHalfSize = GetCurrentPlayBoundsHalfSize();
	const float RingOutHalfSize = ArenaDebugHalfSize + FMath::Max(OutOfArenaRespawnTuning.HorizontalOutOfBoundsPadding, 0.0f);
	const FVector ArenaBoxCenter = BoundsCenter + FVector(0.0f, 0.0f, 72.0f);
	const FVector RingOutBoxCenter = BoundsCenter + FVector(0.0f, 0.0f, 104.0f);

	if (StaffsAtDawnTuning.bDrawArenaBoundsDebug)
	{
		DrawDebugBox(World, ArenaBoxCenter, FVector(ArenaDebugHalfSize, ArenaDebugHalfSize, 18.0f), FColor::Cyan, false, 0.0f, 0, 4.0f);
		DrawDebugString(World, ArenaBoxCenter + FVector(-ArenaDebugHalfSize, -ArenaDebugHalfSize, 32.0f), FString::Printf(TEXT("Arena half %.0f"), ArenaDebugHalfSize), nullptr, FColor::Cyan, 0.0f, true, 1.0f);
	}

	if (StaffsAtDawnTuning.bDrawRingOutBoundsDebug)
	{
		DrawDebugBox(World, RingOutBoxCenter, FVector(RingOutHalfSize, RingOutHalfSize, 18.0f), FColor::Orange, false, 0.0f, 0, 4.0f);
		DrawDebugString(World, RingOutBoxCenter + FVector(-RingOutHalfSize, RingOutHalfSize, 32.0f), FString::Printf(TEXT("Respawn bounds %.0f | FallZ %.0f"), RingOutHalfSize, GetCurrentOutOfArenaFallZThreshold()), nullptr, FColor::Orange, 0.0f, true, 1.0f);
	}

	if (StaffsAtDawnTuning.bDrawPlayerSpawnDebug)
	{
		TArray<FTransform> SpawnTransforms;
		if (ActiveStaffsAtDawnArena)
		{
			SpawnTransforms = ActiveStaffsAtDawnArena->GetPlayerSpawnTransforms();
		}
		else
		{
			const int32 TargetPlayerCount = GetDesiredLocalPlayerCountForSession();
			for (int32 PlayerIndex = 0; PlayerIndex < TargetPlayerCount; ++PlayerIndex)
			{
				FTransform SpawnTransform;
				if (ActivePrototypeArena && ActivePrototypeArena->GetPlayerSpawnTransform(PlayerIndex, TargetPlayerCount, SpawnTransform))
				{
					SpawnTransforms.Add(SpawnTransform);
					continue;
				}

				const float AngleRadians = (2.0f * UE_PI * static_cast<float>(PlayerIndex)) / static_cast<float>(TargetPlayerCount);
				const FVector SpawnLocation = BoundsCenter + FVector(FMath::Cos(AngleRadians) * SpawnRadius, FMath::Sin(AngleRadians) * SpawnRadius, 120.0f);
				FRotator SpawnRotation = (BoundsCenter - SpawnLocation).Rotation();
				SpawnRotation.Pitch = 0.0f;
				SpawnRotation.Roll = 0.0f;
				SpawnTransforms.Add(FTransform(SpawnRotation, SpawnLocation));
			}
		}

		for (int32 SpawnIndex = 0; SpawnIndex < SpawnTransforms.Num(); ++SpawnIndex)
		{
			const FTransform& SpawnTransform = SpawnTransforms[SpawnIndex];
			const FVector SpawnLocation = SpawnTransform.GetLocation();
			const FVector ArrowEnd = SpawnLocation + SpawnTransform.GetRotation().GetAxisX() * 95.0f;
			DrawDebugSphere(World, SpawnLocation, 34.0f, 12, FColor::Green, false, 0.0f, 0, 2.5f);
			DrawDebugDirectionalArrow(World, SpawnLocation, ArrowEnd, 28.0f, FColor::Green, false, 0.0f, 0, 3.0f);
			DrawDebugString(World, SpawnLocation + FVector(0.0f, 0.0f, 58.0f), FString::Printf(TEXT("P%d Spawn"), SpawnIndex + 1), nullptr, FColor::Green, 0.0f, true, 1.0f);
		}
	}

	if (StaffsAtDawnTuning.bDrawFuturePowerupSpawnDebug && ActiveStaffsAtDawnArena)
	{
		const TArray<FVector> PowerupSpawnLocations = ActiveStaffsAtDawnArena->GetFuturePowerupSpawnLocations();
		for (int32 SpawnIndex = 0; SpawnIndex < PowerupSpawnLocations.Num(); ++SpawnIndex)
		{
			const FVector SpawnLocation = PowerupSpawnLocations[SpawnIndex];
			DrawDebugSphere(World, SpawnLocation, 26.0f, 10, FColor::Purple, false, 0.0f, 0, 2.0f);
			DrawDebugLine(World, SpawnLocation + FVector(-38.0f, 0.0f, 0.0f), SpawnLocation + FVector(38.0f, 0.0f, 0.0f), FColor::Purple, false, 0.0f, 0, 2.0f);
			DrawDebugLine(World, SpawnLocation + FVector(0.0f, -38.0f, 0.0f), SpawnLocation + FVector(0.0f, 38.0f, 0.0f), FColor::Purple, false, 0.0f, 0, 2.0f);
			DrawDebugString(World, SpawnLocation + FVector(0.0f, 0.0f, 48.0f), FString::Printf(TEXT("Future Powerup %d"), SpawnIndex + 1), nullptr, FColor::Purple, 0.0f, true, 1.0f);
		}
	}
}

void AWizardStaffGameMode::SpawnPartyHall()
{
	ActivePartyHall = nullptr;

	if (bUseAuthoredPartyHall)
	{
		ActivePartyHall = FindAuthoredPartyHall();
		if (ActivePartyHall)
		{
			UE_LOG(LogTemp, Log, TEXT("Using authored Wizard Staff party hall: %s"), *ActivePartyHall->GetName());
			return;
		}
	}

	if (!bSpawnPartyHall)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && RuntimePartyHallClass)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = MakeUniqueObjectName(World, RuntimePartyHallClass, TEXT("RuntimePartyHall"));
		ActivePartyHall = World->SpawnActor<AWizardStaffPartyHall>(RuntimePartyHallClass, PartyHallSpawnLocation, FRotator::ZeroRotator, SpawnParameters);
		if (ActivePartyHall)
		{
			UE_LOG(LogTemp, Log, TEXT("Spawned runtime Wizard Staff party hall fallback."));
		}
	}
}

AWizardStaffPartyHall* AWizardStaffGameMode::FindAuthoredPartyHall() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AWizardStaffPartyHall> It(World); It; ++It)
	{
		AWizardStaffPartyHall* PartyHall = *It;
		if (IsValid(PartyHall))
		{
			return PartyHall;
		}
	}

	return nullptr;
}

void AWizardStaffGameMode::UpdatePartyHallSigns() const
{
	if (!ActivePartyHall)
	{
		return;
	}

	FString StandingsText = TEXT("STANDINGS\n#  RDY  W  F  S");
	const TArray<AWizardStaffWizardCharacter*> CurrentWizards = GetCurrentWizards();
	for (const AWizardStaffWizardCharacter* Wizard : CurrentWizards)
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		StandingsText += FString::Printf(
			TEXT("\nP%d  %s  %d  %d  %d"),
			PlayerIndex + 1,
			IsPartyHallPlayerReady(PlayerIndex) ? TEXT("YES") : TEXT("NO"),
			GetPlayerRoundWins(PlayerIndex),
			GetPlayerGrandWizardFavor(PlayerIndex),
			Wizard->GetStaffSegmentCount());
	}

	const int32 LeaderIndex = GetCurrentStandingLeaderPlayerIndex();
	const FString LeaderText = LeaderIndex == INDEX_NONE
		? TEXT("LEADER\nTie")
		: FString::Printf(TEXT("LEADER\nP%d"), LeaderIndex + 1);
	const FString ReadyLine = PartyHallTuning.bEnableReadyBell
		? FString::Printf(TEXT("\nReady %d/%d"), GetPartyHallReadyPlayerCount(), CurrentWizards.Num())
		: FString();
	const FString NextPhaseName = CompletedTrialCount >= FMath::Max(PartyMatchTuning.TrialsBeforeFinalRound, 1)
		? TEXT("Grand Wizard Final")
		: GetActiveTrialName();
	const FString CountdownText = FString::Printf(TEXT("NEXT TRIAL\n%s\n%.0fs%s"), *NextPhaseName, IntermissionRemainingTime, *ReadyLine);
	const FString PresetText = FString::Printf(TEXT("PRESET\n%s"), *GetActivePrototypeTuningPresetText());

	ActivePartyHall->UpdateIntermissionSigns(StandingsText, CountdownText, PresetText, LeaderText);
}

void AWizardStaffGameMode::ResetPartyHallReadyStates()
{
	const int32 PlayerCount = FMath::Max(GetDesiredLocalPlayerCountForSession(), GetCurrentWizards().Num());
	PartyHallReadyPlayers.SetNumZeroed(PlayerCount);
	for (uint8& bReady : PartyHallReadyPlayers)
	{
		bReady = 0;
	}

	PartyHallReadyFeedbackMessage.Reset();
	PartyHallReadyFeedbackExpireTime = 0.0f;
	bPartyHallAllReadyTriggered = false;

	if (ActivePartyHall)
	{
		ActivePartyHall->ResetReadyBellFeedback();
	}
	SyncAllReplicatedPlayerStates();
}

void AWizardStaffGameMode::EnsurePartyHallReadyStateSize(int32 MinimumPlayerCount)
{
	const int32 DesiredCount = FMath::Max(FMath::Max(GetDesiredLocalPlayerCountForSession(), MinimumPlayerCount), GetCurrentWizards().Num());
	if (PartyHallReadyPlayers.Num() >= DesiredCount)
	{
		return;
	}

	const int32 OldCount = PartyHallReadyPlayers.Num();
	PartyHallReadyPlayers.SetNumZeroed(DesiredCount);
	for (int32 Index = OldCount; Index < PartyHallReadyPlayers.Num(); ++Index)
	{
		PartyHallReadyPlayers[Index] = 0;
	}
}

bool AWizardStaffGameMode::AreAllActivePartyHallPlayersReady() const
{
	bool bSawActivePlayer = false;
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}

		bSawActivePlayer = true;
		if (!IsPartyHallPlayerReady(PlayerIndex))
		{
			return false;
		}
	}

	return bSawActivePlayer;
}

void AWizardStaffGameMode::SetPartyHallReadyFeedbackMessage(const FString& Message, const FColor& MessageColor)
{
	PartyHallReadyFeedbackMessage = Message;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	PartyHallReadyFeedbackExpireTime = Now + FMath::Max(PartyHallTuning.ReadyBellFeedbackDuration, 0.0f);
	AWizardStaffHUD::PushGameplayMessage(this, Message, MessageColor, PartyHallTuning.ReadyBellFeedbackDuration, EWizardHudMessageCategory::Gameplay);

	UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
	if (GEngine && PartyMatchTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4725, PartyHallTuning.ReadyBellFeedbackDuration, MessageColor, Message);
	}
}

void AWizardStaffGameMode::SpawnLegacyRuntimeArenaBlocks()
{
	const float WallThickness = 0.35f;
	const float WallHeight = 1.5f;
	const float FloorScale = (ArenaHalfSize * 2.0f) / 100.0f;
	const float WallLength = FloorScale;
	const float WallOffset = ArenaHalfSize + (WallThickness * 50.0f);

	SpawnArenaBlock(TEXT("PrototypeArenaFloor"), FVector(0.0f, 0.0f, -6.0f), FVector(FloorScale, FloorScale, 0.12f));
	SpawnArenaBlock(TEXT("PrototypeArenaNorthWall"), FVector(0.0f, WallOffset, 70.0f), FVector(WallLength, WallThickness, WallHeight));
	SpawnArenaBlock(TEXT("PrototypeArenaSouthWall"), FVector(0.0f, -WallOffset, 70.0f), FVector(WallLength, WallThickness, WallHeight));
	SpawnArenaBlock(TEXT("PrototypeArenaEastWall"), FVector(WallOffset, 0.0f, 70.0f), FVector(WallThickness, WallLength, WallHeight));
	SpawnArenaBlock(TEXT("PrototypeArenaWestWall"), FVector(-WallOffset, 0.0f, 70.0f), FVector(WallThickness, WallLength, WallHeight));

	SpawnArenaBlock(TEXT("PrototypeDoorwayLeftWall"), FVector(-500.0f, 180.0f, 70.0f), FVector(4.8f, 0.35f, 1.4f));
	SpawnArenaBlock(TEXT("PrototypeDoorwayRightWall"), FVector(500.0f, 180.0f, 70.0f), FVector(4.8f, 0.35f, 1.4f));
	SpawnArenaBlock(TEXT("PrototypeDoorwayLintel"), FVector(0.0f, 180.0f, 240.0f), FVector(2.3f, 0.35f, 0.35f));

	SpawnArenaBlock(TEXT("PrototypeTableA"), FVector(-360.0f, -320.0f, 38.0f), FVector(1.5f, 0.9f, 0.28f));
	SpawnArenaBlock(TEXT("PrototypeTableB"), FVector(390.0f, -230.0f, 38.0f), FVector(1.3f, 1.0f, 0.28f));
	SpawnArenaBlock(TEXT("PrototypeBlockPropA"), FVector(-90.0f, -540.0f, 55.0f), FVector(0.8f, 0.8f, 1.1f));
	SpawnArenaBlock(TEXT("PrototypeBlockPropB"), FVector(210.0f, 500.0f, 55.0f), FVector(0.9f, 0.65f, 1.1f));
}

void AWizardStaffGameMode::SpawnMugRunPickups()
{
	if (!MugRunTuning.ManaMugPickupClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TArray<FVector> SpawnLocations = GetDefaultMugSpawnLocations();
	if (SpawnLocations.Num() <= 0)
	{
		return;
	}

	const int32 SafeSpawnCount = FMath::Clamp(MugRunTuning.MugSpawnCount, 1, SpawnLocations.Num());
	for (int32 MugIndex = SpawnedMugs.Num() - 1; MugIndex >= 0; --MugIndex)
	{
		if (!IsValid(SpawnedMugs[MugIndex]))
		{
			SpawnedMugs.RemoveAt(MugIndex);
		}
	}

	for (int32 SpawnIndex = SpawnedMugs.Num(); SpawnIndex < SafeSpawnCount; ++SpawnIndex)
	{
		FActorSpawnParameters SpawnParameters;

		const FVector SpawnLocation = SpawnLocations[SpawnIndex] + FVector(0.0f, 0.0f, MugRunTuning.MugSpawnZ);
		AWizardStaffManaMugPickup* MugPickup = World->SpawnActor<AWizardStaffManaMugPickup>(MugRunTuning.ManaMugPickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
		if (MugPickup)
		{
			if (bHasAppliedPrototypeTuningPreset)
			{
				ApplyPrototypeTuningPresetToPickup(MugPickup, GetPrototypeTuningPresetValues(ActivePrototypeTuningPreset));
			}
			SpawnedMugs.Add(MugPickup);
		}
	}
}

void AWizardStaffGameMode::ResetMugRunStateForNewTrial()
{
	MugRunWinnerMessage.Reset();
	MugRunRemainingTime = MugRunTuning.MatchDuration;
	MugRunPostMatchRemainingTime = 0.0f;
	bMugRunMatchActive = false;
	MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;
	ResetMugRunPickups();
}

EWizardTrialType AWizardStaffGameMode::GetTrialTypeForTrialIndex(int32 TrialIndex) const
{
	switch (TrialIndex)
	{
	case 0:
		return EWizardTrialType::MugRun;
	case 1:
		return EWizardTrialType::StaffsAtDawn;
	default:
		return EWizardTrialType::CauldronCatastrophe;
	}
}

void AWizardStaffGameMode::StageWizardsForCurrentPhase()
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			RespawnWizardInArena(Wizard);
		}
	}
}

void AWizardStaffGameMode::SetWizardPrototypeInputsLocked(bool bLocked) const
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			Wizard->SetPrototypeInputLocked(bLocked);
		}
	}
}

void AWizardStaffGameMode::ResetWizardStaffsForTrialStart()
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			Wizard->ResetForNewMatch(true, true);
		}
	}
}

void AWizardStaffGameMode::ResetStaffsAtDawnForNewTrial()
{
	ResetStaffsAtDawnPowerups();
	StaffsAtDawnRemainingTime = FMath::Max(StaffsAtDawnTuning.TrialDuration, 0.0f);
	bStaffsAtDawnTrialActive = false;
	StaffsAtDawnScores.Reset();
	StaffsAtDawnBonksTowardSegment.Reset();
	EnsureStaffsAtDawnScoreSize(GetDesiredLocalPlayerCountForSession());
	ClearPendingOutOfArenaRespawns();
	RecentBonkAttackerPlayerIndexes.Reset();
	RecentBonkTimes.Reset();
	RecentBonkWasStaffClash.Reset();
	RecentStaffsAtDawnBroomBoostTimes.Reset();
	StaffsAtDawnBroomBoostRingOutThreats.Reset();
	StaffsAtDawnFeedbackMessage.Reset();
	StaffsAtDawnFeedbackExpireTime = 0.0f;
	MugRunWinnerMessage.Reset();
}

void AWizardStaffGameMode::InitializeStaffsAtDawnPowerups()
{
	ResetStaffsAtDawnPowerups();

	if (!StaffsAtDawnTuning.bEnableStaffsAtDawnPowerups
		|| StaffsAtDawnTuning.PowerupSpawnCount <= 0
		|| StaffsAtDawnTuning.DefaultPowerupType == EWizardStaffsAtDawnPowerupType::None
		|| !StaffsAtDawnPowerupPickupClass)
	{
		return;
	}

	TArray<FVector> CandidateLocations = GetStaffsAtDawnPowerupSpawnLocations();
	if (CandidateLocations.Num() <= 0)
	{
		return;
	}

	for (int32 LocationIndex = CandidateLocations.Num() - 1; LocationIndex > 0; --LocationIndex)
	{
		const int32 SwapIndex = FMath::RandRange(0, LocationIndex);
		CandidateLocations.Swap(LocationIndex, SwapIndex);
	}

	const int32 DesiredSpawnCount = FMath::Min(FMath::Max(StaffsAtDawnTuning.PowerupSpawnCount, 0), CandidateLocations.Num());
	for (int32 SpawnIndex = 0; SpawnIndex < DesiredSpawnCount; ++SpawnIndex)
	{
		StaffsAtDawnPowerupSpawnLocations.Add(CandidateLocations[SpawnIndex]);
		SpawnedStaffsAtDawnPowerups.Add(nullptr);
		StaffsAtDawnPowerupRespawnTimers.Add(FMath::Max(StaffsAtDawnTuning.InitialPowerupSpawnDelay, 0.0f));
	}

	if (StaffsAtDawnTuning.InitialPowerupSpawnDelay <= 0.0f)
	{
		for (int32 SpawnIndex = 0; SpawnIndex < StaffsAtDawnPowerupSpawnLocations.Num(); ++SpawnIndex)
		{
			SpawnStaffsAtDawnPowerupAtIndex(SpawnIndex);
		}
	}
}

void AWizardStaffGameMode::UpdateStaffsAtDawnPowerups(float DeltaSeconds)
{
	if (!IsStaffsAtDawnActive())
	{
		return;
	}

	if (!StaffsAtDawnTuning.bEnableStaffsAtDawnPowerups)
	{
		ResetStaffsAtDawnPowerups();
		return;
	}

	for (int32 SpawnIndex = 0; SpawnIndex < StaffsAtDawnPowerupRespawnTimers.Num(); ++SpawnIndex)
	{
		if (SpawnedStaffsAtDawnPowerups.IsValidIndex(SpawnIndex)
			&& IsValid(SpawnedStaffsAtDawnPowerups[SpawnIndex])
			&& SpawnedStaffsAtDawnPowerups[SpawnIndex]->IsPickupActive())
		{
			continue;
		}

		if (StaffsAtDawnPowerupRespawnTimers[SpawnIndex] < 0.0f)
		{
			continue;
		}

		StaffsAtDawnPowerupRespawnTimers[SpawnIndex] = FMath::Max(0.0f, StaffsAtDawnPowerupRespawnTimers[SpawnIndex] - DeltaSeconds);
		if (StaffsAtDawnPowerupRespawnTimers[SpawnIndex] <= 0.0f)
		{
			SpawnStaffsAtDawnPowerupAtIndex(SpawnIndex);
		}
	}
}

void AWizardStaffGameMode::ResetStaffsAtDawnPowerups()
{
	for (AWizardStaffStaffsAtDawnPowerupPickup* PowerupPickup : SpawnedStaffsAtDawnPowerups)
	{
		if (IsValid(PowerupPickup))
		{
			PowerupPickup->SetPickupActive(false);
			PowerupPickup->Destroy();
		}
	}

	SpawnedStaffsAtDawnPowerups.Reset();
	StaffsAtDawnPowerupSpawnLocations.Reset();
	StaffsAtDawnPowerupRespawnTimers.Reset();
}

void AWizardStaffGameMode::SpawnStaffsAtDawnPowerupAtIndex(int32 SpawnIndex)
{
	if (!IsStaffsAtDawnActive()
		|| !StaffsAtDawnPowerupPickupClass
		|| !StaffsAtDawnPowerupSpawnLocations.IsValidIndex(SpawnIndex)
		|| !SpawnedStaffsAtDawnPowerups.IsValidIndex(SpawnIndex))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation = ChooseStaffsAtDawnPowerupSpawnLocationForSlot(SpawnIndex);
	StaffsAtDawnPowerupSpawnLocations[SpawnIndex] = SpawnLocation;

	if (IsValid(SpawnedStaffsAtDawnPowerups[SpawnIndex]))
	{
		AWizardStaffStaffsAtDawnPowerupPickup* ExistingPowerup = SpawnedStaffsAtDawnPowerups[SpawnIndex];
		ExistingPowerup->SetActorLocation(SpawnLocation);
		ExistingPowerup->SetPowerupType(StaffsAtDawnTuning.DefaultPowerupType);
		ExistingPowerup->SetPickupActive(true);
		ExistingPowerup->ForceNetUpdate();
		StaffsAtDawnPowerupRespawnTimers[SpawnIndex] = -1.0f;

		UE_LOG(LogTemp, Log, TEXT("Reactivated Staffs at Dawn powerup %s at slot %d."), *GetStaffsAtDawnPowerupTypeText(StaffsAtDawnTuning.DefaultPowerupType), SpawnIndex + 1);
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWizardStaffStaffsAtDawnPowerupPickup* PowerupPickup = World->SpawnActor<AWizardStaffStaffsAtDawnPowerupPickup>(
		StaffsAtDawnPowerupPickupClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParameters);

	if (!PowerupPickup)
	{
		return;
	}

	PowerupPickup->SetPowerupType(StaffsAtDawnTuning.DefaultPowerupType);
	PowerupPickup->SetPickupActive(true);
	SpawnedStaffsAtDawnPowerups[SpawnIndex] = PowerupPickup;
	StaffsAtDawnPowerupRespawnTimers[SpawnIndex] = -1.0f;

	UE_LOG(LogTemp, Log, TEXT("Spawned Staffs at Dawn powerup %s at slot %d."), *GetStaffsAtDawnPowerupTypeText(StaffsAtDawnTuning.DefaultPowerupType), SpawnIndex + 1);
}

void AWizardStaffGameMode::RemoveStaffsAtDawnPowerupAtIndex(int32 SpawnIndex, bool bScheduleRespawn, float RespawnDelayOverride)
{
	if (!SpawnedStaffsAtDawnPowerups.IsValidIndex(SpawnIndex) || !StaffsAtDawnPowerupRespawnTimers.IsValidIndex(SpawnIndex))
	{
		return;
	}

	const bool bShouldRespawn = bScheduleRespawn
		&& IsStaffsAtDawnActive()
		&& StaffsAtDawnTuning.bRespawnPowerupsAfterPickup
		&& StaffsAtDawnTuning.bEnableStaffsAtDawnPowerups;
	const float RespawnDelay = RespawnDelayOverride >= 0.0f ? RespawnDelayOverride : StaffsAtDawnTuning.PowerupRespawnDelay;

	if (IsValid(SpawnedStaffsAtDawnPowerups[SpawnIndex]))
	{
		SpawnedStaffsAtDawnPowerups[SpawnIndex]->SetPickupActive(false);
		if (!bShouldRespawn)
		{
			SpawnedStaffsAtDawnPowerups[SpawnIndex]->Destroy();
			SpawnedStaffsAtDawnPowerups[SpawnIndex] = nullptr;
		}
	}
	else
	{
		SpawnedStaffsAtDawnPowerups[SpawnIndex] = nullptr;
	}

	StaffsAtDawnPowerupRespawnTimers[SpawnIndex] = bShouldRespawn ? FMath::Max(RespawnDelay, 0.0f) : -1.0f;
}

TArray<FVector> AWizardStaffGameMode::GetStaffsAtDawnPowerupSpawnLocations() const
{
	if (ActiveStaffsAtDawnArena)
	{
		const TArray<FVector> AuthoredLocations = ActiveStaffsAtDawnArena->GetFuturePowerupSpawnLocations();
		if (AuthoredLocations.Num() > 0)
		{
			return AuthoredLocations;
		}
	}

	TArray<FVector> FallbackLocations;
	const FVector Center = GetCurrentPlayBoundsCenter();
	const float Offset = FMath::Clamp(GetCurrentPlayBoundsHalfSize() * 0.35f, 220.0f, 720.0f);
	FallbackLocations.Add(Center + FVector(0.0f, 0.0f, 72.0f));
	FallbackLocations.Add(Center + FVector(Offset, 0.0f, 72.0f));
	FallbackLocations.Add(Center + FVector(0.0f, Offset, 72.0f));
	FallbackLocations.Add(Center + FVector(-Offset, 0.0f, 72.0f));
	FallbackLocations.Add(Center + FVector(0.0f, -Offset, 72.0f));
	return FallbackLocations;
}

FVector AWizardStaffGameMode::ChooseStaffsAtDawnPowerupSpawnLocationForSlot(int32 SpawnIndex) const
{
	const TArray<FVector> CandidateLocations = GetStaffsAtDawnPowerupSpawnLocations();
	if (CandidateLocations.Num() <= 0)
	{
		return StaffsAtDawnPowerupSpawnLocations.IsValidIndex(SpawnIndex)
			? StaffsAtDawnPowerupSpawnLocations[SpawnIndex]
			: GetCurrentPlayBoundsCenter() + FVector(0.0f, 0.0f, 72.0f);
	}

	constexpr float SameLocationToleranceSquared = 25.0f;
	const FVector PreviousLocation = StaffsAtDawnPowerupSpawnLocations.IsValidIndex(SpawnIndex)
		? StaffsAtDawnPowerupSpawnLocations[SpawnIndex]
		: FVector::ZeroVector;

	TArray<FVector> AvailableLocations;
	TArray<FVector> PreferredLocations;
	for (const FVector& CandidateLocation : CandidateLocations)
	{
		if (IsStaffsAtDawnPowerupLocationReserved(CandidateLocation, SpawnIndex))
		{
			continue;
		}

		AvailableLocations.Add(CandidateLocation);
		if (FVector::DistSquared(CandidateLocation, PreviousLocation) > SameLocationToleranceSquared)
		{
			PreferredLocations.Add(CandidateLocation);
		}
	}

	if (PreferredLocations.Num() > 0)
	{
		return PreferredLocations[FMath::RandRange(0, PreferredLocations.Num() - 1)];
	}
	if (AvailableLocations.Num() > 0)
	{
		return AvailableLocations[FMath::RandRange(0, AvailableLocations.Num() - 1)];
	}
	return CandidateLocations[FMath::RandRange(0, CandidateLocations.Num() - 1)];
}

bool AWizardStaffGameMode::IsStaffsAtDawnPowerupLocationReserved(const FVector& CandidateLocation, int32 IgnoredSpawnIndex) const
{
	constexpr float SameLocationToleranceSquared = 25.0f;
	for (int32 SpawnIndex = 0; SpawnIndex < StaffsAtDawnPowerupSpawnLocations.Num(); ++SpawnIndex)
	{
		if (SpawnIndex == IgnoredSpawnIndex)
		{
			continue;
		}

		const bool bHasActivePickup = SpawnedStaffsAtDawnPowerups.IsValidIndex(SpawnIndex) && IsValid(SpawnedStaffsAtDawnPowerups[SpawnIndex]);
		const bool bHasPendingRespawn = StaffsAtDawnPowerupRespawnTimers.IsValidIndex(SpawnIndex) && StaffsAtDawnPowerupRespawnTimers[SpawnIndex] >= 0.0f;
		if (!bHasActivePickup && !bHasPendingRespawn)
		{
			continue;
		}

		if (FVector::DistSquared(StaffsAtDawnPowerupSpawnLocations[SpawnIndex], CandidateLocation) <= SameLocationToleranceSquared)
		{
			return true;
		}
	}
	return false;
}

void AWizardStaffGameMode::HandleStaffsAtDawnPowerupPickedUp(AWizardStaffStaffsAtDawnPowerupPickup* Pickup, AWizardStaffWizardCharacter* Wizard)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsStaffsAtDawnActive() || !IsValid(Pickup) || !Pickup->IsPickupActive() || !Wizard)
	{
		return;
	}

	const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
	if (PlayerIndex == INDEX_NONE)
	{
		return;
	}

	int32 SpawnIndex = INDEX_NONE;
	for (int32 CandidateIndex = 0; CandidateIndex < SpawnedStaffsAtDawnPowerups.Num(); ++CandidateIndex)
	{
		if (SpawnedStaffsAtDawnPowerups[CandidateIndex].Get() == Pickup)
		{
			SpawnIndex = CandidateIndex;
			break;
		}
	}
	if (SpawnIndex == INDEX_NONE)
	{
		return;
	}

	const EWizardStaffsAtDawnPowerupType CollectedPowerupType = Pickup->GetPowerupType();
	const FString PowerupName = GetStaffsAtDawnPowerupTypeText(CollectedPowerupType);

	float RespawnDelayOverride = -1.0f;
	if (CollectedPowerupType == EWizardStaffsAtDawnPowerupType::MegaStaffBrew)
	{
		RespawnDelayOverride = StaffsAtDawnTuning.MegaStaffPickupRespawnDelay;
		const int32 GrantedSegments = Wizard->ActivateMegaStaffBrew(
			StaffsAtDawnTuning.MegaStaffBonusSegments,
			StaffsAtDawnTuning.MegaStaffDuration,
			StaffsAtDawnTuning.MegaStaffStressMultiplierDuringEffect,
			StaffsAtDawnTuning.MegaStaffKnockbackMultiplierDuringEffect,
			StaffsAtDawnTuning.bRemoveTemporarySegmentsOnExpire,
			StaffsAtDawnTuning.bShowMegaStaffDebug);
		RecordTelemetryMegaStaffPickup(Wizard, GrantedSegments);
		SetStaffsAtDawnFeedbackMessage(
			FString::Printf(TEXT("P%d drank Mega Staff Brew: +%d staff for %.0fs"), PlayerIndex + 1, GrantedSegments, StaffsAtDawnTuning.MegaStaffDuration),
			GrantedSegments > 0 ? FColor::Green : FColor::Orange);
		PublishReplicatedGameplayEvent(
			EWizardReplicatedGameplayEventType::MegaStaffGranted,
			FString::Printf(TEXT("P%d drank Mega Staff Brew"), PlayerIndex + 1),
			PlayerIndex,
			INDEX_NONE,
			static_cast<float>(GrantedSegments),
			false);
	}
	else
	{
		SetStaffsAtDawnFeedbackMessage(
			FString::Printf(TEXT("P%d grabbed %s (no effect yet)"), PlayerIndex + 1, *PowerupName),
			FColor::Green);
		PublishReplicatedGameplayEvent(
			EWizardReplicatedGameplayEventType::StaffsPowerupCollected,
			FString::Printf(TEXT("P%d grabbed %s"), PlayerIndex + 1, *PowerupName),
			PlayerIndex,
			INDEX_NONE,
			0.0f,
			false);
	}

	UE_LOG(LogTemp, Log, TEXT("Player %d collected Staffs at Dawn powerup %s."), PlayerIndex + 1, *PowerupName);

	RemoveStaffsAtDawnPowerupAtIndex(SpawnIndex, true, RespawnDelayOverride);
}

void AWizardStaffGameMode::EnsureStaffsAtDawnScoreSize(int32 MinimumPlayerCount)
{
	int32 DesiredSize = FMath::Max(MinimumPlayerCount, GetDesiredLocalPlayerCountForSession());
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != INDEX_NONE)
		{
			DesiredSize = FMath::Max(DesiredSize, PlayerIndex + 1);
		}
	}

	const int32 OldSize = StaffsAtDawnScores.Num();
	if (OldSize < DesiredSize)
	{
		StaffsAtDawnScores.SetNum(DesiredSize);
		for (int32 ScoreIndex = OldSize; ScoreIndex < StaffsAtDawnScores.Num(); ++ScoreIndex)
		{
			StaffsAtDawnScores[ScoreIndex] = 0;
		}
	}

	const int32 OldProgressSize = StaffsAtDawnBonksTowardSegment.Num();
	if (OldProgressSize < DesiredSize)
	{
		StaffsAtDawnBonksTowardSegment.SetNum(DesiredSize);
		for (int32 ProgressIndex = OldProgressSize; ProgressIndex < StaffsAtDawnBonksTowardSegment.Num(); ++ProgressIndex)
		{
			StaffsAtDawnBonksTowardSegment[ProgressIndex] = 0;
		}
	}
}

void AWizardStaffGameMode::AddStaffsAtDawnScore(int32 PlayerIndex, int32 Points, const FString& Reason)
{
	if (!IsStaffsAtDawnActive() || PlayerIndex == INDEX_NONE || Points == 0)
	{
		return;
	}

	EnsureStaffsAtDawnScoreSize(PlayerIndex + 1);
	if (!StaffsAtDawnScores.IsValidIndex(PlayerIndex))
	{
		return;
	}

	StaffsAtDawnScores[PlayerIndex] += Points;
	const FString FeedbackText = FString::Printf(
		TEXT("P%d +%d: %s  |  Score %d"),
		PlayerIndex + 1,
		Points,
		*Reason,
		StaffsAtDawnScores[PlayerIndex]);
	const FColor FeedbackColor = Reason.Contains(TEXT("Ring-out")) ? FColor::Red : FColor::Orange;
	SetStaffsAtDawnFeedbackMessage(FeedbackText, FeedbackColor);

	if (bEnablePlaytestTelemetry)
	{
		if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
		{
			Stats->StaffsAtDawnScore += Points;
		}
	}

	SyncReplicatedPlayerStateForIndex(PlayerIndex);
}

void AWizardStaffGameMode::AddStaffsAtDawnBonkSegmentProgress(int32 PlayerIndex)
{
	if (!IsStaffsAtDawnActive() || PlayerIndex == INDEX_NONE || StaffsAtDawnTuning.LandedBonksPerStaffSegment <= 0)
	{
		return;
	}

	EnsureStaffsAtDawnScoreSize(PlayerIndex + 1);
	if (!StaffsAtDawnBonksTowardSegment.IsValidIndex(PlayerIndex))
	{
		return;
	}

	++StaffsAtDawnBonksTowardSegment[PlayerIndex];
	const int32 SafeBonksPerSegment = FMath::Max(StaffsAtDawnTuning.LandedBonksPerStaffSegment, 1);
	if (StaffsAtDawnBonksTowardSegment[PlayerIndex] < SafeBonksPerSegment)
	{
		return;
	}

	const int32 SegmentsToGrant = StaffsAtDawnBonksTowardSegment[PlayerIndex] / SafeBonksPerSegment;
	StaffsAtDawnBonksTowardSegment[PlayerIndex] %= SafeBonksPerSegment;
	GrantStaffsAtDawnCombatSegments(PlayerIndex, SegmentsToGrant, TEXT("Landed Bonks"));
}

void AWizardStaffGameMode::GrantStaffsAtDawnCombatSegments(int32 PlayerIndex, int32 SegmentCount, const FString& Reason)
{
	if (!IsStaffsAtDawnActive() || PlayerIndex == INDEX_NONE || SegmentCount <= 0)
	{
		return;
	}

	AWizardStaffWizardCharacter* Wizard = GetWizardForPlayerIndex(PlayerIndex);
	if (!Wizard || !Wizard->StaffComponent)
	{
		return;
	}

	int32 GrantedSegments = 0;
	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const int32 PreviousCount = Wizard->StaffComponent->GetSegmentCount();
		const int32 NewCount = Wizard->StaffComponent->AddStaffSegment();
		if (NewCount <= PreviousCount)
		{
			break;
		}
		++GrantedSegments;
	}

	if (GrantedSegments <= 0)
	{
		return;
	}

	Wizard->AddManaSloshForStaffGrowth(GrantedSegments, FName(*Reason));

	if (bEnablePlaytestTelemetry)
	{
		if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
		{
			Stats->StaffsAtDawnCombatSegmentsGained += GrantedSegments;
		}
	}

	SetStaffsAtDawnFeedbackMessage(
		FString::Printf(TEXT("P%d staff grew +%d: %s"), PlayerIndex + 1, GrantedSegments, *Reason),
		FColor::Yellow);
	SyncReplicatedPlayerStateForIndex(PlayerIndex);
}

void AWizardStaffGameMode::SetStaffsAtDawnFeedbackMessage(const FString& Message, const FColor& MessageColor)
{
	StaffsAtDawnFeedbackMessage = Message;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	StaffsAtDawnFeedbackExpireTime = Now + FMath::Max(StaffsAtDawnTuning.ScoreFeedbackDuration, 0.0f);
	const EWizardHudMessageCategory Category = Message.Contains(TEXT("Mega Staff")) || Message.Contains(TEXT("powerup")) || Message.Contains(TEXT("Brew"))
		? EWizardHudMessageCategory::Powerup
		: EWizardHudMessageCategory::Scoring;
	AWizardStaffHUD::PushGameplayMessage(this, Message, MessageColor, StaffsAtDawnTuning.ScoreFeedbackDuration, Category);

	if (GEngine && StaffsAtDawnTuning.bShowDebug && !Message.IsEmpty() && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4815, FMath::Max(StaffsAtDawnTuning.ScoreFeedbackDuration, 0.75f), MessageColor, Message);
	}
}

void AWizardStaffGameMode::AnnounceStaffsAtDawnWinner()
{
	EnsureStaffsAtDawnScoreSize(GetDesiredLocalPlayerCountForSession());

	int32 BestScore = INDEX_NONE;
	TArray<int32> WinningPlayerIndexes;
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}

		const int32 Score = GetStaffsAtDawnScore(PlayerIndex);
		UE_LOG(LogTemp, Log, TEXT("Staffs at Dawn final score: Player %d score %d."), PlayerIndex + 1, Score);

		if (Score > BestScore)
		{
			BestScore = Score;
			WinningPlayerIndexes.Reset();
			WinningPlayerIndexes.Add(PlayerIndex);
		}
		else if (Score == BestScore)
		{
			WinningPlayerIndexes.Add(PlayerIndex);
		}
	}

	if (WinningPlayerIndexes.Num() == 0)
	{
		MugRunWinnerMessage = TEXT("Staffs at Dawn ended: no winner.");
	}
	else if (WinningPlayerIndexes.Num() == 1)
	{
		MugRunWinnerMessage = FString::Printf(TEXT("Staffs at Dawn Winner: P%d with %d points!"), WinningPlayerIndexes[0] + 1, BestScore);
	}
	else
	{
		MugRunWinnerMessage = FString::Printf(TEXT("Staffs at Dawn Tie at %d points:"), BestScore);
		for (const int32 PlayerIndex : WinningPlayerIndexes)
		{
			MugRunWinnerMessage += FString::Printf(TEXT(" P%d"), PlayerIndex + 1);
		}
	}

	for (const int32 PlayerIndex : WinningPlayerIndexes)
	{
		EnsurePlayerRoundWinsSize(PlayerIndex + 1);
		if (PlayerRoundWins.IsValidIndex(PlayerIndex))
		{
			++PlayerRoundWins[PlayerIndex];
		}
		const bool bTiedWin = WinningPlayerIndexes.Num() > 1;
		const int32 FavorAward = bTiedWin ? GrandWizardFavorTuning.TiedTrialWinnerFavor : GrandWizardFavorTuning.StaffsAtDawnWinnerFavor;
		AddGrandWizardFavor(PlayerIndex, FavorAward, bTiedWin ? TEXT("Staffs at Dawn Tie") : TEXT("Staffs at Dawn Winner"));
	}
	SyncPlaytestTelemetryRoundWins();
	SyncAllReplicatedPlayerStates();
	SetStaffsAtDawnFeedbackMessage(MugRunWinnerMessage, FColor::Yellow);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *MugRunWinnerMessage);
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4210, 8.0f, FColor::Yellow, MugRunWinnerMessage);
	}
}

void AWizardStaffGameMode::ResetWizardGameplayStateForMatchSetup()
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		Wizard->ResetForNewMatch(RematchTuning.bResetStaffsBetweenMatches, RematchTuning.bResetSloshBetweenMatches);
	}
}

void AWizardStaffGameMode::ResetMugRunPickups()
{
	UWorld* World = GetWorld();
	if (!World || !MugRunTuning.ManaMugPickupClass)
	{
		return;
	}

	const TArray<FVector> SpawnLocations = GetDefaultMugSpawnLocations();
	if (SpawnLocations.Num() <= 0)
	{
		SetMugRunPickupsActive(false);
		return;
	}

	const int32 SafeSpawnCount = FMath::Clamp(MugRunTuning.MugSpawnCount, 1, SpawnLocations.Num());
	for (int32 MugIndex = SpawnedMugs.Num() - 1; MugIndex >= 0; --MugIndex)
	{
		AWizardStaffManaMugPickup* MugPickup = SpawnedMugs[MugIndex];
		if (!IsValid(MugPickup))
		{
			SpawnedMugs.RemoveAt(MugIndex);
			continue;
		}

		if (MugIndex >= SafeSpawnCount)
		{
			MugPickup->Destroy();
			SpawnedMugs.RemoveAt(MugIndex);
		}
	}

	SpawnMugRunPickups();

	for (int32 MugIndex = 0; MugIndex < SpawnedMugs.Num(); ++MugIndex)
	{
		AWizardStaffManaMugPickup* MugPickup = SpawnedMugs[MugIndex];
		if (!MugPickup || !SpawnLocations.IsValidIndex(MugIndex))
		{
			continue;
		}

		MugPickup->SetActorLocation(SpawnLocations[MugIndex] + FVector(0.0f, 0.0f, MugRunTuning.MugSpawnZ));
		if (bHasAppliedPrototypeTuningPreset)
		{
			ApplyPrototypeTuningPresetToPickup(MugPickup, GetPrototypeTuningPresetValues(ActivePrototypeTuningPreset));
		}
		MugPickup->SetPickupActive(false);
	}
}

void AWizardStaffGameMode::CleanupArcanePinballProjectiles()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AWizardStaffArcanePinballProjectile> It(World); It; ++It)
	{
		if (AWizardStaffArcanePinballProjectile* Projectile = *It)
		{
			Projectile->Destroy();
		}
	}
}

void AWizardStaffGameMode::UpdateLooseSnappedSegments(float DeltaSeconds)
{
	if (LooseSnappedSegments.Num() <= 0)
	{
		return;
	}

	for (int32 SegmentIndex = LooseSnappedSegments.Num() - 1; SegmentIndex >= 0; --SegmentIndex)
	{
		FWizardTrackedLooseSegment& TrackedSegment = LooseSnappedSegments[SegmentIndex];
		AActor* LooseSegment = TrackedSegment.Actor.Get();
		if (!IsValid(LooseSegment))
		{
			LooseSnappedSegments.RemoveAt(SegmentIndex);
			continue;
		}

		TrackedSegment.Age += DeltaSeconds;
		TrackedSegment.ChaosCooldownRemaining = FMath::Max(0.0f, TrackedSegment.ChaosCooldownRemaining - DeltaSeconds);
		if (TrackedSegment.bIsFading)
		{
			const float SafeFadeDuration = FMath::Max(LooseSegmentChaosTuning.FadeOutDuration, 0.0f);
			if (SafeFadeDuration <= 0.0f)
			{
				RemoveLooseSnappedSegmentAt(SegmentIndex);
				continue;
			}

			TrackedSegment.FadeElapsed += DeltaSeconds;
			const float FadeAlpha = FMath::Clamp(1.0f - (TrackedSegment.FadeElapsed / SafeFadeDuration), 0.0f, 1.0f);
			LooseSegment->SetActorScale3D(TrackedSegment.InitialScale * FadeAlpha);
			if (TrackedSegment.FadeElapsed >= SafeFadeDuration)
			{
				RemoveLooseSnappedSegmentAt(SegmentIndex);
			}
			continue;
		}

		const bool bChaosEffectsEnabled = LooseSegmentChaosTuning.bEnableLooseSegmentChaosEffects;
		const float ChaosActiveDuration = FMath::Max(LooseSegmentChaosTuning.ChaosActiveDuration, 0.0f);
		if (bChaosEffectsEnabled && ChaosActiveDuration > 0.0f)
		{
			if (TrackedSegment.Age >= ChaosActiveDuration)
			{
				StartLooseSnappedSegmentFadeOrDestroy(SegmentIndex, false);
				continue;
			}

			const float PulseAlpha = 0.5f + (FMath::Sin((TrackedSegment.Age * 9.0f) + static_cast<float>(SegmentIndex)) * 0.5f);
			const float PulseScale = 1.0f + (PulseAlpha * 0.075f);
			LooseSegment->SetActorScale3D(TrackedSegment.InitialScale * PulseScale);
		}

		const float Lifetime = FMath::Max(LooseSegmentChaosTuning.LooseSegmentLifetime, 0.0f);
		if (Lifetime > 0.0f && TrackedSegment.Age >= Lifetime)
		{
			StartLooseSnappedSegmentFadeOrDestroy(SegmentIndex, false);
		}
	}

	EnforceLooseSnappedSegmentBudget();
}

void AWizardStaffGameMode::PruneLooseSnappedSegments()
{
	for (int32 SegmentIndex = LooseSnappedSegments.Num() - 1; SegmentIndex >= 0; --SegmentIndex)
	{
		if (!IsValid(LooseSnappedSegments[SegmentIndex].Actor.Get()))
		{
			LooseSnappedSegments.RemoveAt(SegmentIndex);
		}
	}
}

void AWizardStaffGameMode::EnforceLooseSnappedSegmentBudget()
{
	PruneLooseSnappedSegments();

	const int32 MaxLooseSegments = FMath::Max(LooseSegmentChaosTuning.MaxLooseSegments, 0);
	while (LooseSnappedSegments.Num() > MaxLooseSegments)
	{
		int32 OldestIndex = INDEX_NONE;
		float OldestAge = -1.0f;
		for (int32 SegmentIndex = 0; SegmentIndex < LooseSnappedSegments.Num(); ++SegmentIndex)
		{
			if (LooseSnappedSegments[SegmentIndex].Age > OldestAge)
			{
				OldestAge = LooseSnappedSegments[SegmentIndex].Age;
				OldestIndex = SegmentIndex;
			}
		}

		if (OldestIndex == INDEX_NONE)
		{
			return;
		}

		RemoveLooseSnappedSegmentAt(OldestIndex);
	}
}

void AWizardStaffGameMode::RemoveLooseSnappedSegmentAt(int32 SegmentIndex)
{
	if (!LooseSnappedSegments.IsValidIndex(SegmentIndex))
	{
		return;
	}

	if (AActor* LooseSegment = LooseSnappedSegments[SegmentIndex].Actor.Get())
	{
		LooseSegment->Destroy();
	}
	LooseSnappedSegments.RemoveAt(SegmentIndex);
}

void AWizardStaffGameMode::StartLooseSnappedSegmentFadeOrDestroy(int32 SegmentIndex, bool bImmediateDestroy)
{
	if (!LooseSnappedSegments.IsValidIndex(SegmentIndex))
	{
		return;
	}

	FWizardTrackedLooseSegment& TrackedSegment = LooseSnappedSegments[SegmentIndex];
	AActor* LooseSegment = TrackedSegment.Actor.Get();
	if (!IsValid(LooseSegment))
	{
		LooseSnappedSegments.RemoveAt(SegmentIndex);
		return;
	}

	if (bImmediateDestroy || LooseSegmentChaosTuning.FadeOutDuration <= 0.0f)
	{
		RemoveLooseSnappedSegmentAt(SegmentIndex);
		return;
	}

	TrackedSegment.bIsFading = true;
	TrackedSegment.FadeElapsed = 0.0f;
	TrackedSegment.InitialScale = LooseSegment->GetActorScale3D();

	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(LooseSegment->GetRootComponent()))
	{
		RootPrimitive->SetSimulatePhysics(false);
		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWizardStaffGameMode::HandleLooseSegmentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!LooseSegmentChaosTuning.bEnableLooseSegmentChaosEffects || !HitComponent || !OtherActor || OtherActor == HitComponent->GetOwner())
	{
		return;
	}

	const int32 SegmentIndex = FindTrackedLooseSegmentIndex(HitComponent->GetOwner(), HitComponent);
	if (!LooseSnappedSegments.IsValidIndex(SegmentIndex))
	{
		return;
	}

	FWizardTrackedLooseSegment& TrackedSegment = LooseSnappedSegments[SegmentIndex];
	if (TrackedSegment.bIsFading
		|| TrackedSegment.ChaosCooldownRemaining > 0.0f
		|| TrackedSegment.Age > FMath::Max(LooseSegmentChaosTuning.ChaosActiveDuration, 0.0f))
	{
		return;
	}

	const float ImpactSpeed = GetLooseSegmentImpactSpeed(TrackedSegment, NormalImpulse);
	if (ImpactSpeed < FMath::Max(LooseSegmentChaosTuning.MinImpactSpeedForChaosEffect, 0.0f))
	{
		return;
	}

	FVector ImpactDirection = HitComponent->GetPhysicsLinearVelocity();
	ImpactDirection.Z = 0.0f;
	if (!ImpactDirection.Normalize())
	{
		ImpactDirection = Hit.ImpactNormal * -1.0f;
		ImpactDirection.Z = 0.0f;
		ImpactDirection.Normalize();
	}
	if (ImpactDirection.IsNearlyZero())
	{
		ImpactDirection = FVector::ForwardVector;
	}

	bool bDidTrigger = false;
	if (AWizardStaffWizardCharacter* HitWizard = Cast<AWizardStaffWizardCharacter>(OtherActor))
	{
		const EWizardLooseSegmentChaosEffectType EffectType = ChooseLooseSegmentChaosEffect(ImpactSpeed, true);
		bDidTrigger = ApplyLooseSegmentChaosEffect(SegmentIndex, HitWizard, EffectType, Hit.ImpactPoint, ImpactDirection);
	}
	else if (ImpactSpeed >= FMath::Max(LooseSegmentChaosTuning.MinImpactSpeedForChaosEffect * 3.0f, LooseSegmentChaosTuning.MinImpactSpeedForChaosEffect))
	{
		bDidTrigger = TriggerLooseSegmentArcanePop(SegmentIndex, Hit.ImpactPoint, ImpactDirection);
	}

	if (bDidTrigger && LooseSnappedSegments.IsValidIndex(SegmentIndex))
	{
		LooseSnappedSegments[SegmentIndex].ChaosCooldownRemaining = FMath::Max(LooseSegmentChaosTuning.ChaosTriggerCooldown, 0.0f);
	}
}

int32 AWizardStaffGameMode::FindTrackedLooseSegmentIndex(AActor* LooseSegment, UPrimitiveComponent* HitComponent) const
{
	for (int32 SegmentIndex = 0; SegmentIndex < LooseSnappedSegments.Num(); ++SegmentIndex)
	{
		const FWizardTrackedLooseSegment& TrackedSegment = LooseSnappedSegments[SegmentIndex];
		if ((LooseSegment && TrackedSegment.Actor.Get() == LooseSegment)
			|| (HitComponent && TrackedSegment.CollisionComponent.Get() == HitComponent))
		{
			return SegmentIndex;
		}
	}

	return INDEX_NONE;
}

float AWizardStaffGameMode::GetLooseSegmentImpactSpeed(const FWizardTrackedLooseSegment& TrackedSegment, const FVector& NormalImpulse) const
{
	float ImpactSpeed = 0.0f;
	if (UPrimitiveComponent* CollisionComponent = TrackedSegment.CollisionComponent.Get())
	{
		ImpactSpeed = CollisionComponent->GetPhysicsLinearVelocity().Size();
	}

	return FMath::Max(ImpactSpeed, NormalImpulse.Size() * 0.01f);
}

EWizardLooseSegmentChaosEffectType AWizardStaffGameMode::ChooseLooseSegmentChaosEffect(float ImpactSpeed, bool bHitWizard) const
{
	const float MinSpeed = FMath::Max(LooseSegmentChaosTuning.MinImpactSpeedForChaosEffect, 1.0f);
	if (ImpactSpeed >= MinSpeed * 3.0f)
	{
		return EWizardLooseSegmentChaosEffectType::ArcanePop;
	}

	if (bHitWizard && ImpactSpeed >= MinSpeed * 1.35f)
	{
		return EWizardLooseSegmentChaosEffectType::TripBonk;
	}

	return bHitWizard ? EWizardLooseSegmentChaosEffectType::ManaSplash : EWizardLooseSegmentChaosEffectType::None;
}

bool AWizardStaffGameMode::CanLooseSegmentAffectWizard(const FWizardTrackedLooseSegment& TrackedSegment, const AWizardStaffWizardCharacter* Wizard) const
{
	if (!Wizard)
	{
		return false;
	}

	const AWizardStaffWizardCharacter* SourceWizard = TrackedSegment.SourceWizard.Get();
	if (SourceWizard && Wizard == SourceWizard)
	{
		return LooseSegmentChaosTuning.bAffectOwner;
	}

	return LooseSegmentChaosTuning.bAffectOtherPlayers;
}

bool AWizardStaffGameMode::ApplyLooseSegmentChaosEffect(int32 SegmentIndex, AWizardStaffWizardCharacter* HitWizard, EWizardLooseSegmentChaosEffectType EffectType, const FVector& ImpactLocation, const FVector& ImpactDirection)
{
	if (!LooseSnappedSegments.IsValidIndex(SegmentIndex) || !CanLooseSegmentAffectWizard(LooseSnappedSegments[SegmentIndex], HitWizard))
	{
		return false;
	}

	float ManaSloshAdded = 0.0f;
	switch (EffectType)
	{
	case EWizardLooseSegmentChaosEffectType::ManaSplash:
		ManaSloshAdded = FMath::Max(LooseSegmentChaosTuning.ManaSplashSloshAmount, 0.0f);
		HitWizard->AddManaSlosh(ManaSloshAdded);
		RecordTelemetryLooseSegmentChaosHit(HitWizard, EffectType, ManaSloshAdded);
		break;
	case EWizardLooseSegmentChaosEffectType::TripBonk:
		HitWizard->ApplyBonkReaction(ImpactDirection, LooseSegmentChaosTuning.TripBonkKnockback, LooseSegmentChaosTuning.TripBonkUpwardBoost, 0);
		RecordTelemetryLooseSegmentChaosHit(HitWizard, EffectType, 0.0f);
		AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("P%d tripped on unstable staff magic"), GetPlayerIndexForWizard(HitWizard) + 1), FColor::Cyan, 1.8f, EWizardHudMessageCategory::Gameplay);
		break;
	case EWizardLooseSegmentChaosEffectType::ArcanePop:
		return TriggerLooseSegmentArcanePop(SegmentIndex, ImpactLocation, ImpactDirection);
	case EWizardLooseSegmentChaosEffectType::None:
	default:
		return false;
	}

	if (LooseSegmentChaosTuning.bShowLooseSegmentChaosDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Loose segment chaos hit P%d with effect %d."), GetPlayerIndexForWizard(HitWizard) + 1, static_cast<int32>(EffectType));
		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(World, ImpactLocation, 36.0f, 12, EffectType == EWizardLooseSegmentChaosEffectType::ManaSplash ? FColor::Cyan : FColor::Orange, false, 0.6f, 0, 2.0f);
		}
	}

	return true;
}

bool AWizardStaffGameMode::TriggerLooseSegmentArcanePop(int32 SegmentIndex, const FVector& ImpactLocation, const FVector& ImpactDirection)
{
	if (!LooseSnappedSegments.IsValidIndex(SegmentIndex))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FWizardTrackedLooseSegment& TrackedSegment = LooseSnappedSegments[SegmentIndex];
	const float Radius = FMath::Max(LooseSegmentChaosTuning.ArcanePopRadius, 0.0f);
	if (Radius <= 0.0f)
	{
		return false;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LooseSegmentArcanePop), false);
	if (AActor* LooseActor = TrackedSegment.Actor.Get())
	{
		QueryParams.AddIgnoredActor(LooseActor);
	}
	World->OverlapMultiByObjectType(Overlaps, ImpactLocation, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Radius), QueryParams);

	int32 AffectedWizards = 0;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(Overlap.GetActor());
		if (!CanLooseSegmentAffectWizard(TrackedSegment, Wizard))
		{
			continue;
		}

		FVector KnockbackDirection = Wizard->GetActorLocation() - ImpactLocation;
		KnockbackDirection.Z = 0.0f;
		if (!KnockbackDirection.Normalize())
		{
			KnockbackDirection = ImpactDirection;
		}

		Wizard->ApplyBonkReaction(KnockbackDirection, LooseSegmentChaosTuning.ArcanePopKnockback, LooseSegmentChaosTuning.ArcanePopUpwardBoost, 0);
		RecordTelemetryLooseSegmentChaosHit(Wizard, EWizardLooseSegmentChaosEffectType::ArcanePop, 0.0f);
		++AffectedWizards;
	}

	if (LooseSegmentChaosTuning.bShowLooseSegmentChaosDebug)
	{
		DrawDebugSphere(World, ImpactLocation, Radius, 18, FColor::Magenta, false, 0.8f, 0, 2.5f);
		UE_LOG(LogTemp, Log, TEXT("Loose segment Arcane Pop affected %d wizard(s)."), AffectedWizards);
	}

	if (AffectedWizards > 0)
	{
		AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("Loose staff Arcane Pop hit %d wizard%s"), AffectedWizards, AffectedWizards == 1 ? TEXT("") : TEXT("s")), FColor::Magenta, 1.8f, EWizardHudMessageCategory::Gameplay);
	}

	return true;
}

void AWizardStaffGameMode::SetMugRunPickupsActive(bool bNewActive)
{
	for (AWizardStaffManaMugPickup* MugPickup : SpawnedMugs)
	{
		if (IsValid(MugPickup))
		{
			MugPickup->SetPickupActive(bNewActive);
		}
	}
}

TArray<FVector> AWizardStaffGameMode::GetDefaultMugSpawnLocations() const
{
	if (ActivePrototypeArena)
	{
		const TArray<FVector> AuthoredSpawnLocations = ActivePrototypeArena->GetMugSpawnLocations();
		if (AuthoredSpawnLocations.Num() > 0)
		{
			return AuthoredSpawnLocations;
		}
	}

	const FVector ArenaCenter = GetArenaCenter();
	return
	{
		ArenaCenter + FVector(-700.0f, -650.0f, 0.0f),
		ArenaCenter + FVector(700.0f, -650.0f, 0.0f),
		ArenaCenter + FVector(-700.0f, 650.0f, 0.0f),
		ArenaCenter + FVector(700.0f, 650.0f, 0.0f),
		ArenaCenter + FVector(-220.0f, 0.0f, 0.0f),
		ArenaCenter + FVector(220.0f, 0.0f, 0.0f),
		ArenaCenter + FVector(-620.0f, 120.0f, 0.0f),
		ArenaCenter + FVector(620.0f, 120.0f, 0.0f),
		ArenaCenter + FVector(-320.0f, -520.0f, 0.0f),
		ArenaCenter + FVector(360.0f, 470.0f, 0.0f),
		ArenaCenter + FVector(0.0f, -760.0f, 0.0f),
		ArenaCenter + FVector(0.0f, 760.0f, 0.0f)
	};
}

TArray<AWizardStaffWizardCharacter*> AWizardStaffGameMode::GetCurrentWizards() const
{
	TArray<AWizardStaffWizardCharacter*> Wizards;

	const UWorld* World = GetWorld();
	if (!World)
	{
		return Wizards;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (!PlayerController)
		{
			continue;
		}

		AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(PlayerController->GetPawn());
		if (Wizard)
		{
			Wizards.Add(Wizard);
		}
	}

	return Wizards;
}

AWizardStaffWizardCharacter* AWizardStaffGameMode::GetWizardForPlayerIndex(int32 PlayerIndex) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const AWizardStaffPlayerState* WizardPlayerState = PlayerController
			? Cast<AWizardStaffPlayerState>(PlayerController->PlayerState)
			: nullptr;
		if (WizardPlayerState && WizardPlayerState->GetWizardDisplaySlot() == PlayerIndex)
		{
			return Cast<AWizardStaffWizardCharacter>(PlayerController->GetPawn());
		}
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
		const AWizardStaffPlayerState* WizardPlayerState = Cast<AWizardStaffPlayerState>(PlayerState);
		if (WizardPlayerState && WizardPlayerState->GetWizardDisplaySlot() >= 0)
		{
			continue;
		}
		const int32 CurrentPlayerIndex = PlayerState && PlayerState->GetPlayerId() >= 0
			? PlayerState->GetPlayerId()
			: INDEX_NONE;
		if (CurrentPlayerIndex == PlayerIndex)
		{
			return Cast<AWizardStaffWizardCharacter>(PlayerController->GetPawn());
		}
	}

	int32 ControllerIndex = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const AWizardStaffPlayerState* WizardPlayerState = PlayerController
			? Cast<AWizardStaffPlayerState>(PlayerController->PlayerState)
			: nullptr;
		const bool bHasAssignedSlot = WizardPlayerState && WizardPlayerState->GetWizardDisplaySlot() >= 0;
		if (PlayerController && !bHasAssignedSlot && ControllerIndex == PlayerIndex)
		{
			return Cast<AWizardStaffWizardCharacter>(PlayerController->GetPawn());
		}
		++ControllerIndex;
	}

	return nullptr;
}

const FWizardPrototypeTuningPresetValues& AWizardStaffGameMode::GetPrototypeTuningPresetValues(EWizardPrototypeTuningPreset Preset) const
{
	switch (Preset)
	{
	case EWizardPrototypeTuningPreset::Stable:
		return StableTuningPreset;
	case EWizardPrototypeTuningPreset::Absurd:
		return AbsurdTuningPreset;
	case EWizardPrototypeTuningPreset::Chaotic:
	default:
		return ChaoticTuningPreset;
	}
}

void AWizardStaffGameMode::ApplyPrototypeTuningPresetValues(const FWizardPrototypeTuningPresetValues& PresetValues)
{
	MugRunTuning.MatchDuration = PresetValues.MatchDuration;
	MugRunTuning.MugSpawnCount = PresetValues.MugSpawnCount;
	MugRunTuning.BrewRewardChance = FMath::Clamp(PresetValues.BrewRewardChance, 0.0f, 1.0f);

	LooseSegmentChaosTuning.LooseSegmentLifetime = PresetValues.LooseSegmentLifetime;
	LooseSegmentChaosTuning.MaxLooseSegments = PresetValues.MaxLooseSegments;
	LooseSegmentChaosTuning.FadeOutDuration = PresetValues.LooseSegmentFadeOutDuration;

	OutOfArenaRespawnTuning.RespawnDelay = PresetValues.OutOfArenaRespawnDelay;
	OutOfArenaRespawnTuning.HorizontalOutOfBoundsPadding = PresetValues.HorizontalOutOfBoundsPadding;
	OutOfArenaRespawnTuning.FallZThreshold = PresetValues.FallZThreshold;

	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		ApplyPrototypeTuningPresetToWizard(Wizard, PresetValues);
	}

	for (AWizardStaffManaMugPickup* MugPickup : SpawnedMugs)
	{
		ApplyPrototypeTuningPresetToPickup(MugPickup, PresetValues);
	}
}

void AWizardStaffGameMode::ApplyPrototypeTuningPresetToWizard(AWizardStaffWizardCharacter* Wizard, const FWizardPrototypeTuningPresetValues& PresetValues) const
{
	if (!Wizard)
	{
		return;
	}

	Wizard->ManaTuning.SloshGainedPerDrink = PresetValues.SloshGainedPerStaffSegment;
	Wizard->ManaTuning.SloshGainedPerStaffSegment = PresetValues.SloshGainedPerStaffSegment;
	Wizard->ManaTuning.SloshReducedOnStaffSnap = PresetValues.SloshReducedOnStaffSnap;
	Wizard->ManaTuning.SloshDecayPerSecond = PresetValues.SloshDecayPerSecond;
	Wizard->ManaTuning.TurnPenaltyPerSlosh = PresetValues.TurnPenaltyPerSlosh;
	Wizard->ManaTuning.MovementPenaltyPerSlosh = PresetValues.MovementPenaltyPerSlosh;
	Wizard->ManaTuning.AccelerationPenaltyPerSlosh = PresetValues.AccelerationPenaltyPerSlosh;
	Wizard->ManaTuning.BrakingPenaltyPerSlosh = PresetValues.BrakingPenaltyPerSlosh;
	Wizard->ManaTuning.MinMovementMultiplier = PresetValues.MinMovementMultiplier;
	Wizard->ManaTuning.MinTurnMultiplier = PresetValues.MinTurnMultiplier;
	Wizard->ManaTuning.MinAccelerationMultiplier = PresetValues.MinAccelerationMultiplier;
	Wizard->ManaTuning.MinBrakingMultiplier = PresetValues.MinBrakingMultiplier;
	Wizard->ManaTuning.SloshOversteerDegreesPerSecond = PresetValues.SloshOversteerDegreesPerSecond;
	Wizard->ManaTuning.StumbleChancePerSecondAtMaxSlosh = PresetValues.StumbleChancePerSecondAtMaxSlosh;
	Wizard->ManaTuning.SloshVisualLeanDegrees = PresetValues.SloshVisualLeanDegrees;
	Wizard->ManaTuning.SloshStaffWobbleDegrees = PresetValues.SloshStaffWobbleDegrees;

	if (PresetValues.bSetSloshOnApply)
	{
		Wizard->ManaSlosh = FMath::Clamp(PresetValues.SloshAlphaOnApply, 0.0f, 1.0f) * FMath::Max(Wizard->ManaTuning.MaxSlosh, 1.0f);
	}
	else
	{
		Wizard->ManaSlosh = FMath::Clamp(Wizard->ManaSlosh, 0.0f, FMath::Max(Wizard->ManaTuning.MaxSlosh, 1.0f));
	}

	Wizard->StaffHeftTuning.SegmentCountBeforeHeftPenalty = PresetValues.SegmentCountBeforeHeftPenalty;
	Wizard->StaffHeftTuning.MovementPenaltyPerHeavySegment = PresetValues.MovementPenaltyPerHeavySegment;
	Wizard->StaffHeftTuning.TurnPenaltyPerHeavySegment = PresetValues.TurnPenaltyPerHeavySegment;
	Wizard->StaffHeftTuning.MaxMovementPenalty = PresetValues.MaxMovementPenalty;
	Wizard->StaffHeftTuning.MaxTurnPenalty = PresetValues.MaxTurnPenalty;
	Wizard->StaffHeftTuning.BonkCooldownPerHeavySegment = PresetValues.BonkCooldownPerHeavySegment;
	Wizard->StaffHeftTuning.BonkVisualDurationPerHeavySegment = PresetValues.BonkVisualDurationPerHeavySegment;
	Wizard->StaffHeftTuning.MaxBonkCooldownBonus = PresetValues.MaxBonkCooldownBonus;
	Wizard->StaffHeftTuning.MaxBonkVisualDurationBonus = PresetValues.MaxBonkVisualDurationBonus;

	Wizard->BonkTuning.KnockbackStrength = PresetValues.BonkKnockbackStrength;
	Wizard->BonkTuning.KnockbackPerManaSlosh = PresetValues.BonkKnockbackPerManaSlosh;
	Wizard->BonkTuning.KnockbackPerStaffSegment = PresetValues.BonkKnockbackPerStaffSegment;
	Wizard->BonkTuning.MaxKnockbackStrength = PresetValues.BonkMaxKnockbackStrength;
	Wizard->BonkTuning.UpwardBoost = PresetValues.BonkUpwardBoost;
	Wizard->BonkTuning.Cooldown = PresetValues.BonkCooldown;
	Wizard->BonkTuning.VisualDuration = PresetValues.BonkVisualDuration;
	Wizard->BonkTuning.StrikeEaseExponent = PresetValues.BonkStrikeEaseExponent;
	Wizard->BonkTuning.StaffContactPadding = PresetValues.StaffContactPadding;
	Wizard->BonkTuning.HitStressMultiplier = PresetValues.BonkHitStressMultiplier;
	Wizard->BonkTuning.WhiffStressMultiplier = PresetValues.BonkWhiffStressMultiplier;

	Wizard->ArcanePinballTuning.ProjectileSpeed = PresetValues.ArcanePinballProjectileSpeed;
	Wizard->ArcanePinballTuning.SpeedMultiplierPerWallBounce = PresetValues.ArcanePinballSpeedMultiplierPerWallBounce;
	Wizard->ArcanePinballTuning.MaxProjectileSpeed = PresetValues.ArcanePinballMaxProjectileSpeed;
	Wizard->ArcanePinballTuning.MaxBounces = PresetValues.ArcanePinballMaxBounces;
	Wizard->ArcanePinballTuning.Lifetime = PresetValues.ArcanePinballLifetime;
	Wizard->ArcanePinballTuning.HitKnockback = PresetValues.ArcanePinballHitKnockback;
	Wizard->ArcanePinballTuning.SloshOnHit = PresetValues.ArcanePinballSloshOnHit;
	Wizard->ArcanePinballTuning.StressOnCast = PresetValues.ArcanePinballStressOnCast;
	Wizard->ArcanePinballTuning.StressOnHit = PresetValues.ArcanePinballStressOnHit;
	Wizard->ArcanePinballTuning.bAllowSelfHit = PresetValues.bArcanePinballAllowSelfHit;
	Wizard->ArcanePinballTuning.bDestroyOnPlayerHit = PresetValues.bArcanePinballDestroyOnPlayerHit;

	if (Wizard->StaffComponent)
	{
		Wizard->StaffComponent->CollisionTuning.CollisionLengthPerSegment = PresetValues.CollisionLengthPerSegment;
		Wizard->StaffComponent->CollisionTuning.CollisionThickness = PresetValues.CollisionThickness;
		Wizard->StaffComponent->CollisionTuning.ObstructedControlMultiplier = PresetValues.ObstructedControlMultiplier;
		Wizard->StaffComponent->CollisionTuning.ObstructionRecoverySpeed = PresetValues.ObstructionRecoverySpeed;

		Wizard->StaffComponent->StuckTuning.StuckTimeBeforeStressBoost = PresetValues.StuckTimeBeforeStressBoost;
		Wizard->StaffComponent->StuckTuning.StuckStressPerSecond = PresetValues.StuckStressPerSecond;
		Wizard->StaffComponent->StuckTuning.StuckTimeBeforeCollisionRelief = PresetValues.StuckTimeBeforeCollisionRelief;
		Wizard->StaffComponent->StuckTuning.CollisionReliefDuration = PresetValues.CollisionReliefDuration;
		Wizard->StaffComponent->StuckTuning.CollisionReliefControlMultiplier = PresetValues.CollisionReliefControlMultiplier;
		Wizard->StaffComponent->StuckTuning.GentleNudgeDistance = PresetValues.GentleNudgeDistance;

		Wizard->StaffComponent->StressTuning.MaxStaffStress = PresetValues.MaxStaffStress;
		Wizard->StaffComponent->StressTuning.StressGainedPerBonk = PresetValues.StressGainedPerBonk;
		Wizard->StaffComponent->StressTuning.StressGainedPerWallImpact = PresetValues.StressGainedPerWallImpact;
		Wizard->StaffComponent->StressTuning.StressMultiplierPerSegment = PresetValues.StressMultiplierPerSegment;
		Wizard->StaffComponent->StressTuning.StressDecayRate = PresetValues.StressDecayRate;
		Wizard->StaffComponent->StressTuning.CaughtStressPerSecond = PresetValues.CaughtStressPerSecond;
		Wizard->StaffComponent->StressTuning.SnapImpulseForce = PresetValues.SnapImpulseForce;
		Wizard->StaffComponent->StressTuning.SnapUpwardImpulse = PresetValues.SnapUpwardImpulse;
		Wizard->StaffComponent->StressTuning.SnappedSegmentLinearDamping = PresetValues.SnappedSegmentLinearDamping;
		Wizard->StaffComponent->StressTuning.SnappedSegmentAngularDamping = PresetValues.SnappedSegmentAngularDamping;
		Wizard->StaffComponent->StressTuning.bSnapImpulseIgnoresSegmentMass = PresetValues.bSnapImpulseIgnoresSegmentMass;
		Wizard->StaffComponent->StaffStress = FMath::Clamp(Wizard->StaffComponent->StaffStress, 0.0f, FMath::Max(PresetValues.MaxStaffStress, 1.0f));
	}

	Wizard->SyncReplicatedManaSloshFromAuthority(true);
	Wizard->SyncReplicatedStaffStressFromAuthority(true);
}

void AWizardStaffGameMode::ApplyPrototypeTuningPresetToPickup(AWizardStaffManaMugPickup* MugPickup, const FWizardPrototypeTuningPresetValues& PresetValues) const
{
	if (!MugPickup)
	{
		return;
	}

	MugPickup->PickupTuning.RespawnDelay = PresetValues.MugRespawnDelay;
	if (MugPickup->PickupCollision)
	{
		MugPickup->PickupCollision->SetSphereRadius(MugPickup->PickupTuning.PickupRadius);
	}
}

void AWizardStaffGameMode::AnnouncePrototypeTuningPreset() const
{
	const FString PresetText = GetActivePrototypeTuningPresetText();
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff tuning preset active: %s"), *PresetText);
	AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("Preset: %s"), *PresetText), FColor::Magenta, 2.5f, EWizardHudMessageCategory::Debug);
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4500, 2.5f, FColor::Magenta, FString::Printf(TEXT("Preset: %s"), *PresetText));
	}
}

void AWizardStaffGameMode::EnsurePlayerRoundWinsSize(int32 MinimumPlayerCount)
{
	int32 DesiredSize = FMath::Max(MinimumPlayerCount, GetDesiredLocalPlayerCountForSession());
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != INDEX_NONE)
		{
			DesiredSize = FMath::Max(DesiredSize, PlayerIndex + 1);
		}
	}

	const int32 OldSize = PlayerRoundWins.Num();
	if (OldSize >= DesiredSize)
	{
		return;
	}

	PlayerRoundWins.SetNum(DesiredSize);
	for (int32 WinIndex = OldSize; WinIndex < PlayerRoundWins.Num(); ++WinIndex)
	{
		PlayerRoundWins[WinIndex] = 0;
	}
}

void AWizardStaffGameMode::EnsurePlayerGrandWizardFavorSize(int32 MinimumPlayerCount)
{
	int32 DesiredSize = FMath::Max(MinimumPlayerCount, GetDesiredLocalPlayerCountForSession());
	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != INDEX_NONE)
		{
			DesiredSize = FMath::Max(DesiredSize, PlayerIndex + 1);
		}
	}

	const int32 OldSize = PlayerGrandWizardFavor.Num();
	if (OldSize >= DesiredSize)
	{
		return;
	}

	PlayerGrandWizardFavor.SetNum(DesiredSize);
	for (int32 FavorIndex = OldSize; FavorIndex < PlayerGrandWizardFavor.Num(); ++FavorIndex)
	{
		PlayerGrandWizardFavor[FavorIndex] = 0;
	}
}

void AWizardStaffGameMode::AddGrandWizardFavor(int32 PlayerIndex, int32 FavorAmount, const FString& Reason)
{
	if (PlayerIndex == INDEX_NONE || FavorAmount <= 0)
	{
		return;
	}

	EnsurePlayerGrandWizardFavorSize(PlayerIndex + 1);
	if (!PlayerGrandWizardFavor.IsValidIndex(PlayerIndex))
	{
		return;
	}

	PlayerGrandWizardFavor[PlayerIndex] += FavorAmount;
	const FString FeedbackText = FString::Printf(TEXT("P%d gained +%d Favor: %s"), PlayerIndex + 1, FavorAmount, *Reason);
	if (bEnablePlaytestTelemetry)
	{
		if (FWizardPlayerPlaytestStats* Stats = FindOrAddPlayerPlaytestStats(PlayerIndex))
		{
			Stats->GrandWizardFavorEarned = PlayerGrandWizardFavor[PlayerIndex];
		}
	}
	SetGrandWizardFavorFeedbackMessage(FeedbackText, FColor::Green);

	UE_LOG(LogTemp, Log, TEXT("%s. Total Favor %d."), *FeedbackText, PlayerGrandWizardFavor[PlayerIndex]);
	SyncReplicatedPlayerStateForIndex(PlayerIndex);
}

void AWizardStaffGameMode::SetGrandWizardFavorFeedbackMessage(const FString& Message, const FColor& MessageColor)
{
	GrandWizardFavorFeedbackMessage = Message;
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	GrandWizardFavorFeedbackExpireTime = Now + FMath::Max(StaffsAtDawnTuning.ScoreFeedbackDuration, 0.0f);
	AWizardStaffHUD::PushGameplayMessage(this, Message, MessageColor, StaffsAtDawnTuning.ScoreFeedbackDuration, EWizardHudMessageCategory::Scoring);

	if (GEngine && PartyMatchTuning.bShowDebug && !Message.IsEmpty() && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(-1, FMath::Max(StaffsAtDawnTuning.ScoreFeedbackDuration, 0.75f), MessageColor, Message);
	}
}

FString AWizardStaffGameMode::GetPrototypeTuningPresetText(EWizardPrototypeTuningPreset Preset)
{
	switch (Preset)
	{
	case EWizardPrototypeTuningPreset::Stable:
		return TEXT("Stable");
	case EWizardPrototypeTuningPreset::Absurd:
		return TEXT("Arcane Catastrophe");
	case EWizardPrototypeTuningPreset::Chaotic:
	default:
		return TEXT("Chaotic");
	}
}

FString AWizardStaffGameMode::GetPartyMatchStateText(EWizardPartyMatchState State)
{
	switch (State)
	{
	case EWizardPartyMatchState::PartyHall:
		return TEXT("Party Hall");
	case EWizardPartyMatchState::Intermission:
		return TEXT("Intermission");
	case EWizardPartyMatchState::Trial:
		return TEXT("Trial");
	case EWizardPartyMatchState::Results:
		return TEXT("Results");
	case EWizardPartyMatchState::FinalRound:
	default:
		return TEXT("Final Round");
	}
}

FString AWizardStaffGameMode::GetTrialStateText(EWizardTrialState State)
{
	switch (State)
	{
	case EWizardTrialState::WaitingToStart:
		return TEXT("Waiting To Start");
	case EWizardTrialState::Countdown:
		return TEXT("Countdown");
	case EWizardTrialState::Active:
		return TEXT("Active");
	case EWizardTrialState::Results:
		return TEXT("Results");
	case EWizardTrialState::Finished:
	default:
		return TEXT("Finished");
	}
}

FString AWizardStaffGameMode::GetTrialTypeText(EWizardTrialType TrialType)
{
	switch (TrialType)
	{
	case EWizardTrialType::StaffsAtDawn:
		return TEXT("Staffs at Dawn");
	case EWizardTrialType::CauldronCatastrophe:
		return TEXT("Cauldron Catastrophe");
	case EWizardTrialType::MugRun:
	default:
		return TEXT("Mug Run");
	}
}

void AWizardStaffGameMode::UpdateLeaderHighlights() const
{
	const TArray<AWizardStaffWizardCharacter*> Wizards = GetCurrentWizards();
	if (PartyMatchState == EWizardPartyMatchState::FinalRound)
	{
		const int32 HighlightPlayerIndex = GrandWizardWinnerPlayerIndex != INDEX_NONE ? GrandWizardWinnerPlayerIndex : GrandWizardCandidatePlayerIndex;
		for (AWizardStaffWizardCharacter* Wizard : Wizards)
		{
			if (!Wizard)
			{
				continue;
			}

			const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
			Wizard->SetLeaderHighlight(HighlightPlayerIndex != INDEX_NONE && PlayerIndex == HighlightPlayerIndex);
		}
		return;
	}

	const int32 StandingLeaderIndex = GetCurrentStandingLeaderPlayerIndex();
	for (AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (Wizard)
		{
			const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
			Wizard->SetLeaderHighlight(StandingLeaderIndex != INDEX_NONE && PlayerIndex == StandingLeaderIndex);
		}
	}
}

void AWizardStaffGameMode::DrawMugRunDebug() const
{
	if (!MugRunTuning.bShowDebug || !GEngine || !AWizardStaffHUD::IsFullDebugMode(this))
	{
		return;
	}

	const int32 TrialTarget = FMath::Max(PartyMatchTuning.TrialsBeforeFinalRound, 1);
	const int32 DisplayTrialNumber = FMath::Clamp(CompletedTrialCount + 1, 1, TrialTarget);
	FString ScoreText = FString::Printf(
		TEXT("Party %s | Preset %s | Trial %d/%d %s %s"),
		*GetPartyMatchStateText(),
		*GetActivePrototypeTuningPresetText(),
		DisplayTrialNumber,
		TrialTarget,
		*GetActiveTrialName(),
		*GetActiveTrialStateText());

	if (ActiveTrialState == EWizardTrialState::Countdown)
	{
		ScoreText += FString::Printf(TEXT(" %.0fs"), TrialCountdownRemainingTime);
	}
	else if (PartyMatchState == EWizardPartyMatchState::FinalRound)
	{
		ScoreText += FString::Printf(TEXT(" %.0fs | Candidate %s"), FinalRoundRemainingTime, *GetGrandWizardCandidateText());
		if (GrandWizardStealPlayerIndex != INDEX_NONE)
		{
			ScoreText += FString::Printf(TEXT(" | P%d stealing %.0f%%"), GrandWizardStealPlayerIndex + 1, GetGrandWizardStealProgressAlpha() * 100.0f);
		}
		if (!GrandWizardWinnerMessage.IsEmpty())
		{
			ScoreText += FString::Printf(TEXT(" | %s"), *GrandWizardWinnerMessage);
		}
	}
	else if (IsPartyHallActive() && ActiveTrialState == EWizardTrialState::WaitingToStart)
	{
		ScoreText += FString::Printf(TEXT(" %.0fs"), IntermissionRemainingTime);
	}
	else if (ActiveTrialState == EWizardTrialState::Results)
	{
		ScoreText += FString::Printf(TEXT(" %.0fs"), TrialResultsRemainingTime);
	}
	else if (ActiveTrialType == EWizardTrialType::StaffsAtDawn && bStaffsAtDawnTrialActive)
	{
		ScoreText += FString::Printf(TEXT(" %.0fs"), StaffsAtDawnRemainingTime);
	}
	else if (MugRunMatchState == EWizardMugRunMatchState::Playing)
	{
		ScoreText += FString::Printf(TEXT(" %.0fs"), MugRunRemainingTime);
	}

	for (const AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		const bool bLeader = PlayerIndex == GetCurrentStandingLeaderPlayerIndex();
		if (ActiveTrialType == EWizardTrialType::StaffsAtDawn)
		{
			ScoreText += FString::Printf(
				TEXT(" | P%d Score %d Staff %d Favor %d Wins %d%s"),
				PlayerIndex + 1,
				GetStaffsAtDawnScore(PlayerIndex),
				Wizard->GetStaffSegmentCount(),
				GetPlayerGrandWizardFavor(PlayerIndex),
				GetPlayerRoundWins(PlayerIndex),
				bLeader ? TEXT(" Leader") : TEXT(""));
		}
		else
		{
			ScoreText += FString::Printf(
				TEXT(" | P%d Staff %d Favor %d Wins %d%s"),
				PlayerIndex + 1,
				Wizard->GetStaffSegmentCount(),
				GetPlayerGrandWizardFavor(PlayerIndex),
				GetPlayerRoundWins(PlayerIndex),
				bLeader ? TEXT(" Leader") : TEXT(""));
		}
	}

	GEngine->AddOnScreenDebugMessage(4100, 0.0f, FColor::Green, ScoreText);
}

void AWizardStaffGameMode::AnnounceMugRunWinner()
{
	const TArray<AWizardStaffWizardCharacter*> Wizards = GetCurrentWizards();
	int32 BestScore = INDEX_NONE;
	TArray<int32> WinningPlayerIndexes;

	for (const AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		const int32 SegmentCount = Wizard->GetStaffSegmentCount();

		UE_LOG(LogTemp, Log, TEXT("Mug Run final score: Player %d staff height %d."), PlayerIndex + 1, SegmentCount);

		if (SegmentCount > BestScore)
		{
			BestScore = SegmentCount;
			WinningPlayerIndexes.Reset();
			WinningPlayerIndexes.Add(PlayerIndex);
		}
		else if (SegmentCount == BestScore)
		{
			WinningPlayerIndexes.Add(PlayerIndex);
		}
	}

	if (WinningPlayerIndexes.Num() == 0)
	{
		MugRunWinnerMessage = TEXT("Mug Run ended: no winner.");
	}
	else if (WinningPlayerIndexes.Num() == 1)
	{
		MugRunWinnerMessage = FString::Printf(TEXT("Mug Run Winner: P%d with staff height %d!"), WinningPlayerIndexes[0] + 1, BestScore);
	}
	else
	{
		MugRunWinnerMessage = FString::Printf(TEXT("Mug Run Tie at staff height %d:"), BestScore);
		for (const int32 PlayerIndex : WinningPlayerIndexes)
		{
			MugRunWinnerMessage += FString::Printf(TEXT(" P%d"), PlayerIndex + 1);
		}
	}

	for (const int32 PlayerIndex : WinningPlayerIndexes)
	{
		EnsurePlayerRoundWinsSize(PlayerIndex + 1);
		if (PlayerRoundWins.IsValidIndex(PlayerIndex))
		{
			++PlayerRoundWins[PlayerIndex];
		}
		const bool bTiedWin = WinningPlayerIndexes.Num() > 1;
		const int32 FavorAward = bTiedWin ? GrandWizardFavorTuning.TiedTrialWinnerFavor : GrandWizardFavorTuning.MugRunWinnerFavor;
		AddGrandWizardFavor(PlayerIndex, FavorAward, bTiedWin ? TEXT("Mug Run Tie") : TEXT("Mug Run Winner"));
	}
	SyncPlaytestTelemetryRoundWins();
	SyncAllReplicatedPlayerStates();

	UE_LOG(LogTemp, Warning, TEXT("%s"), *MugRunWinnerMessage);
	AWizardStaffHUD::PushGameplayMessage(this, MugRunWinnerMessage, FColor::Yellow, 4.0f, EWizardHudMessageCategory::Scoring);
	if (GEngine && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4200, 8.0f, FColor::Yellow, MugRunWinnerMessage);
	}
}

AStaticMeshActor* AWizardStaffGameMode::SpawnArenaBlock(FName Name, const FVector& Location, const FVector& Scale) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!CubeMesh)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = MakeUniqueObjectName(World, AStaticMeshActor::StaticClass(), Name);

	AStaticMeshActor* Block = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParameters);
	if (!Block)
	{
		return nullptr;
	}

	Block->SetActorScale3D(Scale);
	Block->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	Block->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
	Block->GetStaticMeshComponent()->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	return Block;
}

FVector AWizardStaffGameMode::GetArenaCenter() const
{
	if (bCauldronCatastropheActive)
	{
		return IsValid(ActiveCauldronArena) ? ActiveCauldronArena->GetActorLocation() : RuntimeCauldronArenaLocation;
	}

	if (ShouldUseStaffsAtDawnArena())
	{
		return ActiveStaffsAtDawnArena->GetArenaBoundsCenter();
	}

	return ActivePrototypeArena ? ActivePrototypeArena->GetArenaBoundsCenter() : FVector::ZeroVector;
}

FVector AWizardStaffGameMode::GetCurrentPlayBoundsCenter() const
{
	if (IsPartyHallActive())
	{
		return ActivePartyHall ? ActivePartyHall->GetHallBoundsCenter() : PartyHallSpawnLocation;
	}

	return GetArenaCenter();
}

float AWizardStaffGameMode::GetCurrentPlayBoundsHalfSize() const
{
	if (IsPartyHallActive())
	{
		return ActivePartyHall ? ActivePartyHall->GetHallHalfSize() : PartyHallFallbackHalfSize;
	}

	if (ShouldUseStaffsAtDawnArena())
	{
		return ActiveStaffsAtDawnArena->GetArenaHalfSize();
	}

	if (bCauldronCatastropheActive)
	{
		return CauldronCatastropheTuning.ArenaHalfSize;
	}

	return ArenaHalfSize;
}

bool AWizardStaffGameMode::IsPartyHallActive() const
{
	return PartyMatchState == EWizardPartyMatchState::PartyHall || PartyMatchState == EWizardPartyMatchState::Intermission;
}

float AWizardStaffGameMode::GetCurrentOutOfArenaFallZThreshold() const
{
	float FallZThreshold = OutOfArenaRespawnTuning.FallZThreshold;
	if (ShouldUseStaffsAtDawnArena() && ActiveStaffsAtDawnArena)
	{
		FallZThreshold = FMath::Max(FallZThreshold, ActiveStaffsAtDawnArena->GetRingOutFallZ());
	}
	return FallZThreshold;
}

void AWizardStaffGameMode::UpdateOutOfArenaRespawns(float DeltaSeconds)
{
	if (!OutOfArenaRespawnTuning.bEnableOutOfArenaRespawn)
	{
		ClearPendingOutOfArenaRespawns();
		return;
	}

	// Countdown, Results, and Party Hall deliberately stage wizards away from the active Trial bounds.
	// Treating those positions as ring-outs can race the phase teleport and produce empty-arena flashes.
	if (ActiveTrialState != EWizardTrialState::Active)
	{
		ClearPendingOutOfArenaRespawns();
		return;
	}

	TArray<TWeakObjectPtr<AWizardStaffWizardCharacter>> TimersToRemove;
	TArray<TWeakObjectPtr<AWizardStaffWizardCharacter>> WizardsToRespawn;

	for (TPair<TWeakObjectPtr<AWizardStaffWizardCharacter>, float>& PendingRespawn : PendingOutOfArenaRespawns)
	{
		AWizardStaffWizardCharacter* Wizard = PendingRespawn.Key.Get();
		if (!Wizard)
		{
			TimersToRemove.Add(PendingRespawn.Key);
			continue;
		}

		if (OutOfArenaRespawnTuning.bCancelRespawnIfPlayerReturns && !IsWizardOutOfArena(Wizard))
		{
			CauldronProcessedRingOutSpills.Remove(PendingRespawn.Key);
			RecordTelemetryBroomBoostRingOutSave(Wizard);
			Wizard->SyncReplicatedOutOfArenaRespawnStateFromAuthority(false, 0.0f, true);
			TimersToRemove.Add(PendingRespawn.Key);
			if (GEngine && OutOfArenaRespawnTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
			{
				GEngine->AddOnScreenDebugMessage(static_cast<uint64>(4300) + Wizard->GetUniqueID(), 0.75f, FColor::Green, TEXT("Respawn canceled: wizard returned to play area."));
			}
			continue;
		}

		PendingRespawn.Value = FMath::Max(0.0f, PendingRespawn.Value - DeltaSeconds);
		Wizard->SyncReplicatedOutOfArenaRespawnStateFromAuthority(true, PendingRespawn.Value);
		if (PendingRespawn.Value <= 0.0f)
		{
			WizardsToRespawn.Add(PendingRespawn.Key);
			TimersToRemove.Add(PendingRespawn.Key);
		}
	}

	for (const TWeakObjectPtr<AWizardStaffWizardCharacter>& WizardKey : WizardsToRespawn)
	{
		if (AWizardStaffWizardCharacter* Wizard = WizardKey.Get())
		{
			RecordTelemetryOutOfArenaRespawn(Wizard);
			RespawnWizardInArena(Wizard);
			CauldronProcessedRingOutSpills.Remove(WizardKey);
		}
	}

	for (const TWeakObjectPtr<AWizardStaffWizardCharacter>& TimerKey : TimersToRemove)
	{
		PendingOutOfArenaRespawns.Remove(TimerKey);
	}

	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard || !IsWizardOutOfArena(Wizard))
		{
			continue;
		}

		const TWeakObjectPtr<AWizardStaffWizardCharacter> WizardKey(Wizard);
		if (!PendingOutOfArenaRespawns.Contains(WizardKey))
		{
			const float RespawnDelay = FMath::Max(OutOfArenaRespawnTuning.RespawnDelay, 0.0f);
			PendingOutOfArenaRespawns.Add(WizardKey, RespawnDelay);
			Wizard->CancelStaffClashStateForReset();
			if (bCauldronCatastropheActive)
			{
				HandleCauldronRingOutSpill(Wizard);
			}
			Wizard->SyncReplicatedOutOfArenaRespawnStateFromAuthority(true, RespawnDelay, true);
			const int32 PlayerIndex = GetPlayerIndexForWizard(Wizard);
			PublishReplicatedGameplayEvent(
				EWizardReplicatedGameplayEventType::RingOutPending,
				PlayerIndex == INDEX_NONE ? TEXT("Wizard ring-out") : FString::Printf(TEXT("P%d ring-out"), PlayerIndex + 1),
				PlayerIndex,
				INDEX_NONE,
				RespawnDelay,
				true,
				1.8f);
			const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			if (HasRecentStaffsAtDawnBroomBoost(Wizard, Now))
			{
				MarkStaffsAtDawnBroomBoostRingOutThreat(Wizard);
			}
			if (GEngine && OutOfArenaRespawnTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
			{
				const FString DebugText = FString::Printf(TEXT("Wizard out of play area: respawning in %.1fs."), OutOfArenaRespawnTuning.RespawnDelay);
				GEngine->AddOnScreenDebugMessage(static_cast<uint64>(4300) + Wizard->GetUniqueID(), 1.0f, FColor::Orange, DebugText);
			}
		}
	}
}

void AWizardStaffGameMode::ClearPendingOutOfArenaRespawns(bool bNotifyWizards)
{
	if (bNotifyWizards)
	{
		for (const TPair<TWeakObjectPtr<AWizardStaffWizardCharacter>, float>& PendingRespawn : PendingOutOfArenaRespawns)
		{
			if (AWizardStaffWizardCharacter* Wizard = PendingRespawn.Key.Get())
			{
				Wizard->SyncReplicatedOutOfArenaRespawnStateFromAuthority(false, 0.0f, true);
			}
		}
	}

	PendingOutOfArenaRespawns.Reset();
}

bool AWizardStaffGameMode::IsWizardOutOfArena(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!Wizard)
	{
		return false;
	}

	const FVector Location = Wizard->GetActorLocation();
	const FVector BoundsCenter = GetCurrentPlayBoundsCenter();
	const float HorizontalLimit = GetCurrentPlayBoundsHalfSize() + FMath::Max(OutOfArenaRespawnTuning.HorizontalOutOfBoundsPadding, 0.0f);
	return FMath::Abs(Location.X - BoundsCenter.X) > HorizontalLimit
		|| FMath::Abs(Location.Y - BoundsCenter.Y) > HorizontalLimit
		|| Location.Z < GetCurrentOutOfArenaFallZThreshold();
}

void AWizardStaffGameMode::RespawnWizardInArena(AWizardStaffWizardCharacter* Wizard) const
{
	if (!Wizard)
	{
		return;
	}

	AController* Controller = Wizard->GetController();
	const FTransform RespawnTransform = GetSpawnTransformForController(Controller);

	if (UCharacterMovementComponent* MovementComponent = Wizard->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
	Wizard->CancelMovementStateForRespawn();

	Wizard->SetActorLocationAndRotation(
		RespawnTransform.GetLocation(),
		RespawnTransform.Rotator(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	Wizard->SyncReplicatedOutOfArenaRespawnStateFromAuthority(false, 0.0f, true);
	Wizard->ForceNetUpdate();

	if (GEngine && OutOfArenaRespawnTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(4300) + Wizard->GetUniqueID(), 1.0f, FColor::Cyan, TEXT("Wizard respawned in play area."));
	}
}

void AWizardStaffGameMode::EnterPartyHallIntermission()
{
	PartyMatchState = EWizardPartyMatchState::PartyHall;
	ActiveTrialState = EWizardTrialState::WaitingToStart;
	bWizardsStagedForActiveTrial = false;
	// Lock only during the authoritative transfer away from a prior Trial floor.
	SetWizardPrototypeInputsLocked(true);
	if (CompletedTrialCount > 0)
	{
		ClearReplicatedGameplayEventFeed();
	}
	if (CompletedTrialCount < FMath::Max(PartyMatchTuning.TrialsBeforeFinalRound, 1))
	{
		ActiveTrialType = GetTrialTypeForTrialIndex(CompletedTrialCount);
	}
	IntermissionRemainingTime = FMath::Max(PartyMatchTuning.IntermissionDuration, 0.0f);
	TrialCountdownRemainingTime = 0.0f;
	TrialResultsRemainingTime = 0.0f;
	MugRunPostMatchRemainingTime = 0.0f;
	bMugRunMatchActive = false;
	MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;
	SetMugRunPickupsActive(false);
	// Keep the Mug Run arena out of sight and collision while Party Hall owns the next phase.
	SetPrototypeArenaPhasePresentationActive(false);
	ClearPendingOutOfArenaRespawns();
	ResetPartyHallReadyStates();

	StageWizardsForCurrentPhase();

	// Keep the prior Cauldron arena alive for the full intermission. On clients, actor destruction
	// can arrive before the pawn teleport is visually settled even when the server orders teleport first.
	// StartTrialCountdown or EnterGrandWizardFinalRound performs teardown after this grace period.
	SetWizardPrototypeInputsLocked(false);
	UpdatePartyHallSigns();

	AWizardStaffHUD::PushGameplayMessage(this, FString::Printf(TEXT("Party Hall intermission %.0fs"), IntermissionRemainingTime), FColor::Cyan, 2.0f, EWizardHudMessageCategory::Gameplay);
	if (GEngine && PartyMatchTuning.bShowDebug && AWizardStaffHUD::IsFullDebugMode(this))
	{
		GEngine->AddOnScreenDebugMessage(4620, 2.0f, FColor::Cyan, FString::Printf(TEXT("Party Hall intermission %.0fs"), IntermissionRemainingTime));
	}

	if (IntermissionRemainingTime <= 0.0f)
	{
		StartNextTrial();
	}
}
