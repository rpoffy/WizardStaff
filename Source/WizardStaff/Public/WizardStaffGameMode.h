#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "GameFramework/GameModeBase.h"
#include "WizardStaffStaffsAtDawnPowerupTypes.h"
#include "WizardStaffGameMode.generated.h"

class AActor;
class AStaticMeshActor;
class AController;
class APlayerController;
class AWizardStaffManaMugPickup;
class AWizardStaffFinalRitualCircle;
class AWizardStaffGameState;
class AWizardStaffPlayerState;
class AWizardStaffPartyHall;
class AWizardStaffPrototypeArena;
class AWizardStaffStaffsAtDawnArena;
class AWizardStaffStaffsAtDawnPowerupPickup;
class AWizardStaffSharedCamera;
class AWizardStaffWizardCharacter;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
enum class EWizardReplicatedGameplayEventType : uint8;

UENUM(BlueprintType)
enum class EWizardMugRunMatchState : uint8
{
	WaitingToStart,
	Playing,
	PostMatch
};

UENUM(BlueprintType)
enum class EWizardPartyMatchState : uint8
{
	PartyHall,
	Intermission,
	Trial,
	Results,
	FinalRound
};

UENUM(BlueprintType)
enum class EWizardTrialState : uint8
{
	WaitingToStart,
	Countdown,
	Active,
	Results,
	Finished
};

UENUM(BlueprintType)
enum class EWizardTrialType : uint8
{
	MugRun,
	StaffsAtDawn
};

UENUM(BlueprintType)
enum class EWizardPrototypeTuningPreset : uint8
{
	Stable,
	Chaotic,
	Absurd
};

UENUM(BlueprintType)
enum class EWizardPrototypeSessionMode : uint8
{
	LocalPrototype UMETA(DisplayName = "Local Prototype"),
	LocalWithBots UMETA(DisplayName = "Local With Bots"),
	OnlineListenServer UMETA(DisplayName = "Online Listen Server"),
	OnlineClient UMETA(DisplayName = "Online Client")
};

UENUM(BlueprintType)
enum class EWizardLooseSegmentChaosEffectType : uint8
{
	None UMETA(DisplayName = "None"),
	ManaSplash UMETA(DisplayName = "Mana Splash"),
	TripBonk UMETA(DisplayName = "Trip Bonk"),
	ArcanePop UMETA(DisplayName = "Arcane Pop")
};

USTRUCT(BlueprintType)
struct FWizardMugRunTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run")
	bool bEnableMugRun = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run", meta = (ClampMin = "5.0"))
	float MatchDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run")
	TSubclassOf<AWizardStaffManaMugPickup> ManaMugPickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run", meta = (ClampMin = "1"))
	int32 MugSpawnCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run", meta = (ClampMin = "0.0"))
	float MugSpawnZ = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run|Brew Rewards")
	bool bEnableBrewRewardsInMugRun = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run|Brew Rewards", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrewRewardChance = 0.50f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run|Brew Rewards")
	bool bReplaceExistingRewardOnPickup = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mug Run")
	bool bShowDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardPartyMatchTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Match", meta = (ClampMin = "0.0"))
	float IntermissionDuration = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Match", meta = (ClampMin = "0.0"))
	float TrialCountdownDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Match", meta = (ClampMin = "0.0"))
	float TrialResultsDisplayDuration = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Match", meta = (ClampMin = "1"))
	int32 TrialsBeforeFinalRound = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Match|Reset")
	bool bResetStaffsAtTrialStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Match")
	bool bShowDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardStaffsAtDawnTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn", meta = (ClampMin = "5.0"))
	float TrialDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn", meta = (ClampMin = "0"))
	int32 PointsPerBonk = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn", meta = (ClampMin = "0"))
	int32 PointsPerOutOfArena = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn", meta = (ClampMin = "0.0"))
	float OutOfArenaCreditWindow = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Combat Staff Growth", meta = (ClampMin = "0"))
	int32 LandedBonksPerStaffSegment = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Combat Staff Growth", meta = (ClampMin = "0"))
	int32 StaffSegmentsPerOutOfArena = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn", meta = (ClampMin = "0.0"))
	float ScoreFeedbackDuration = 2.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Broom Boost Telemetry", meta = (ClampMin = "0.0"))
	float BroomBoostRecoveryTelemetryWindow = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups")
	bool bEnableStaffsAtDawnPowerups = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups", meta = (ClampMin = "0"))
	int32 PowerupSpawnCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups", meta = (ClampMin = "0.0"))
	float PowerupRespawnDelay = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups", meta = (ClampMin = "0.0"))
	float InitialPowerupSpawnDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups")
	bool bRespawnPowerupsAfterPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups")
	EWizardStaffsAtDawnPowerupType DefaultPowerupType = EWizardStaffsAtDawnPowerupType::MegaStaffBrew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups|Mega Staff Brew", meta = (ClampMin = "0"))
	int32 MegaStaffBonusSegments = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups|Mega Staff Brew", meta = (ClampMin = "0.1"))
	float MegaStaffDuration = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups|Mega Staff Brew", meta = (ClampMin = "0.0"))
	float MegaStaffPickupRespawnDelay = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups|Mega Staff Brew", meta = (ClampMin = "0.0"))
	float MegaStaffStressMultiplierDuringEffect = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups|Mega Staff Brew", meta = (ClampMin = "0.0"))
	float MegaStaffKnockbackMultiplierDuringEffect = 1.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups|Mega Staff Brew")
	bool bRemoveTemporarySegmentsOnExpire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups|Mega Staff Brew")
	bool bShowMegaStaffDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn")
	bool bShowDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Arena Debug")
	bool bShowArenaDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Arena Debug")
	bool bDrawArenaBoundsDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Arena Debug")
	bool bDrawPlayerSpawnDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Arena Debug")
	bool bDrawRingOutBoundsDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staffs At Dawn|Arena Debug")
	bool bDrawFuturePowerupSpawnDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardGrandWizardFavorTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grand Wizard Favor", meta = (ClampMin = "0"))
	int32 MugRunWinnerFavor = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grand Wizard Favor", meta = (ClampMin = "0"))
	int32 StaffsAtDawnWinnerFavor = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grand Wizard Favor", meta = (ClampMin = "0"))
	int32 TiedTrialWinnerFavor = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grand Wizard Favor")
	bool bGrantFavorForStaffsAtDawnRingOuts = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grand Wizard Favor", meta = (ClampMin = "0"))
	int32 FavorPerStaffsAtDawnRingOut = 1;
};

USTRUCT(BlueprintType)
struct FWizardPartyHallTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Bonk")
	bool bUseGentleBonks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Bonk", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BonkKnockbackMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Bonk")
	bool bDisableBonkStaffStress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Ready Bell")
	bool bEnableReadyBell = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Ready Bell", meta = (ClampMin = "0.0"))
	float ReadyBellAllReadyCountdownDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party Hall|Ready Bell", meta = (ClampMin = "0.0"))
	float ReadyBellFeedbackDuration = 1.75f;
};

USTRUCT(BlueprintType)
struct FWizardPlaytestBotTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.02"))
	float BotThinkInterval = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0"))
	float BotMoveAggression = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0"))
	float BotTurnAggression = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BotBonkChance = 0.36f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BotStaffClashMashChance = 0.78f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0"))
	float BotBonkRangePadding = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BotJumpChance = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BotBroomBoostRecoveryChance = 0.78f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BotRewardUseChance = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0"))
	float BotReadyBellDelayMin = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.0"))
	float BotReadyBellDelayMax = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.1"))
	float BotTargetRefreshTime = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "0.1"))
	float BotStuckTimeBeforeRetarget = 1.2f;
};

USTRUCT(BlueprintType)
struct FWizardFinalRoundTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round", meta = (ClampMin = "5.0"))
	float FinalRoundDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round", meta = (ClampMin = "50.0"))
	float CircleRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round", meta = (ClampMin = "0.0"))
	float CandidateNearCirclePadding = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round", meta = (ClampMin = "0.0"))
	float CandidateSwapHoldDuration = 0.70f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round")
	bool bRequireCandidateOutsideCircleToSteal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round")
	bool bLeaderStartsWithBonus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round", meta = (ClampMin = "0.0"))
	float LeaderStartBonusDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Readability", meta = (ClampMin = "0.0"))
	float FeedbackMessageDuration = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage")
	bool bUseFavorBasedFinalStaffSetup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage")
	bool bLockManaSloshAtFinalStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage", meta = (ClampMin = "0"))
	int32 CandidateStartingBaseSegments = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage", meta = (ClampMin = "0.0"))
	float CandidateSegmentsPerFavor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage", meta = (ClampMin = "0"))
	int32 CandidateMaxStartingSegments = 14;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage", meta = (ClampMin = "0"))
	int32 ChallengerStartingBaseSegments = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage", meta = (ClampMin = "0.0"))
	float ChallengerSegmentsPerFavor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round|Favor Advantage", meta = (ClampMin = "0"))
	int32 ChallengerMaxStartingSegments = 9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Round")
	bool bShowDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardPlayerPlaytestStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 MugsCollected = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 StaffSegmentsGained = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 StaffSegmentsSnappedOff = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 BonksAttempted = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 BonksLanded = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 TimesBonked = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Staff Clash")
	int32 StaffClashesStarted = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Staff Clash")
	int32 StaffClashesWon = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Staff Clash")
	int32 StaffClashTies = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Staff Clash")
	int32 StaffClashRingOutsCaused = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 OutOfArenaRespawns = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Arcane Pinball")
	int32 ArcanePinballRewardsReceived = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Arcane Pinball")
	int32 ArcanePinballsCast = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Arcane Pinball")
	int32 ArcanePinballHitsOnPlayers = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Arcane Pinball")
	int32 ArcanePinballSelfHits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Arcane Pinball")
	int32 ArcanePinballTotalBounces = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Arcane Pinball")
	float ArcanePinballCastStressGained = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Arcane Pinball")
	float ArcanePinballHitStressGained = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 StaffsAtDawnScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 StaffsAtDawnCombatSegmentsGained = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 StaffsAtDawnRingOutsCaused = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Mega Staff")
	int32 MegaStaffPickupsCollected = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Mega Staff")
	int32 MegaStaffSegmentsGranted = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Mega Staff")
	int32 MegaStaffSegmentsSnappedDuringEffect = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Mega Staff")
	int32 MegaStaffRingOutsCaused = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Broom Boost")
	int32 BroomBoostsUsed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Broom Boost")
	int32 BroomBoostRingOutSaves = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Broom Boost")
	int32 BroomBoostFailedRingOutRecoveries = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Loose Segments")
	int32 LooseSegmentChaosHits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Loose Segments")
	float LooseSegmentManaSloshAdded = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Loose Segments")
	int32 LooseSegmentTripBonks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry|Loose Segments")
	int32 LooseSegmentArcanePops = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 GrandWizardFavorEarned = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	float TimeAsGrandWizardCandidate = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 FinalStaffSegmentCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	int32 RoundWins = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playtest Telemetry")
	bool bFinalWinner = false;
};

USTRUCT(BlueprintType)
struct FWizardPrototypeTuningPresetValues
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Collision", meta = (ClampMin = "1.0"))
	float CollisionLengthPerSegment = 38.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Collision", meta = (ClampMin = "1.0"))
	float CollisionThickness = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Collision", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ObstructedControlMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Collision", meta = (ClampMin = "0.0"))
	float ObstructionRecoverySpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stuck", meta = (ClampMin = "0.0"))
	float StuckTimeBeforeStressBoost = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stuck", meta = (ClampMin = "0.0"))
	float StuckStressPerSecond = 26.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stuck", meta = (ClampMin = "0.0"))
	float StuckTimeBeforeCollisionRelief = 2.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stuck", meta = (ClampMin = "0.0"))
	float CollisionReliefDuration = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stuck", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CollisionReliefControlMultiplier = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stuck", meta = (ClampMin = "0.0"))
	float GentleNudgeDistance = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "1.0"))
	float MaxStaffStress = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float StressGainedPerBonk = 9.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float StressGainedPerWallImpact = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float StressMultiplierPerSegment = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float StressDecayRate = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float CaughtStressPerSecond = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float SnapImpulseForce = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float SnapUpwardImpulse = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float SnappedSegmentLinearDamping = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress", meta = (ClampMin = "0.0"))
	float SnappedSegmentAngularDamping = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Stress")
	bool bSnapImpulseIgnoresSegmentMass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Respawn", meta = (ClampMin = "0.0"))
	float OutOfArenaRespawnDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Respawn", meta = (ClampMin = "0.0"))
	float HorizontalOutOfBoundsPadding = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Respawn")
	float FallZThreshold = -450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0"))
	float SloshGainedPerStaffSegment = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0"))
	float SloshReducedOnStaffSnap = 11.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0"))
	float SloshDecayPerSecond = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TurnPenaltyPerSlosh = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovementPenaltyPerSlosh = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccelerationPenaltyPerSlosh = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrakingPenaltyPerSlosh = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinMovementMultiplier = 0.62f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinTurnMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinAccelerationMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinBrakingMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0"))
	float SloshOversteerDegreesPerSecond = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0"))
	float StumbleChancePerSecondAtMaxSlosh = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0"))
	float SloshVisualLeanDegrees = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0"))
	float SloshStaffWobbleDegrees = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh")
	bool bSetSloshOnApply = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SloshAlphaOnApply = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0"))
	int32 SegmentCountBeforeHeftPenalty = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovementPenaltyPerHeavySegment = 0.035f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TurnPenaltyPerHeavySegment = 0.055f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float MaxMovementPenalty = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float MaxTurnPenalty = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0"))
	float BonkCooldownPerHeavySegment = 0.045f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0"))
	float BonkVisualDurationPerHeavySegment = 0.018f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0"))
	float MaxBonkCooldownBonus = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Staff Heft", meta = (ClampMin = "0.0"))
	float MaxBonkVisualDurationBonus = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkKnockbackStrength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkKnockbackPerManaSlosh = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkKnockbackPerStaffSegment = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkMaxKnockbackStrength = 1250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkUpwardBoost = 145.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkCooldown = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.01"))
	float BonkVisualDuration = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.1"))
	float BonkStrikeEaseExponent = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float StaffContactPadding = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkHitStressMultiplier = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Bonk", meta = (ClampMin = "0.0"))
	float BonkWhiffStressMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Mug Run", meta = (ClampMin = "5.0"))
	float MatchDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Mug Run", meta = (ClampMin = "1"))
	int32 MugSpawnCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Mug Run", meta = (ClampMin = "0.0"))
	float MugRespawnDelay = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Mug Run|Brew Rewards", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrewRewardChance = 0.50f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0.0"))
	float ArcanePinballProjectileSpeed = 1350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "1.0"))
	float ArcanePinballSpeedMultiplierPerWallBounce = 1.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0.0"))
	float ArcanePinballMaxProjectileSpeed = 3900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0"))
	int32 ArcanePinballMaxBounces = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0.1"))
	float ArcanePinballLifetime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0.0"))
	float ArcanePinballHitKnockback = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0.0"))
	float ArcanePinballSloshOnHit = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0.0"))
	float ArcanePinballStressOnCast = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball", meta = (ClampMin = "0.0"))
	float ArcanePinballStressOnHit = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball")
	bool bArcanePinballAllowSelfHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Arcane Pinball")
	bool bArcanePinballDestroyOnPlayerHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Loose Segments", meta = (ClampMin = "0.0"))
	float LooseSegmentLifetime = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Loose Segments", meta = (ClampMin = "0"))
	int32 MaxLooseSegments = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset|Loose Segments", meta = (ClampMin = "0.0"))
	float LooseSegmentFadeOutDuration = 1.0f;
};

USTRUCT(BlueprintType)
struct FWizardMugRunRematchTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rematch", meta = (ClampMin = "0.0"))
	float PostMatchDuration = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rematch")
	bool bResetStaffsBetweenMatches = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rematch")
	bool bResetSloshBetweenMatches = true;
};

USTRUCT(BlueprintType)
struct FWizardLooseSegmentChaosTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects")
	bool bEnableLooseSegmentChaosEffects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float ChaosActiveDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float ChaosTriggerCooldown = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float MinImpactSpeedForChaosEffect = 340.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects")
	bool bAffectOwner = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects")
	bool bAffectOtherPlayers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects")
	bool bShowLooseSegmentChaosDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float ManaSplashSloshAmount = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float TripBonkKnockback = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float TripBonkUpwardBoost = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float ArcanePopRadius = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float ArcanePopKnockback = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects", meta = (ClampMin = "0.0"))
	float ArcanePopUpwardBoost = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments|Chaos Effects")
	FLinearColor ChaosActiveColor = FLinearColor(0.15f, 0.95f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments", meta = (ClampMin = "0.0"))
	float LooseSegmentLifetime = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments", meta = (ClampMin = "0"))
	int32 MaxLooseSegments = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments")
	bool bCleanupLooseSegmentsOnRematch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loose Segments", meta = (ClampMin = "0.0"))
	float FadeOutDuration = 1.0f;
};

USTRUCT()
struct FWizardTrackedLooseSegment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> Actor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AWizardStaffWizardCharacter> SourceWizard;

	UPROPERTY(Transient)
	TWeakObjectPtr<UPrimitiveComponent> CollisionComponent;

	UPROPERTY(Transient)
	float Age = 0.0f;

	UPROPERTY(Transient)
	float ChaosCooldownRemaining = 0.0f;

	UPROPERTY(Transient)
	float FadeElapsed = 0.0f;

	UPROPERTY(Transient)
	bool bIsFading = false;

	UPROPERTY(Transient)
	FVector InitialScale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct FWizardOutOfArenaRespawnTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out Of Arena")
	bool bEnableOutOfArenaRespawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out Of Arena", meta = (ClampMin = "0.0"))
	float RespawnDelay = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out Of Arena", meta = (ClampMin = "0.0"))
	float HorizontalOutOfBoundsPadding = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out Of Arena")
	float FallZThreshold = -450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out Of Arena")
	bool bCancelRespawnIfPlayerReturns = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out Of Arena")
	bool bShowDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardKeyboardFallbackControls
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback", meta = (ClampMin = "1", ClampMax = "3"))
	int32 TargetPlayerIndex = 1;

	// The fallback feeds +/-1 into the same wizard movement and turn functions as normal input.
	// Tune fallback turn feel with the wizard's TurnRateDegreesPerSecond.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey MoveForwardKey = EKeys::I;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey MoveBackwardKey = EKeys::K;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey MoveLeftKey = EKeys::J;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey MoveRightKey = EKeys::L;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey TurnLeftKey = EKeys::U;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey TurnRightKey = EKeys::O;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey HopKey = EKeys::RightShift;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey QuickBonkKey = EKeys::RightControl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey DrinkMugKey = EKeys::P;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	FKey UseRewardKey = EKeys::Semicolon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard Fallback")
	bool bShowDebug = true;
};

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWizardStaffGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Party Match")
	void StartPartyMatch();

	UFUNCTION(BlueprintCallable, Category = "Party Match")
	void StartNextTrial();

	UFUNCTION(BlueprintCallable, Category = "Mug Run")
	void StartMugRunMatch();

	UFUNCTION(BlueprintCallable, Category = "Mug Run")
	void EndMugRunMatch();

	UFUNCTION(BlueprintCallable, Category = "Staffs At Dawn")
	void StartStaffsAtDawnTrial();

	UFUNCTION(BlueprintCallable, Category = "Staffs At Dawn")
	void EndStaffsAtDawnTrial();

	UFUNCTION(BlueprintCallable, Exec, Category = "Mug Run")
	void RestartMugRunMatch();

	UFUNCTION(BlueprintCallable, Category = "Loose Segments")
	void RegisterLooseSnappedSegment(AActor* LooseSegment);

	void RegisterLooseSnappedSegmentFromWizard(AActor* LooseSegment, AWizardStaffWizardCharacter* SourceWizard);

	UFUNCTION(BlueprintCallable, Category = "Loose Segments")
	void CleanupLooseSnappedSegments();

	UFUNCTION(BlueprintPure, Category = "Loose Segments")
	int32 GetLooseSnappedSegmentCount() const { return LooseSnappedSegments.Num(); }

	static FName GetLooseSnappedSegmentTag();

	UFUNCTION(BlueprintPure, Category = "Mug Run")
	float GetMugRunRemainingTime() const { return MugRunRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Mug Run")
	float GetMugRunPostMatchRemainingTime() const { return MugRunPostMatchRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Mug Run")
	EWizardMugRunMatchState GetMugRunMatchState() const { return MugRunMatchState; }

	UFUNCTION(BlueprintPure, Category = "Mug Run")
	FString GetMugRunMatchStateText() const;

	UFUNCTION(BlueprintPure, Category = "Mug Run")
	FString GetMugRunWinnerMessage() const { return MugRunWinnerMessage; }

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn")
	float GetStaffsAtDawnRemainingTime() const { return StaffsAtDawnRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn")
	int32 GetStaffsAtDawnScore(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn")
	FString GetStaffsAtDawnFeedbackMessage() const;

	UFUNCTION(BlueprintPure, Category = "Staffs At Dawn")
	bool IsStaffsAtDawnActive() const;

	UFUNCTION(BlueprintCallable, Category = "Staffs At Dawn|Powerups")
	void HandleStaffsAtDawnPowerupPickedUp(AWizardStaffStaffsAtDawnPowerupPickup* Pickup, AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintPure, Category = "Playtest Telemetry|Staffs At Dawn")
	int32 GetStaffsAtDawnTelemetryRingOuts() const { return StaffsAtDawnTelemetryRingOuts; }

	UFUNCTION(BlueprintPure, Category = "Playtest Telemetry|Staffs At Dawn")
	int32 GetStaffsAtDawnTelemetryRoundsCompleted() const { return StaffsAtDawnTelemetryRoundsCompleted; }

	UFUNCTION(BlueprintPure, Category = "Playtest Telemetry|Staffs At Dawn")
	float GetAverageStaffsAtDawnRespawnsPerRound() const;

	UFUNCTION(BlueprintPure, Category = "Party Match")
	EWizardPartyMatchState GetPartyMatchState() const { return PartyMatchState; }

	UFUNCTION(BlueprintPure, Category = "Party Match")
	EWizardTrialState GetActiveTrialState() const { return ActiveTrialState; }

	UFUNCTION(BlueprintPure, Category = "Party Match")
	EWizardTrialType GetActiveTrialType() const { return ActiveTrialType; }

	UFUNCTION(BlueprintPure, Category = "Party Match")
	float GetTrialCountdownRemainingTime() const { return TrialCountdownRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Party Match")
	float GetTrialResultsRemainingTime() const { return TrialResultsRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Party Match")
	float GetIntermissionRemainingTime() const { return IntermissionRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Party Match")
	int32 GetCompletedTrialCount() const { return CompletedTrialCount; }

	UFUNCTION(BlueprintPure, Category = "Party Match")
	int32 GetPlayerRoundWins(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Grand Wizard Favor")
	int32 GetPlayerGrandWizardFavor(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Grand Wizard Favor")
	FString GetGrandWizardFavorFeedbackMessage() const;

	UFUNCTION(BlueprintPure, Category = "Party Match")
	int32 GetCurrentStandingLeaderPlayerIndex() const;

	UFUNCTION(BlueprintPure, Category = "Party Match")
	FString GetPartyMatchStateText() const;

	UFUNCTION(BlueprintPure, Category = "Party Match")
	FString GetActiveTrialStateText() const;

	UFUNCTION(BlueprintPure, Category = "Party Match")
	FString GetActiveTrialName() const;

	UFUNCTION(BlueprintPure, Category = "Party Hall")
	bool IsPartyHallIntermissionActive() const;

	UFUNCTION(BlueprintPure, Category = "Party Hall")
	float GetPartyHallBonkKnockbackMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Party Hall")
	bool ShouldDisablePartyHallBonkStress() const;

	UFUNCTION(BlueprintCallable, Category = "Party Hall|Ready Bell")
	void NotifyPartyHallReadyBellBonked(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintPure, Category = "Party Hall|Ready Bell")
	bool IsPartyHallPlayerReady(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Party Hall|Ready Bell")
	int32 GetPartyHallReadyPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "Party Hall|Ready Bell")
	FString GetPartyHallReadyFeedbackMessage() const;

	UFUNCTION(BlueprintPure, Category = "Playtest Bots")
	bool GetPartyHallReadyBellLocation(FVector& OutLocation) const;

	UFUNCTION(BlueprintPure, Category = "Final Round")
	float GetFinalRoundRemainingTime() const { return FinalRoundRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Playtest Bots")
	FVector GetPlaytestBoundsCenter() const;

	UFUNCTION(BlueprintPure, Category = "Playtest Bots")
	float GetPlaytestBoundsHalfSize() const;

	UFUNCTION(BlueprintPure, Category = "Playtest Bots")
	FVector GetPlaytestFinalCircleCenter() const;

	UFUNCTION(BlueprintPure, Category = "Final Round")
	int32 GetGrandWizardCandidatePlayerIndex() const { return GrandWizardCandidatePlayerIndex; }

	UFUNCTION(BlueprintPure, Category = "Final Round")
	int32 GetGrandWizardWinnerPlayerIndex() const { return GrandWizardWinnerPlayerIndex; }

	UFUNCTION(BlueprintPure, Category = "Final Round")
	int32 GetGrandWizardStealPlayerIndex() const { return GrandWizardStealPlayerIndex; }

	UFUNCTION(BlueprintPure, Category = "Final Round")
	int32 GetGrandWizardCircleChallengerPlayerIndex() const;

	UFUNCTION(BlueprintPure, Category = "Final Round")
	float GetGrandWizardStealProgressAlpha() const;

	UFUNCTION(BlueprintPure, Category = "Final Round")
	float GetGrandWizardCandidateBonusRemainingTime() const { return GrandWizardStartBonusRemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Final Round")
	bool IsGrandWizardCandidateVulnerable() const;

	UFUNCTION(BlueprintPure, Category = "Final Round")
	FString GetGrandWizardWinnerMessage() const { return GrandWizardWinnerMessage; }

	UFUNCTION(BlueprintPure, Category = "Final Round")
	FString GetGrandWizardCandidateText() const;

	UFUNCTION(BlueprintPure, Category = "Final Round")
	FString GetGrandWizardCandidateIntroMessage() const { return GrandWizardCandidateIntroMessage; }

	UFUNCTION(BlueprintPure, Category = "Final Round")
	FString GetGrandWizardFinalFeedbackMessage() const;

	UFUNCTION(BlueprintPure, Category = "Final Round")
	FString GetGrandWizardFinalStaffSetupMessage() const { return GrandWizardFinalStaffSetupMessage; }

	UFUNCTION(BlueprintCallable, Category = "Prototype Tuning")
	void SetPrototypeTuningPreset(EWizardPrototypeTuningPreset NewPreset);

	UFUNCTION(BlueprintCallable, Exec, Category = "Prototype Tuning")
	void CyclePrototypeTuningPreset();

	UFUNCTION(BlueprintCallable, Exec, Category = "Prototype Tuning")
	void SetPrototypeTuningPresetByName(const FString& PresetName);

	UFUNCTION(BlueprintPure, Category = "Prototype Tuning")
	EWizardPrototypeTuningPreset GetActivePrototypeTuningPreset() const { return ActivePrototypeTuningPreset; }

	UFUNCTION(BlueprintPure, Category = "Prototype Tuning")
	FString GetActivePrototypeTuningPresetText() const;

	UFUNCTION(BlueprintCallable, Exec, Category = "Playtest Bots")
	void SetPlaytestBotsEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintCallable, Exec, Category = "Playtest Bots")
	void TogglePlaytestBots();

	UFUNCTION(BlueprintPure, Category = "Playtest Bots")
	bool IsPlayerIndexPlaytestBot(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Playtest Bots")
	int32 GetDesiredLocalPlayerCountForSession() const;

	UFUNCTION(BlueprintPure, Category = "Online Scaffold")
	EWizardPrototypeSessionMode GetPrototypeSessionMode() const { return PrototypeSessionMode; }

	UFUNCTION(BlueprintPure, Category = "Online Scaffold")
	FString GetPrototypeSessionModeText() const;

	UFUNCTION(BlueprintPure, Category = "Local Multiplayer")
	int32 GetPlayerIndexForWizard(const AWizardStaffWizardCharacter* Wizard) const;

	UFUNCTION(BlueprintCallable, Category = "Online Scaffold")
	void SyncReplicatedObservableState();

	void PublishReplicatedGameplayEvent(
		EWizardReplicatedGameplayEventType EventType,
		const FString& DisplayText,
		int32 PrimaryPlayerIndex = INDEX_NONE,
		int32 SecondaryPlayerIndex = INDEX_NONE,
		float NumericValue = 0.0f,
		bool bAlsoPushLocalHudMessage = false,
		float LocalHudLifetime = 2.6f);
	void ClearReplicatedGameplayEventFeed();

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerAddStaffSegmentToPlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerRemoveStaffSegmentFromPlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerClearStaffSegmentsForPlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerSnapTopStaffSegmentForPlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerAddManaSloshToPlayer(int32 PlayerIndex, float SloshAmount = 25.0f);

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerClearManaSloshForPlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerAddStaffStressToPlayer(int32 PlayerIndex, float StressAmount = 25.0f);

	UFUNCTION(BlueprintCallable, Exec, Category = "Online Scaffold")
	void DebugServerClearStaffStressForPlayer(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry")
	void ResetPlaytestTelemetry();

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry")
	void RecordTelemetryMugCollected(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Mug Run|Brew Rewards")
	bool TryGrantMugRunBrewReward(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry")
	void RecordTelemetryStaffSegmentGained(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry")
	void RecordTelemetryStaffSegmentSnapped(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry")
	void RecordTelemetryBonkAttempt(AWizardStaffWizardCharacter* Attacker);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry")
	void RecordTelemetryBonkLanded(AWizardStaffWizardCharacter* Attacker, AWizardStaffWizardCharacter* Victim);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Staff Clash")
	void RecordTelemetryStaffClashStarted(AWizardStaffWizardCharacter* WizardA, AWizardStaffWizardCharacter* WizardB);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Staff Clash")
	void RecordTelemetryStaffClashWon(AWizardStaffWizardCharacter* Winner, AWizardStaffWizardCharacter* Loser);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Staff Clash")
	void RecordTelemetryStaffClashTied(AWizardStaffWizardCharacter* WizardA, AWizardStaffWizardCharacter* WizardB);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry")
	void RecordTelemetryOutOfArenaRespawn(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Broom Boost")
	void RecordTelemetryBroomBoostUsed(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Arcane Pinball")
	void RecordTelemetryArcanePinballRewardReceived(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Arcane Pinball")
	void RecordTelemetryArcanePinballCast(AWizardStaffWizardCharacter* Caster, float CastStressGained);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Arcane Pinball")
	void RecordTelemetryArcanePinballHit(AWizardStaffWizardCharacter* Caster, AWizardStaffWizardCharacter* HitWizard, bool bSelfHit, float HitStressGained);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Arcane Pinball")
	void RecordTelemetryArcanePinballBounce(AWizardStaffWizardCharacter* Caster);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Mega Staff")
	void RecordTelemetryMegaStaffPickup(AWizardStaffWizardCharacter* Wizard, int32 SegmentsGranted);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Mega Staff")
	void RecordTelemetryMegaStaffSegmentSnapped(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Mega Staff")
	void RecordTelemetryMegaStaffRingOut(AWizardStaffWizardCharacter* Wizard);

	UFUNCTION(BlueprintCallable, Category = "Playtest Telemetry|Loose Segments")
	void RecordTelemetryLooseSegmentChaosHit(AWizardStaffWizardCharacter* Wizard, EWizardLooseSegmentChaosEffectType EffectType, float ManaSloshAdded);

	UFUNCTION(BlueprintPure, Category = "Playtest Telemetry")
	FWizardPlayerPlaytestStats GetPlayerPlaytestStats(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Playtest Telemetry")
	int32 GetPlaytestStatsPlayerCount() const { return PlayerPlaytestStats.Num(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Local Multiplayer", meta = (ClampMin = "2", ClampMax = "4"))
	int32 DesiredLocalPlayerCount = 2;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Online Scaffold")
	EWizardPrototypeSessionMode PrototypeSessionMode = EWizardPrototypeSessionMode::LocalPrototype;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Local Multiplayer")
	bool bUseSharedCamera = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Local Multiplayer", meta = (ClampMin = "0.0"))
	float SpawnRadius = 260.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Local Multiplayer")
	TSubclassOf<AWizardStaffSharedCamera> SharedCameraClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Local Multiplayer|Keyboard Fallback")
	FWizardKeyboardFallbackControls KeyboardFallbackControls;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Bots")
	bool bEnablePlaytestBots = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Bots")
	bool bAutoEnableSoloPlaytestBotInPIE = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Bots")
	bool bFillMissingPlayersWithBots = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "1", ClampMax = "4"))
	int32 DesiredHumanPlayers = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Bots", meta = (ClampMin = "2", ClampMax = "4"))
	int32 DesiredTotalPlayers = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Bots")
	bool bShowPlaytestBotDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Bots|Tuning")
	FWizardPlaytestBotTuning PlaytestBotTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Party Match|Tuning")
	FWizardPartyMatchTuning PartyMatchTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Party Hall|Tuning")
	FWizardPartyHallTuning PartyHallTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Staffs At Dawn|Tuning")
	FWizardStaffsAtDawnTuning StaffsAtDawnTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grand Wizard Favor|Tuning")
	FWizardGrandWizardFavorTuning GrandWizardFavorTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Final Round|Tuning")
	FWizardFinalRoundTuning FinalRoundTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Telemetry")
	bool bEnablePlaytestTelemetry = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Playtest Telemetry")
	bool bShowPlaytestTelemetryDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Tuning")
	bool bApplyTuningPresetOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Tuning")
	EWizardPrototypeTuningPreset ActivePrototypeTuningPreset = EWizardPrototypeTuningPreset::Chaotic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Tuning|Presets")
	FWizardPrototypeTuningPresetValues StableTuningPreset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Tuning|Presets")
	FWizardPrototypeTuningPresetValues ChaoticTuningPreset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Tuning|Presets")
	FWizardPrototypeTuningPresetValues AbsurdTuningPreset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Arena")
	bool bUseAuthoredPrototypeArena = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Arena")
	bool bSpawnPrototypeArena = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Arena", meta = (ClampMin = "300.0"))
	float ArenaHalfSize = 950.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Arena")
	TSubclassOf<AWizardStaffPrototypeArena> RuntimePrototypeArenaClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Staffs At Dawn|Arena")
	bool bUseAuthoredStaffsAtDawnArena = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Staffs At Dawn|Arena")
	bool bSpawnStaffsAtDawnArena = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Staffs At Dawn|Arena")
	TSubclassOf<AWizardStaffStaffsAtDawnArena> RuntimeStaffsAtDawnArenaClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Staffs At Dawn|Powerups")
	TSubclassOf<AWizardStaffStaffsAtDawnPowerupPickup> StaffsAtDawnPowerupPickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Staffs At Dawn|Arena")
	FVector RuntimeStaffsAtDawnArenaLocation = FVector(3200.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Party Hall")
	bool bUseAuthoredPartyHall = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Party Hall")
	bool bSpawnPartyHall = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Party Hall")
	TSubclassOf<AWizardStaffPartyHall> RuntimePartyHallClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Party Hall")
	FVector PartyHallSpawnLocation = FVector(0.0f, 2600.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Party Hall", meta = (ClampMin = "300.0"))
	float PartyHallFallbackHalfSize = 640.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Prototype Arena|Respawn")
	FWizardOutOfArenaRespawnTuning OutOfArenaRespawnTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mug Run|Tuning")
	FWizardMugRunTuning MugRunTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mug Run|Rematch")
	FWizardMugRunRematchTuning RematchTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mug Run|Loose Segments")
	FWizardLooseSegmentChaosTuning LooseSegmentChaosTuning;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mug Run")
	float MugRunRemainingTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mug Run")
	float MugRunPostMatchRemainingTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mug Run")
	bool bMugRunMatchActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mug Run")
	EWizardMugRunMatchState MugRunMatchState = EWizardMugRunMatchState::WaitingToStart;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mug Run")
	FString MugRunWinnerMessage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	EWizardPartyMatchState PartyMatchState = EWizardPartyMatchState::Intermission;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	EWizardTrialState ActiveTrialState = EWizardTrialState::WaitingToStart;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	EWizardTrialType ActiveTrialType = EWizardTrialType::MugRun;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	float TrialCountdownRemainingTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	float TrialResultsRemainingTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	float IntermissionRemainingTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	int32 CompletedTrialCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Party Match")
	TArray<int32> PlayerRoundWins;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grand Wizard Favor")
	TArray<int32> PlayerGrandWizardFavor;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Staffs At Dawn")
	float StaffsAtDawnRemainingTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Staffs At Dawn")
	bool bStaffsAtDawnTrialActive = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Staffs At Dawn")
	TArray<int32> StaffsAtDawnScores;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Staffs At Dawn")
	TArray<int32> StaffsAtDawnBonksTowardSegment;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Staffs At Dawn")
	FString StaffsAtDawnFeedbackMessage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Staffs At Dawn")
	float StaffsAtDawnFeedbackExpireTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Playtest Telemetry")
	TArray<FWizardPlayerPlaytestStats> PlayerPlaytestStats;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Playtest Telemetry|Staffs At Dawn")
	int32 StaffsAtDawnTelemetryRingOuts = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Playtest Telemetry|Staffs At Dawn")
	int32 StaffsAtDawnTelemetryRespawns = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Playtest Telemetry|Staffs At Dawn")
	int32 StaffsAtDawnTelemetryRoundsCompleted = 0;

	TMap<TWeakObjectPtr<AWizardStaffWizardCharacter>, float> RecentStaffsAtDawnBroomBoostTimes;
	TSet<TWeakObjectPtr<AWizardStaffWizardCharacter>> StaffsAtDawnBroomBoostRingOutThreats;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Final Round")
	float FinalRoundRemainingTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Final Round")
	int32 GrandWizardCandidatePlayerIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Final Round")
	int32 GrandWizardWinnerPlayerIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Final Round")
	FString GrandWizardWinnerMessage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Final Round")
	FString GrandWizardCandidateIntroMessage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Final Round")
	FString GrandWizardFinalStaffSetupMessage;

protected:
	void ApplyPlaytestBotPIEDefaults();
	EWizardPrototypeSessionMode DetectPrototypeSessionMode() const;
	void RefreshPrototypeSessionMode(const TCHAR* Reason);
	bool IsStandaloneLocalPrototypeSession() const;
	bool ShouldHoldOnlineIntermissionForPlayers() const;
	int32 GetConnectedPlayerControllerCount() const;
	void AssignOnlineScaffoldPlayerSlot(AController* Controller);
	void EnsureLocalPlayers();
	void SyncPlaytestBots();
	void UpdateKeyboardFallbackControls();
	void SpawnSharedCamera();
	void AssignSharedCameraToAllPlayers();
	void AssignSharedCameraToPlayer(APlayerController* PlayerController);
	FTransform GetSpawnTransformForController(const AController* Controller) const;
	FTransform GetPartyHallSpawnTransformForController(const AController* Controller) const;
	int32 GetControllerIndex(const AController* Controller) const;
	void SpawnPrototypeArena();
	AWizardStaffPrototypeArena* FindAuthoredPrototypeArena() const;
	void SpawnStaffsAtDawnArena();
	AWizardStaffStaffsAtDawnArena* FindAuthoredStaffsAtDawnArena() const;
	bool ShouldUseStaffsAtDawnArena() const;
	FString GetStaffsAtDawnArenaSourceText() const;
	int32 GetStaffsAtDawnArenaPlayerSpawnCount() const;
	int32 GetStaffsAtDawnArenaFuturePowerupSpawnCount() const;
	void DrawStaffsAtDawnArenaDebug() const;
	void SpawnPartyHall();
	AWizardStaffPartyHall* FindAuthoredPartyHall() const;
	void UpdatePartyHallSigns() const;
	void ResetPartyHallReadyStates();
	void EnsurePartyHallReadyStateSize(int32 MinimumPlayerCount = 0);
	bool AreAllActivePartyHallPlayersReady() const;
	void SetPartyHallReadyFeedbackMessage(const FString& Message, const FColor& MessageColor);
	void SpawnLegacyRuntimeArenaBlocks();
	AStaticMeshActor* SpawnArenaBlock(FName Name, const FVector& Location, const FVector& Scale) const;
	FVector GetArenaCenter() const;
	FVector GetCurrentPlayBoundsCenter() const;
	float GetCurrentPlayBoundsHalfSize() const;
	bool IsPartyHallActive() const;
	float GetCurrentOutOfArenaFallZThreshold() const;
	void UpdateOutOfArenaRespawns(float DeltaSeconds);
	void ClearPendingOutOfArenaRespawns(bool bNotifyWizards = true);
	bool IsWizardOutOfArena(const AWizardStaffWizardCharacter* Wizard) const;
	void RespawnWizardInArena(AWizardStaffWizardCharacter* Wizard) const;
	void EnterPartyHallIntermission();
	void StartTrialCountdown(EWizardTrialType TrialType);
	void StartActiveTrial();
	void FinishActiveTrialResults();
	EWizardTrialType GetTrialTypeForTrialIndex(int32 TrialIndex) const;
	void RespawnWizardsForTrialStart();
	void SetWizardPrototypeInputsLocked(bool bLocked) const;
	void ResetWizardStaffsForTrialStart();
	void ResetStaffsAtDawnForNewTrial();
	void RespawnWizardsForStaffsAtDawn();
	void EnsureStaffsAtDawnScoreSize(int32 MinimumPlayerCount = 0);
	void AddStaffsAtDawnScore(int32 PlayerIndex, int32 Points, const FString& Reason);
	void AddStaffsAtDawnBonkSegmentProgress(int32 PlayerIndex);
	void GrantStaffsAtDawnCombatSegments(int32 PlayerIndex, int32 SegmentCount, const FString& Reason);
	void SetStaffsAtDawnFeedbackMessage(const FString& Message, const FColor& MessageColor);
	void InitializeStaffsAtDawnPowerups();
	void UpdateStaffsAtDawnPowerups(float DeltaSeconds);
	void ResetStaffsAtDawnPowerups();
	void SpawnStaffsAtDawnPowerupAtIndex(int32 SpawnIndex);
	void RemoveStaffsAtDawnPowerupAtIndex(int32 SpawnIndex, bool bScheduleRespawn, float RespawnDelayOverride = -1.0f);
	TArray<FVector> GetStaffsAtDawnPowerupSpawnLocations() const;
	FVector ChooseStaffsAtDawnPowerupSpawnLocationForSlot(int32 SpawnIndex) const;
	bool IsStaffsAtDawnPowerupLocationReserved(const FVector& CandidateLocation, int32 IgnoredSpawnIndex) const;
	void AnnounceStaffsAtDawnWinner();
	void EnterGrandWizardFinalRound();
	void UpdateGrandWizardFinalRound(float DeltaSeconds);
	void FinishGrandWizardFinalRound();
	void ResetGrandWizardFinalRoundState();
	void SpawnOrUpdateFinalRoundCircleVisual();
	void SetFinalRoundCircleVisible(bool bNewVisible);
	void DrawGrandWizardFinalWorldMarkers() const;
	FVector GetFinalRoundCircleCenter() const;
	bool IsWizardInsideFinalRoundCircle(const AWizardStaffWizardCharacter* Wizard, float Radius) const;
	bool IsWizardEligibleForGrandWizardSteal(const AWizardStaffWizardCharacter* Wizard) const;
	int32 SelectInitialGrandWizardCandidate() const;
	FString BuildGrandWizardCandidateIntroMessage(int32 PlayerIndex) const;
	int32 CalculateFinalStartingSegments(int32 PlayerIndex, bool bCandidate) const;
	void ApplyFavorBasedFinalStaffSetup();
	void SetWizardStaffSegmentCount(AWizardStaffWizardCharacter* Wizard, int32 TargetSegmentCount, bool bApplyManaSloshForSegments = false) const;
	int32 GetBestGrandWizardCircleChallenger() const;
	void SetGrandWizardCandidate(int32 PlayerIndex, const FString& Reason);
	void SetGrandWizardFinalFeedbackMessage(const FString& Message, const FColor& MessageColor);
	void RespawnWizardsForFinalRound();
	void ResetMugRunForNewMatch();
	void ResetWizardsForNewMatch();
	void ResetMugRunPickups();
	void UpdateLooseSnappedSegments(float DeltaSeconds);
	void PruneLooseSnappedSegments();
	void EnforceLooseSnappedSegmentBudget();
	void RemoveLooseSnappedSegmentAt(int32 SegmentIndex);
	void StartLooseSnappedSegmentFadeOrDestroy(int32 SegmentIndex, bool bImmediateDestroy);
	UFUNCTION()
	void HandleLooseSegmentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	int32 FindTrackedLooseSegmentIndex(AActor* LooseSegment, UPrimitiveComponent* HitComponent = nullptr) const;
	float GetLooseSegmentImpactSpeed(const FWizardTrackedLooseSegment& TrackedSegment, const FVector& NormalImpulse) const;
	EWizardLooseSegmentChaosEffectType ChooseLooseSegmentChaosEffect(float ImpactSpeed, bool bHitWizard) const;
	bool CanLooseSegmentAffectWizard(const FWizardTrackedLooseSegment& TrackedSegment, const AWizardStaffWizardCharacter* Wizard) const;
	bool ApplyLooseSegmentChaosEffect(int32 SegmentIndex, AWizardStaffWizardCharacter* HitWizard, EWizardLooseSegmentChaosEffectType EffectType, const FVector& ImpactLocation, const FVector& ImpactDirection);
	bool TriggerLooseSegmentArcanePop(int32 SegmentIndex, const FVector& ImpactLocation, const FVector& ImpactDirection);
	void SpawnMugRunPickups();
	void SetMugRunPickupsActive(bool bNewActive);
	TArray<FVector> GetDefaultMugSpawnLocations() const;
	TArray<AWizardStaffWizardCharacter*> GetCurrentWizards() const;
	AWizardStaffWizardCharacter* GetWizardForPlayerIndex(int32 PlayerIndex) const;
	const FWizardPrototypeTuningPresetValues& GetPrototypeTuningPresetValues(EWizardPrototypeTuningPreset Preset) const;
	void ApplyPrototypeTuningPresetValues(const FWizardPrototypeTuningPresetValues& PresetValues);
	void ApplyPrototypeTuningPresetToWizard(AWizardStaffWizardCharacter* Wizard, const FWizardPrototypeTuningPresetValues& PresetValues) const;
	void ApplyPrototypeTuningPresetToPickup(AWizardStaffManaMugPickup* MugPickup, const FWizardPrototypeTuningPresetValues& PresetValues) const;
	void AnnouncePrototypeTuningPreset() const;
	void EnsurePlayerRoundWinsSize(int32 MinimumPlayerCount = 0);
	void EnsurePlayerGrandWizardFavorSize(int32 MinimumPlayerCount = 0);
	void AddGrandWizardFavor(int32 PlayerIndex, int32 FavorAmount, const FString& Reason);
	void SetGrandWizardFavorFeedbackMessage(const FString& Message, const FColor& MessageColor);
	void EnsurePlayerPlaytestStatsSize(int32 MinimumPlayerCount = 0);
	FWizardPlayerPlaytestStats* FindOrAddPlayerPlaytestStats(int32 PlayerIndex);
	bool HasRecentStaffsAtDawnBroomBoost(AWizardStaffWizardCharacter* Wizard, float CurrentTime) const;
	void MarkStaffsAtDawnBroomBoostRingOutThreat(AWizardStaffWizardCharacter* Wizard);
	void RecordTelemetryBroomBoostRingOutSave(AWizardStaffWizardCharacter* Wizard);
	void RecordTelemetryBroomBoostFailedRingOutRecovery(AWizardStaffWizardCharacter* Wizard);
	void SyncPlaytestTelemetryRoundWins();
	void AddGrandWizardCandidateTime(float DeltaSeconds);
	void CaptureFinalPlaytestTelemetrySnapshot();
	void LogPlaytestTelemetrySummary() const;
	static FString GetPrototypeTuningPresetText(EWizardPrototypeTuningPreset Preset);
	static FString GetPartyMatchStateText(EWizardPartyMatchState State);
	static FString GetTrialStateText(EWizardTrialState State);
	static FString GetTrialTypeText(EWizardTrialType TrialType);
	void UpdateLeaderHighlights() const;
	void DrawMugRunDebug() const;
	void AnnounceMugRunWinner();
	void CleanupArcanePinballProjectiles();
	AWizardStaffGameState* GetWizardStaffGameState() const;
	AWizardStaffPlayerState* GetWizardStaffPlayerStateForIndex(int32 PlayerIndex) const;
	void SyncReplicatedPlayerStateForIndex(int32 PlayerIndex);
	void SyncAllReplicatedPlayerStates();
	void BumpMatchSessionGeneration();

private:
	UPROPERTY(Transient)
	TObjectPtr<AWizardStaffSharedCamera> SharedCamera;

	UPROPERTY(Transient)
	TObjectPtr<AWizardStaffPrototypeArena> ActivePrototypeArena;

	UPROPERTY(Transient)
	TObjectPtr<AWizardStaffStaffsAtDawnArena> ActiveStaffsAtDawnArena;

	UPROPERTY(Transient)
	TObjectPtr<AWizardStaffPartyHall> ActivePartyHall;

	UPROPERTY(Transient)
	TObjectPtr<AWizardStaffFinalRitualCircle> FinalRoundCircleActor;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AWizardStaffManaMugPickup>> SpawnedMugs;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AWizardStaffStaffsAtDawnPowerupPickup>> SpawnedStaffsAtDawnPowerups;

	UPROPERTY(Transient)
	TArray<FWizardTrackedLooseSegment> LooseSnappedSegments;

	TMap<TWeakObjectPtr<AWizardStaffWizardCharacter>, float> PendingOutOfArenaRespawns;

	float GrandWizardStealHoldTime = 0.0f;
	float GrandWizardStartBonusRemainingTime = 0.0f;
	int32 GrandWizardStealPlayerIndex = INDEX_NONE;
	float GrandWizardFinalFeedbackExpireTime = 0.0f;
	FString GrandWizardFinalFeedbackMessage;
	float GrandWizardFavorFeedbackExpireTime = 0.0f;
	FString GrandWizardFavorFeedbackMessage;
	TArray<uint8> PartyHallReadyPlayers;
	float PartyHallReadyFeedbackExpireTime = 0.0f;
	FString PartyHallReadyFeedbackMessage;
	bool bPartyHallAllReadyTriggered = false;

	TMap<TWeakObjectPtr<AWizardStaffWizardCharacter>, int32> RecentBonkAttackerPlayerIndexes;
	TMap<TWeakObjectPtr<AWizardStaffWizardCharacter>, float> RecentBonkTimes;
	TMap<TWeakObjectPtr<AWizardStaffWizardCharacter>, bool> RecentBonkWasStaffClash;
	TArray<FVector> StaffsAtDawnPowerupSpawnLocations;
	TArray<float> StaffsAtDawnPowerupRespawnTimers;

	bool bHasAppliedPrototypeTuningPreset = false;
	bool bPrototypeSessionModeLogged = false;
	bool bLoggedWaitingForOnlinePlayers = false;
	bool bWizardsStagedForActiveTrial = false;
};
