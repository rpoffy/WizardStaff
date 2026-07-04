#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WizardStaffWizardCharacter.generated.h"

class UStaticMeshComponent;
class UProceduralMeshComponent;
class UMaterialInstanceDynamic;
class USceneComponent;
class USoundBase;
class UWizardStaffComponent;
class UWizardStaffPlaytestBotComponent;
class AWizardStaffArcanePinballProjectile;

UENUM(BlueprintType)
enum class EWizardBrewRewardType : uint8
{
	None,
	ArcanePinball
};

USTRUCT(BlueprintType)
struct FWizardArcanePinballTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 1250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "1.0"))
	float SpeedMultiplierPerWallBounce = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0.0"))
	float MaxProjectileSpeed = 4200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball")
	bool bLockHeightToLaunchHeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0"))
	int32 MaxBounces = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0.1"))
	float Lifetime = 5.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0.0"))
	float HitKnockback = 560.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0.0"))
	float SloshOnHit = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0.0"))
	float StressOnCast = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball", meta = (ClampMin = "0.0"))
	float StressOnHit = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball")
	bool bAllowSelfHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcane Pinball")
	bool bDestroyOnPlayerHit = false;
};

USTRUCT(BlueprintType)
struct FWizardManaSloshTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshGainedPerDrink = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshGainedPerStaffSegment = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "1.0"))
	float MaxSlosh = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshDecayPerSecond = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TurnPenaltyPerSlosh = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovementPenaltyPerSlosh = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccelerationPenaltyPerSlosh = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrakingPenaltyPerSlosh = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinMovementMultiplier = 0.62f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinTurnMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinAccelerationMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinBrakingMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshOversteerDegreesPerSecond = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshOversteerRecoverySpeed = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float StumbleChancePerSecondAtMaxSlosh = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StumbleMinSloshAlpha = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float StumbleTurnKickDegreesPerSecond = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float StumbleCooldown = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshVisualIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshVisualLeanDegrees = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshStaffWobbleDegrees = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshVisualWobbleFrequency = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana Slosh", meta = (ClampMin = "0.0"))
	float SloshReducedOnStaffSnap = 11.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DebugLowSloshAlpha = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DebugMediumSloshAlpha = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DebugHighSloshAlpha = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Mana Slosh", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DebugAbsurdSloshAlpha = 1.0f;
};

USTRUCT(BlueprintType)
struct FWizardBonkTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float BaseRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float RangePerStaffSegment = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float KnockbackStrength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float KnockbackPerManaSlosh = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float KnockbackPerStaffSegment = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float MaxKnockbackStrength = 1250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float UpwardBoost = 145.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float Cooldown = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float StaffContactPadding = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk|Animation")
	float StrikeStartPitchDegrees = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk|Animation")
	float StrikeEndPitchDegrees = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk|Animation", meta = (ClampMin = "0.0"))
	float StrikeSideWobbleDegrees = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.01"))
	float VisualDuration = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk|Animation", meta = (ClampMin = "0.1"))
	float StrikeEaseExponent = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float HitStressMultiplier = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float WhiffStressMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "0.0"))
	float ImpactFeedbackDuration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk", meta = (ClampMin = "1.0"))
	float ImpactFeedbackRadius = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk")
	bool bShowImpactFeedback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk|Audio")
	TObjectPtr<USoundBase> BonkSwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk|Audio")
	TObjectPtr<USoundBase> BonkHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonk")
	bool bShowDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardStaffClashTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash")
	bool bEnableStaffClash = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "0.1"))
	float ClashDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "0.0"))
	float ClashTimingWindow = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float FacingDotThreshold = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float OpposingForwardDotThreshold = -0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "1.0"))
	float WinnerKnockbackMultiplier = 1.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "0.0"))
	float WinnerKnockbackMultiplierPerStaffSegment = 0.035f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "1.0"))
	float WinnerMaxKnockbackMultiplier = 2.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "0.0"))
	float WinnerUpwardBoostMultiplier = 1.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InputLockMultiplier = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Clash")
	bool bShowDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardHitReactionTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float StumbleDuration = 0.38f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float KnockbackScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float SloshToStumbleMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float RecoveryTime = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float StrongHitThreshold = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float StrongHitControlLossDuration = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StrongHitControlMultiplier = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StumbleControlMultiplier = 0.48f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RecoveryControlMultiplier = 0.82f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	bool bEnableKnockdown = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float KnockdownThreshold = 1180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float KnockdownDuration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KnockdownControlMultiplier = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float StumbleVisualLeanDegrees = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (ClampMin = "0.0"))
	float KnockdownVisualLeanDegrees = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	bool bShowDebug = true;
};

USTRUCT(BlueprintType)
struct FWizardStaffHeftTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft", meta = (ClampMin = "0"))
	int32 SegmentCountBeforeHeftPenalty = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovementPenaltyPerHeavySegment = 0.035f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TurnPenaltyPerHeavySegment = 0.055f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float MaxMovementPenalty = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float MaxTurnPenalty = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft|Bonk", meta = (ClampMin = "0.0"))
	float BonkCooldownPerHeavySegment = 0.045f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft|Bonk", meta = (ClampMin = "0.0"))
	float BonkVisualDurationPerHeavySegment = 0.018f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft|Bonk", meta = (ClampMin = "0.0"))
	float MaxBonkCooldownBonus = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Staff Heft|Bonk", meta = (ClampMin = "0.0"))
	float MaxBonkVisualDurationBonus = 0.25f;
};

USTRUCT(BlueprintType)
struct FWizardBroomBoostTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost")
	bool bEnableBroomBoost = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost", meta = (ClampMin = "0.1"))
	float BoostDuration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost", meta = (ClampMin = "0.0"))
	float ForwardSpeed = 1050.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost", meta = (ClampMin = "0.0"))
	float InitialForwardBoost = 720.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost")
	float InitialUpwardBoost = 145.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost", meta = (ClampMin = "0.0"))
	float VelocityLerpSpeed = 7.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost")
	float MinVerticalVelocityDuringBoost = -80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Control", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float SloshControlPenaltyAtMax = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Control", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float StaffHeftControlPenaltyPerHeavySegment = 0.075f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Control", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float MaxStaffHeftControlPenalty = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Control", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float MinControlMultiplier = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Control", meta = (ClampMin = "0.0"))
	float LandingCooldown = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder")
	bool bEnableHighSloshDisorder = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder", meta = (ClampMin = "0.0"))
	float HighSloshDisorderThreshold = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder", meta = (ClampMin = "0.0"))
	float FullDisorderSlosh = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDisorderAlphaAtThreshold = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HighSloshDisorderChanceAtFullSlosh = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxDisorderYawDegrees = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder", meta = (ClampMin = "0.0"))
	float MaxDisorderSideImpulse = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|High Mana Slosh Disorder", meta = (ClampMin = "0.0"))
	float MaxDisorderVerticalImpulse = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Visual")
	FVector BroomLocalLocation = FVector(0.0f, 0.0f, -82.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Visual")
	FVector HandleScale = FVector(1.15f, 0.06f, 0.06f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Visual")
	FVector BristleLocalLocation = FVector(-54.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost|Visual")
	FVector BristleScale = FVector(0.28f, 0.24f, 0.13f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Broom Boost")
	bool bShowDebug = true;
};

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffWizardCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWizardStaffWizardCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Wizard|Visuals")
	void ApplyPlayerColor(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Visuals")
	void SetLeaderHighlight(bool bNewHighlighted);

	UFUNCTION(BlueprintPure, Category = "Wizard|Visuals")
	static FLinearColor GetPrototypePlayerColor(int32 PlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Mug")
	void DrinkMug();

	// Compatibility wrapper for older prototype Blueprint/input references.
	UFUNCTION(BlueprintCallable, Category = "Wizard|Mug", meta = (DeprecatedFunction, DeprecationMessage = "Use DrinkMug. Mana is no longer a spendable resource; mugs grant staff growth, Mana Slosh, and optional brew rewards."))
	void DrinkManaMug();

	UFUNCTION(BlueprintCallable, Category = "Wizard|Match")
	void ResetForNewMatch(bool bResetStaffSegments, bool bResetSlosh);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Mana Slosh", meta = (DisplayName = "Add Mana Slosh"))
	void AddManaSlosh(float SloshAmount);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Mana Slosh", meta = (DisplayName = "Add Mana Slosh For Staff Growth"))
	void AddManaSloshForStaffGrowth(int32 SegmentCount, FName GrowthSource);

	UFUNCTION(BlueprintPure, Category = "Wizard|Mana Slosh", meta = (DisplayName = "Get Mana Slosh Alpha"))
	float GetManaSloshAlpha() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableManaSlosh() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableMaxManaSlosh() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableManaSloshAlpha() const;

	void SyncReplicatedManaSloshFromAuthority(bool bForce = false);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Mana Slosh", meta = (DisplayName = "Set Mana Slosh Locked"))
	void SetManaSloshLocked(bool bNewLocked);

	UFUNCTION(BlueprintPure, Category = "Wizard|Mana Slosh", meta = (DisplayName = "Is Mana Slosh Locked"))
	bool IsManaSloshLocked() const { return bManaSloshLocked; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Staff")
	int32 GetStaffSegmentCount() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableStaffStress() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableMaxStaffStress() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableStaffStressAlpha() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	int32 GetReplicatedStaffSegmentCount() const { return ReplicatedStaffSegmentCount; }

	void SyncReplicatedStaffSegmentCountFromAuthority();
	void SyncReplicatedStaffStressFromAuthority(bool bForce = false);
	void SyncReplicatedCarriedBrewRewardFromAuthority();
	void SyncReplicatedMegaStaffStateFromAuthority(bool bForce = false);

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	bool IsReadableOutOfArenaRespawning() const { return bReplicatedOutOfArenaRespawning; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableOutOfArenaRespawnRemainingTime() const { return ReplicatedOutOfArenaRespawnRemainingTime; }

	void SyncReplicatedOutOfArenaRespawnStateFromAuthority(bool bNewRespawning, float RespawnTimeRemaining, bool bForce = false);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Bonk")
	void QuickBonk();

	UFUNCTION(BlueprintCallable, Category = "Wizard|Brew Reward")
	void UseReward();

	UFUNCTION(BlueprintCallable, Category = "Wizard|Brew Reward")
	bool TryGrantBrewReward(EWizardBrewRewardType RewardType, bool bReplaceExistingReward);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Brew Reward")
	void ClearCarriedBrewReward();

	void CancelQuickBonkStateForReset();
	void CancelStaffClashStateForReset();
	void CancelMovementStateForRespawn();

	UFUNCTION(BlueprintPure, Category = "Wizard|Brew Reward")
	EWizardBrewRewardType GetCarriedBrewReward() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Brew Reward")
	bool HasCarriedBrewReward() const { return GetCarriedBrewReward() != EWizardBrewRewardType::None; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Brew Reward")
	FString GetCarriedBrewRewardName() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	EWizardBrewRewardType GetReplicatedCarriedBrewReward() const { return ReplicatedCarriedBrewReward; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Brew Reward")
	static FString GetBrewRewardDisplayName(EWizardBrewRewardType RewardType);

	UFUNCTION(BlueprintPure, Category = "Wizard|Brew Reward")
	static FLinearColor GetBrewRewardGlowColor(EWizardBrewRewardType RewardType);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Input")
	void ApplyPrototypeLocalInput(float ForwardValue, float RightValue, float TurnValue);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Input")
	void SetPrototypeInputLocked(bool bNewLocked);

	UFUNCTION(BlueprintPure, Category = "Wizard|Input")
	bool IsPrototypeInputLocked() const { return bPrototypeInputLocked; }

	UFUNCTION(BlueprintCallable, Category = "Wizard|Playtest Bot")
	void SetPlaytestBot(bool bNewPlaytestBot);

	UFUNCTION(BlueprintPure, Category = "Wizard|Playtest Bot")
	bool IsPlaytestBot() const { return bIsPlaytestBot; }

	UFUNCTION(BlueprintCallable, Category = "Wizard|Playtest Bot")
	void BotPressJump();

	UFUNCTION(BlueprintPure, Category = "Wizard|Playtest Bot")
	bool CanBotBroomBoost() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Bonk")
	float GetQuickBonkRange() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Bonk")
	float GetQuickBonkKnockbackStrength() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Bonk")
	float GetQuickBonkCooldown() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Bonk")
	float GetQuickBonkImpactDelay() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Bonk")
	float GetQuickBonkVisualDuration() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Bonk")
	bool CanQuickBonk() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Staff Clash")
	bool IsInStaffClash() const { return bStaffClashActive; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Staff Clash")
	int32 GetStaffClashMashCount() const { return StaffClashMashCount; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	bool IsReadableStaffClashActive() const { return bStaffClashActive; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	float GetReadableStaffClashRemainingTime() const { return StaffClashTimeRemaining; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Staff Clash")
	bool CanReceiveBonkOrSpellHits() const { return !bStaffClashActive; }

	UFUNCTION(BlueprintCallable, Category = "Wizard|Staffs At Dawn Powerup")
	int32 ActivateMegaStaffBrew(int32 BonusSegments, float Duration, float StressMultiplierDuringEffect, float KnockbackMultiplierDuringEffect, bool bRemoveTemporarySegmentsOnExpire, bool bShowDebug);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Staffs At Dawn Powerup")
	void ClearMegaStaffBrew(bool bRemoveTemporarySegments);

	UFUNCTION(BlueprintCallable, Category = "Wizard|Staffs At Dawn Powerup")
	void NotifyMegaStaffSegmentSnapped();

	UFUNCTION(BlueprintPure, Category = "Wizard|Staffs At Dawn Powerup")
	bool IsMegaStaffBrewActive() const { return bMegaStaffBrewActive; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Staffs At Dawn Powerup")
	float GetMegaStaffRemainingTime() const { return MegaStaffTimeRemaining; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Online Scaffold")
	int32 GetReadableMegaStaffTemporarySegmentCount() const { return MegaStaffTemporarySegmentsRemaining; }

	UFUNCTION(BlueprintPure, Category = "Wizard|Staffs At Dawn Powerup")
	float GetStaffStressEffectMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Staffs At Dawn Powerup")
	float GetMegaStaffKnockbackMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Wizard|Hit Reaction")
	void ApplyBonkReaction(FVector KnockbackDirection, float IncomingKnockbackStrength, float IncomingUpwardBoost, int32 AttackerStaffSegments);

	UFUNCTION(BlueprintPure, Category = "Wizard|Hit Reaction")
	bool IsReactingToHit() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Staff Heft")
	float GetStaffHeftMovementMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Wizard|Staff Heft")
	float GetStaffHeftTurnMultiplier() const;

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugAddStaffSegment();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugRemoveStaffSegment();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugResetSlosh();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugMaxStaffStress();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugForceSnapTopSegment();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugRestartMugRunMatch();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugCyclePrototypeTuningPreset();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void CycleWizardHudMode();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugSetLowSlosh();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugSetMediumSlosh();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugSetHighSlosh();

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard|Debug")
	void DebugSetAbsurdSlosh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Visuals")
	TObjectPtr<UStaticMeshComponent> RobeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Visuals")
	TObjectPtr<UStaticMeshComponent> HatMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Visuals")
	TObjectPtr<UStaticMeshComponent> FaceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Visuals")
	TObjectPtr<UProceduralMeshComponent> PlayerMarkerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Visuals")
	TObjectPtr<UStaticMeshComponent> LeaderMarkerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Brew Reward")
	TObjectPtr<UStaticMeshComponent> SpellbookMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Brew Reward")
	TObjectPtr<UStaticMeshComponent> SpellbookGlowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Broom Boost")
	TObjectPtr<UStaticMeshComponent> BroomHandleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Broom Boost")
	TObjectPtr<UStaticMeshComponent> BroomBristleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Staff")
	TObjectPtr<USceneComponent> StaffRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Staff")
	TObjectPtr<UWizardStaffComponent> StaffComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard|Playtest Bot")
	TObjectPtr<UWizardStaffPlaytestBotComponent> PlaytestBotComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Movement", meta = (ClampMin = "0.0", ToolTip = "Global movement input multiplier before Slosh, Staff Heft, staff obstruction, and hit reaction penalties bend control."))
	float MoveInputScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Movement", meta = (ClampMin = "0.0", ToolTip = "Base wizard turn rate used by keyboard, mouse, gamepad stick, and the player-2 keyboard fallback after input axis scaling."))
	float TurnRateDegreesPerSecond = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Movement", meta = (ClampMin = "0.0", ToolTip = "Base walk speed. Slosh, Staff Heft, staff obstruction, and hit reactions should make this feel bent, not unreadable."))
	float WalkSpeed = 520.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Movement", meta = (ClampMin = "0.0"))
	float HopVelocity = 420.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Movement", meta = (ClampMin = "0.0", ToolTip = "Base acceleration for movement response before Slosh acceleration penalties are applied."))
	float MaxAcceleration = 2048.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Movement", meta = (ClampMin = "0.0", ToolTip = "Base braking for stop response before Slosh braking penalties are applied."))
	float BrakingDecelerationWalking = 1600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability")
	bool bShowPlayerGroundMarker = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability")
	FVector PlayerMarkerScale = FVector(0.68f, 0.68f, 0.025f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability")
	float PlayerMarkerZOffset = -87.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability", meta = (ClampMin = "1.0", ToolTip = "Local radius of the flat circle-and-point player ground marker before PlayerMarkerScale is applied."))
	float PlayerMarkerCircleRadius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability", meta = (ClampMin = "0.0", ToolTip = "How far the integrated facing point extends forward from the circle."))
	float PlayerMarkerPointLength = 42.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability", meta = (ClampMin = "0.0", ToolTip = "Half width of the integrated facing point where it tucks into the circle."))
	float PlayerMarkerPointHalfWidth = 32.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability", meta = (ClampMin = "0.0", ToolTip = "How far the facing point base overlaps inside the circle so the marker reads as one combined shape."))
	float PlayerMarkerPointBaseInset = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability", meta = (ClampMin = "12", ClampMax = "96"))
	int32 PlayerMarkerCircleSegments = 40;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability")
	FVector LeaderMarkerScale = FVector(0.22f, 0.22f, 0.26f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability")
	float LeaderMarkerHeight = 136.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Readability")
	FLinearColor LeaderMarkerColor = FLinearColor(1.0f, 0.92f, 0.08f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Staffs At Dawn Powerup")
	FLinearColor MegaStaffVisualColor = FLinearColor(0.18f, 1.0f, 0.35f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Staffs At Dawn Powerup", meta = (ClampMin = "1.0"))
	float MegaStaffMarkerScaleMultiplier = 1.65f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Staffs At Dawn Powerup", meta = (ClampMin = "0.0"))
	float MegaStaffMarkerPulseScale = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Staffs At Dawn Powerup", meta = (ClampMin = "0.0"))
	float MegaStaffExpireWarningTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Brew Reward")
	FVector SpellbookLocalLocation = FVector(16.0f, -43.0f, 18.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Brew Reward")
	FRotator SpellbookLocalRotation = FRotator(8.0f, 0.0f, -12.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Brew Reward")
	FVector SpellbookScale = FVector(0.26f, 0.08f, 0.34f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Brew Reward")
	FVector SpellbookGlowScale = FVector(0.34f, 0.18f, 0.44f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Brew Reward")
	FLinearColor SpellbookInactiveColor = FLinearColor(0.18f, 0.12f, 0.07f, 1.0f);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Mana Slosh", meta = (DisplayName = "Mana Slosh"))
	float ManaSlosh = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Mana Slosh")
	bool bManaSloshLocked = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Mana Slosh")
	float LockedManaSlosh = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Brew Reward")
	EWizardBrewRewardType CarriedBrewReward = EWizardBrewRewardType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Tuning", meta = (DisplayName = "Mana Slosh Tuning"))
	FWizardManaSloshTuning ManaTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Debug")
	bool bShowWizardDebug = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Playtest Bot")
	bool bIsPlaytestBot = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffSegmentCount, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	int32 ReplicatedStaffSegmentCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedManaSlosh, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedManaSlosh = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedMaxManaSlosh = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffStress, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedStaffStress = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedMaxStaffStress = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedCarriedBrewReward, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	EWizardBrewRewardType ReplicatedCarriedBrewReward = EWizardBrewRewardType::None;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedQuickBonkSequence, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	int32 ReplicatedQuickBonkSequence = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedQuickBonkVisualDuration = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedQuickBonkCancelSequence, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	int32 ReplicatedQuickBonkCancelSequence = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedOutOfArenaRespawnState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	bool bReplicatedOutOfArenaRespawning = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedOutOfArenaRespawnRemainingTime = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffClashState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	bool bReplicatedStaffClashActive = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffClashState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	TObjectPtr<AWizardStaffWizardCharacter> ReplicatedStaffClashOpponent = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffClashState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	int32 ReplicatedStaffClashSequence = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffClashState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedStaffClashRemainingTime = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffClashState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	int32 ReplicatedStaffClashMashCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaffClashState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	FVector ReplicatedStaffClashLockedLocation = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedMegaStaffState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	bool bReplicatedMegaStaffActive = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedMegaStaffState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	float ReplicatedMegaStaffRemainingTime = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedMegaStaffState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	int32 ReplicatedMegaStaffTemporarySegmentCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedMegaStaffState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	int32 ReplicatedMegaStaffSequence = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedBroomBoostActive, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	bool bReplicatedBroomBoostActive = false;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPrototypeInputLocked, VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Online Scaffold")
	bool bReplicatedPrototypeInputLocked = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Tuning")
	FWizardBonkTuning BonkTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Tuning")
	FWizardStaffClashTuning StaffClashTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Tuning")
	FWizardHitReactionTuning HitReactionTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Tuning")
	FWizardStaffHeftTuning StaffHeftTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Tuning")
	FWizardBroomBoostTuning BroomBoostTuning;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Brew Reward")
	TSubclassOf<AWizardStaffArcanePinballProjectile> ArcanePinballProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard|Brew Reward")
	FWizardArcanePinballTuning ArcanePinballTuning;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Hit Reaction")
	float HitStumbleTimeRemaining = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Hit Reaction")
	float HitRecoveryTimeRemaining = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Hit Reaction")
	float HitControlLossTimeRemaining = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard|Hit Reaction")
	float HitKnockdownTimeRemaining = 0.0f;

protected:
	void HandleJumpPressed();
	void HandleJumpReleased();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void RefreshColorFromPlayerState();
	void RebuildStaffVisualsFromReplicatedSegmentCount();

	UFUNCTION()
	void OnRep_ReplicatedStaffSegmentCount();

	UFUNCTION()
	void OnRep_ReplicatedManaSlosh();

	UFUNCTION()
	void OnRep_ReplicatedStaffStress();

	UFUNCTION()
	void OnRep_ReplicatedCarriedBrewReward();

	UFUNCTION()
	void OnRep_ReplicatedQuickBonkSequence();

	UFUNCTION()
	void OnRep_ReplicatedQuickBonkCancelSequence();

	UFUNCTION()
	void OnRep_ReplicatedOutOfArenaRespawnState();

	UFUNCTION()
	void OnRep_ReplicatedStaffClashState();

	UFUNCTION()
	void OnRep_ReplicatedMegaStaffState();

	UFUNCTION()
	void OnRep_ReplicatedBroomBoostActive();

	UFUNCTION()
	void OnRep_ReplicatedPrototypeInputLocked();

	UFUNCTION(Server, Unreliable)
	void ServerSetFacingYaw(float NewYaw);

	UFUNCTION(Server, Reliable)
	void ServerUseReward();

	UFUNCTION(Server, Reliable)
	void ServerRequestQuickBonk();

	UFUNCTION(Server, Reliable)
	void ServerRequestBroomBoost();

	void UpdateManaSlosh(float DeltaSeconds);
	void UpdateHitReaction(float DeltaSeconds);
	void UpdateBroomBoost(float DeltaSeconds);
	void UpdateMegaStaffBrew(float DeltaSeconds);
	void UpdateStaffClash(float DeltaSeconds);
	void ExpireMegaStaffBrew();
	void UpdateMegaStaffVisual(float DeltaSeconds);
	bool CanBroomBoost() const;
	void ActivateBroomBoost();
	void EndBroomBoost();
	void SetBroomBoostVisualActive(bool bNewActive);
	float GetBroomBoostControlMultiplier() const;
	bool IsPrototypeInputBlockedForLocalInput() const;
	void UpdateSloshMovementSettings();
	void UpdateSloshVisuals(float DeltaSeconds);
	void UpdateBonkAttack(float DeltaSeconds);
	void UpdateBonkVisual(float DeltaSeconds);
	void ResolveQuickBonkHit();
	int32 PerformQuickBonkHitDetection();
	bool ApplyBonkToTarget(AWizardStaffWizardCharacter* TargetWizard);
	void HandleQuickBonkRequestOnServer();
	bool StartQuickBonkOnAuthority();
	void SyncReplicatedQuickBonkStartFromAuthority(float VisualDuration);
	void ClearQuickBonkState(bool bSyncReplicatedCancel);
	void HandleNetworkedUseRewardRequestOnServer();
	void SyncReplicatedStaffClashStateFromAuthority(bool bForce = false);
	bool TryStartStaffClashWith(AWizardStaffWizardCharacter* TargetWizard);
	bool CanStartStaffClashWith(const AWizardStaffWizardCharacter* TargetWizard) const;
	void ResolveStaffClash();
	void ClearStaffClashState(bool bSyncReplicatedState = true);
	void RegisterStaffClashMash();
	void LockStaffClashPosition();
	void ApplyStaffClashWinningBonkToTarget(AWizardStaffWizardCharacter* TargetWizard, int32 WinnerMashCount, int32 LoserMashCount);
	void AddQuickBonkStressForHitCount(int32 HitCount, FName StressSource);
	void PlayBonkHitFeedback(AWizardStaffWizardCharacter* TargetWizard, const FVector& KnockbackDirection) const;
	void PlayBonkStressFeedback(float StressAdded, bool bSnapped, int32 HitCount) const;
	bool FireArcanePinball();
	bool SpawnArcanePinballReadabilityShell();
	void UpdateSpellbookVisual();
	UFUNCTION()
	void HandleStaffSegmentSnapped(int32 NewSegmentCount, float RemainingStress);
	void DrawSloshDebug() const;
	float GetManaSloshMovementMultiplier() const;
	float GetManaSloshTurnMultiplier() const;
	float GetManaSloshAccelerationMultiplier() const;
	float GetManaSloshBrakingMultiplier() const;
	float GetHitReactionInputMultiplier() const;
	int32 GetHeavyStaffSegmentCount() const;
	int32 GetDebugPlayerIndex() const;
	float GetPartyHallBonkMultiplier() const;

private:
	void SetDebugSloshAlpha(float SloshAlpha);
	void RebuildPlayerGroundMarkerMesh();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RobeMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HatMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PlayerMarkerMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LeaderMarkerMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SpellbookMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SpellbookGlowMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BroomHandleMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BroomBristleMaterialInstance;

	float SloshTurnCarryDegreesPerSecond = 0.0f;
	float StumbleCooldownRemaining = 0.0f;
	FRotator SloshStaffVisualRotation = FRotator::ZeroRotator;
	FRotator HitReactionVisualRotation = FRotator::ZeroRotator;
	FRotator HitReactionStaffVisualRotation = FRotator::ZeroRotator;
	FVector LastHitReactionDirection = FVector::ForwardVector;
	float LastHitReactionSeverity = 0.0f;
	FLinearColor CurrentPlayerMarkerColor = FLinearColor::White;
	FLinearColor CurrentRobeColor = FLinearColor::White;
	FLinearColor CurrentHatColor = FLinearColor::Black;
	float LastQuickBonkTime = -1000.0f;
	float QuickBonkVisualTimeRemaining = 0.0f;
	float QuickBonkImpactTimeRemaining = 0.0f;
	int32 QuickBonkHitCountThisSwing = 0;
	bool bQuickBonkHitResolved = true;
	TSet<AWizardStaffWizardCharacter*> QuickBonkHitWizardsThisSwing;
	TWeakObjectPtr<AWizardStaffWizardCharacter> StaffClashOpponent;
	FVector StaffClashLockedLocation = FVector::ZeroVector;
	float StaffClashTimeRemaining = 0.0f;
	int32 StaffClashMashCount = 0;
	bool bStaffClashActive = false;
	bool bStaffClashResolver = false;
	bool bPrototypeInputLocked = false;
	FRotator StaffRootDefaultRelativeRotation = FRotator::ZeroRotator;
	float BroomBoostTimeRemaining = 0.0f;
	float BroomBoostLandingCooldownRemaining = 0.0f;
	bool bBroomBoostAvailable = true;
	bool bBroomBoostActive = false;
	bool bBroomBoostNeedsLandingCooldown = false;
	FVector BroomBoostDirection = FVector::ForwardVector;
	bool bMegaStaffBrewActive = false;
	int32 MegaStaffTemporarySegmentsRemaining = 0;
	float MegaStaffTimeRemaining = 0.0f;
	float MegaStaffStressMultiplierDuringEffect = 1.0f;
	float MegaStaffKnockbackMultiplierDuringEffect = 1.0f;
	bool bMegaStaffRemoveTemporarySegmentsOnExpire = true;
	bool bMegaStaffShowDebug = true;
	bool bMegaStaffExpireWarningShown = false;
};
