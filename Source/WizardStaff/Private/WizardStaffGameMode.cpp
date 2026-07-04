#include "WizardStaffGameMode.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameInstance.h"
#include "Engine/CollisionProfile.h"
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
#include "Misc/ScopeExit.h"
#include "UObject/UObjectGlobals.h"
#include "WizardStaffArcanePinballProjectile.h"
#include "WizardStaffComponent.h"
#include "WizardStaffFinalRitualCircle.h"
#include "WizardStaffGameState.h"
#include "WizardStaffManaMugPickup.h"
#include "WizardStaffPartyHall.h"
#include "WizardStaffHUD.h"
#include "WizardStaffPlayerState.h"
#include "WizardStaffPrototypeArena.h"
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
	case EWizardReplicatedGameplayEventType::RingOutPending:
		return FColor::Red;
	case EWizardReplicatedGameplayEventType::RespawnComplete:
	case EWizardReplicatedGameplayEventType::StaffClashStarted:
		return FColor::Cyan;
	case EWizardReplicatedGameplayEventType::StaffClashResolved:
	case EWizardReplicatedGameplayEventType::GrandWizardCandidateChanged:
	case EWizardReplicatedGameplayEventType::FinalWinner:
		return FColor::Yellow;
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
	case EWizardReplicatedGameplayEventType::RingOutPending:
	case EWizardReplicatedGameplayEventType::RespawnComplete:
	case EWizardReplicatedGameplayEventType::StaffClashStarted:
	case EWizardReplicatedGameplayEventType::StaffClashResolved:
		return EWizardHudMessageCategory::Gameplay;
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
	return DetectPrototypeSessionMode() == EWizardPrototypeSessionMode::OnlineListenServer
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

	const int32 PlayerIndex = GetControllerIndex(Controller);
	const AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(Controller->GetPawn());
	const int32 StaffSegmentScore = Wizard ? Wizard->GetStaffSegmentCount() : 0;
	const int32 StaffsAtDawnScore = GetStaffsAtDawnScore(PlayerIndex);
	const int32 CurrentTrialScore = ActiveTrialType == EWizardTrialType::StaffsAtDawn ? StaffsAtDawnScore : StaffSegmentScore;
	const bool bBot = IsPlayerIndexPlaytestBot(PlayerIndex);
	const FString SummaryText = FString::Printf(
		TEXT("P%d Staff %d Favor %d Wins %d%s"),
		PlayerIndex + 1,
		StaffSegmentScore,
		GetPlayerGrandWizardFavor(PlayerIndex),
		GetPlayerRoundWins(PlayerIndex),
		bBot ? TEXT(" BOT") : TEXT(""));

	WizardPlayerState->SetWizardPlayerMirror(
		PlayerIndex,
		PlayerIndex,
		GetPlayerRoundWins(PlayerIndex),
		GetPlayerGrandWizardFavor(PlayerIndex),
		CurrentTrialScore,
		StaffsAtDawnScore,
		IsPartyHallPlayerReady(PlayerIndex),
		bBot,
		SummaryText);
	WizardPlayerState->ForceNetUpdate();

	if (!IsStandaloneLocalPrototypeSession())
	{
		UE_LOG(LogTemp, Log, TEXT("WizardStaff online scaffold assigned %s to display slot P%d."),
			*GetNameSafe(Controller),
			PlayerIndex + 1);
	}
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
				UE_LOG(LogTemp, Log, TEXT("WizardStaff listen server holding Party Hall until a second player connects."));
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

	ResetMugRunForNewMatch();
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

	if (ActiveTrialType == EWizardTrialType::MugRun)
	{
		ResetMugRunForNewMatch();
		SetMugRunPickupsActive(false);
	}
	else if (ActiveTrialType == EWizardTrialType::StaffsAtDawn)
	{
		ResetStaffsAtDawnForNewTrial();
		SetMugRunPickupsActive(false);
	}

	if (PartyMatchTuning.bResetStaffsAtTrialStart)
	{
		ResetWizardStaffsForTrialStart();
	}
	RespawnWizardsForTrialStart();
	bWizardsStagedForActiveTrial = true;
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
	default:
		StartStaffsAtDawnTrial();
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
	if (!bWizardsStagedForActiveTrial)
	{
		RespawnWizardsForTrialStart();
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
		RespawnWizardsForStaffsAtDawn();
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
	TrialResultsRemainingTime = 0.0f;
	MugRunPostMatchRemainingTime = 0.0f;
	IntermissionRemainingTime = 0.0f;
	ActiveTrialState = EWizardTrialState::Finished;
	MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;
	bStaffsAtDawnTrialActive = false;
	++CompletedTrialCount;

	if (CompletedTrialCount >= FMath::Max(PartyMatchTuning.TrialsBeforeFinalRound, 1))
	{
		EnterGrandWizardFinalRound();
		return;
	}

	EnterPartyHallIntermission();
}

void AWizardStaffGameMode::EnterGrandWizardFinalRound()
{
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
	return PlayerState ? FMath::Max(PlayerState->GetPlayerId(), 0) : INDEX_NONE;
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

	int32 ControllerIndex = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (PlayerController && ControllerIndex == PlayerIndex)
		{
			return Cast<AWizardStaffPlayerState>(PlayerController->PlayerState);
		}
		++ControllerIndex;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
		if (PlayerState && FMath::Max(PlayerState->GetPlayerId(), 0) == PlayerIndex)
		{
			return Cast<AWizardStaffPlayerState>(PlayerController->PlayerState);
		}
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
	const int32 CurrentTrialScore = ActiveTrialType == EWizardTrialType::StaffsAtDawn ? StaffsAtDawnScore : StaffSegmentScore;
	const bool bReady = IsPartyHallPlayerReady(PlayerIndex);
	const bool bBot = IsPlayerIndexPlaytestBot(PlayerIndex);
	const FString SummaryText = FString::Printf(
		TEXT("P%d Staff %d Favor %d Wins %d Dawn %d%s"),
		PlayerIndex + 1,
		StaffSegmentScore,
		GetPlayerGrandWizardFavor(PlayerIndex),
		GetPlayerRoundWins(PlayerIndex),
		StaffsAtDawnScore,
		bBot ? TEXT(" BOT") : TEXT(""));

	WizardPlayerState->SetWizardPlayerMirror(
		PlayerIndex,
		PlayerIndex,
		GetPlayerRoundWins(PlayerIndex),
		GetPlayerGrandWizardFavor(PlayerIndex),
		CurrentTrialScore,
		StaffsAtDawnScore,
		bReady,
		bBot,
		SummaryText);
	WizardPlayerState->ForceNetUpdate();
}

void AWizardStaffGameMode::SyncAllReplicatedPlayerStates()
{
	int32 DesiredSize = FMath::Max(GetDesiredLocalPlayerCountForSession(), PlayerRoundWins.Num());
	DesiredSize = FMath::Max(DesiredSize, PlayerGrandWizardFavor.Num());
	DesiredSize = FMath::Max(DesiredSize, StaffsAtDawnScores.Num());
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
	if (ShouldUseStaffsAtDawnArena())
	{
		FTransform StaffsAtDawnSpawnTransform;
		if (ActiveStaffsAtDawnArena->GetPlayerSpawnTransform(ControllerIndex, TargetPlayerCount, StaffsAtDawnSpawnTransform))
		{
			return StaffsAtDawnSpawnTransform;
		}
	}

	if (ActivePrototypeArena)
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
	const FString CountdownText = FString::Printf(TEXT("NEXT TRIAL\n%s\n%.0fs%s"), *GetActiveTrialName(), IntermissionRemainingTime, *ReadyLine);
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

void AWizardStaffGameMode::ResetMugRunForNewMatch()
{
	ClearPendingOutOfArenaRespawns();
	MugRunWinnerMessage.Reset();
	MugRunRemainingTime = MugRunTuning.MatchDuration;
	MugRunPostMatchRemainingTime = 0.0f;
	bMugRunMatchActive = false;
	MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;

	if (LooseSegmentChaosTuning.bCleanupLooseSegmentsOnRematch)
	{
		CleanupLooseSnappedSegments();
	}

	ResetWizardsForNewMatch();
	ResetMugRunPickups();
}

EWizardTrialType AWizardStaffGameMode::GetTrialTypeForTrialIndex(int32 TrialIndex) const
{
	return (TrialIndex % 2) == 0 ? EWizardTrialType::MugRun : EWizardTrialType::StaffsAtDawn;
}

void AWizardStaffGameMode::RespawnWizardsForTrialStart()
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

void AWizardStaffGameMode::RespawnWizardsForStaffsAtDawn()
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (Wizard)
		{
			RespawnWizardInArena(Wizard);
		}
	}
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

void AWizardStaffGameMode::ResetWizardsForNewMatch()
{
	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		Wizard->ResetForNewMatch(RematchTuning.bResetStaffsBetweenMatches, RematchTuning.bResetSloshBetweenMatches);
		RespawnWizardInArena(Wizard);
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

	int32 ControllerIndex = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (!PlayerController)
		{
			++ControllerIndex;
			continue;
		}

		if (ControllerIndex == PlayerIndex)
		{
			return Cast<AWizardStaffWizardCharacter>(PlayerController->GetPawn());
		}
		++ControllerIndex;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
		const int32 CurrentPlayerIndex = PlayerState ? FMath::Max(PlayerState->GetPlayerId(), 0) : INDEX_NONE;
		if (CurrentPlayerIndex == PlayerIndex)
		{
			return Cast<AWizardStaffWizardCharacter>(PlayerController->GetPawn());
		}
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
	SetWizardPrototypeInputsLocked(false);
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
	ClearPendingOutOfArenaRespawns();
	ResetPartyHallReadyStates();

	for (AWizardStaffWizardCharacter* Wizard : GetCurrentWizards())
	{
		if (!Wizard)
		{
			continue;
		}

		RespawnWizardInArena(Wizard);
	}
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
