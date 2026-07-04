#include "WizardStaffPlaytestBotComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffHUD.h"
#include "WizardStaffManaMugPickup.h"
#include "WizardStaffStaffsAtDawnPowerupPickup.h"
#include "WizardStaffWizardCharacter.h"

UWizardStaffPlaytestBotComponent::UWizardStaffPlaytestBotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UWizardStaffPlaytestBotComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetBotState();
	SetComponentTickEnabled(bBotEnabled);
}

void UWizardStaffPlaytestBotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AWizardStaffWizardCharacter* Wizard = GetWizardOwner();
	AWizardStaffGameMode* GameMode = GetWizardGameMode();
	if (!bBotEnabled || !Wizard || !GameMode || !GameMode->bEnablePlaytestBots)
	{
		if (Wizard)
		{
			Wizard->ApplyPrototypeLocalInput(0.0f, 0.0f, 0.0f);
		}
		return;
	}

	UpdateStateSignature();

	ThinkTimeRemaining -= DeltaTime;
	TargetRefreshRemaining -= DeltaTime;
	ReverseTimeRemaining = FMath::Max(0.0f, ReverseTimeRemaining - DeltaTime);

	if (ThinkTimeRemaining <= 0.0f)
	{
		const float ThinkInterval = FMath::Max(GameMode->PlaytestBotTuning.BotThinkInterval, 0.02f);
		ThinkTimeRemaining = ThinkInterval;
		Think(ThinkInterval);
	}

	MaybeUseBroomRecovery(GameMode, Wizard);
	ApplyMovement(DeltaTime);
	UpdateStuckHandling(DeltaTime);
	DrawDebug();
}

void UWizardStaffPlaytestBotComponent::SetBotEnabled(bool bNewEnabled)
{
	if (bBotEnabled == bNewEnabled)
	{
		return;
	}

	bBotEnabled = bNewEnabled;
	ResetBotState();
	SetComponentTickEnabled(bBotEnabled);

	if (AWizardStaffWizardCharacter* Wizard = GetWizardOwner())
	{
		Wizard->ApplyPrototypeLocalInput(0.0f, 0.0f, 0.0f);
	}
}

AWizardStaffWizardCharacter* UWizardStaffPlaytestBotComponent::GetWizardOwner() const
{
	return Cast<AWizardStaffWizardCharacter>(GetOwner());
}

AWizardStaffGameMode* UWizardStaffPlaytestBotComponent::GetWizardGameMode() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
}

void UWizardStaffPlaytestBotComponent::ResetBotState()
{
	bHasTarget = false;
	CurrentTargetLocation = FVector::ZeroVector;
	TargetReason = TEXT("None");
	BotDebugState = bBotEnabled ? TEXT("Idle") : TEXT("Disabled");
	ThinkTimeRemaining = 0.0f;
	TargetRefreshRemaining = 0.0f;
	ReverseTimeRemaining = 0.0f;
	ReverseTurnDirection = FMath::RandBool() ? 1.0f : -1.0f;
	StuckTime = 0.0f;
	LastBroomAttemptTime = -1000.0f;
	LastProgressLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	LastStateSignature.Reset();
	if (AWizardStaffGameMode* GameMode = GetWizardGameMode())
	{
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		const float MinDelay = FMath::Max(GameMode->PlaytestBotTuning.BotReadyBellDelayMin, 0.0f);
		const float MaxDelay = FMath::Max(GameMode->PlaytestBotTuning.BotReadyBellDelayMax, MinDelay);
		ReadyBellAllowedTime = Now + FMath::FRandRange(MinDelay, MaxDelay);
	}
}

void UWizardStaffPlaytestBotComponent::UpdateStateSignature()
{
	const AWizardStaffGameMode* GameMode = GetWizardGameMode();
	if (!GameMode)
	{
		return;
	}

	const FString StateSignature = FString::Printf(
		TEXT("%d:%d:%d"),
		static_cast<int32>(GameMode->GetPartyMatchState()),
		static_cast<int32>(GameMode->GetActiveTrialType()),
		static_cast<int32>(GameMode->GetActiveTrialState()));
	if (StateSignature == LastStateSignature)
	{
		return;
	}

	LastStateSignature = StateSignature;
	TargetRefreshRemaining = 0.0f;
	ReverseTimeRemaining = 0.0f;
	StuckTime = 0.0f;
	LastProgressLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float MinDelay = FMath::Max(GameMode->PlaytestBotTuning.BotReadyBellDelayMin, 0.0f);
	const float MaxDelay = FMath::Max(GameMode->PlaytestBotTuning.BotReadyBellDelayMax, MinDelay);
	ReadyBellAllowedTime = Now + FMath::FRandRange(MinDelay, MaxDelay);
}

void UWizardStaffPlaytestBotComponent::Think(float DeltaTime)
{
	AWizardStaffWizardCharacter* Wizard = GetWizardOwner();
	AWizardStaffGameMode* GameMode = GetWizardGameMode();
	if (!Wizard || !GameMode)
	{
		return;
	}

	if (TargetRefreshRemaining <= 0.0f || !bHasTarget || GetDistanceToTarget2D(Wizard) <= 115.0f)
	{
		FVector NewTarget = FVector::ZeroVector;
		FString NewReason;
		bHasTarget = ChooseTarget(NewTarget, NewReason);
		if (bHasTarget)
		{
			CurrentTargetLocation = NewTarget;
			TargetReason = NewReason;
		}
		TargetRefreshRemaining = FMath::Max(GameMode->PlaytestBotTuning.BotTargetRefreshTime, 0.1f);
	}

	MaybeUseActions(GameMode, Wizard, DeltaTime);
}

void UWizardStaffPlaytestBotComponent::ApplyMovement(float DeltaTime)
{
	(void)DeltaTime;

	AWizardStaffWizardCharacter* Wizard = GetWizardOwner();
	AWizardStaffGameMode* GameMode = GetWizardGameMode();
	if (!Wizard || !GameMode)
	{
		return;
	}

	float ForwardValue = 0.0f;
	float RightValue = 0.0f;
	float TurnValue = 0.0f;

	if (ReverseTimeRemaining > 0.0f)
	{
		ForwardValue = -0.7f;
		RightValue = 0.25f * ReverseTurnDirection;
		TurnValue = ReverseTurnDirection;
		BotDebugState = TEXT("Unsticking");
	}
	else if (bHasTarget)
	{
		FVector ToTarget = CurrentTargetLocation - Wizard->GetActorLocation();
		ToTarget.Z = 0.0f;
		const float Distance = ToTarget.Size();
		if (Distance > 1.0f)
		{
			const FVector Direction = ToTarget / Distance;
			const FVector LocalDirection = Wizard->GetActorTransform().InverseTransformVectorNoScale(Direction);
			const float MoveAggression = FMath::Max(GameMode->PlaytestBotTuning.BotMoveAggression, 0.0f);
			const float TurnAggression = FMath::Max(GameMode->PlaytestBotTuning.BotTurnAggression, 0.0f);
			const float Slowdown = Distance < 160.0f ? FMath::Clamp(Distance / 160.0f, 0.25f, 1.0f) : 1.0f;

			ForwardValue = FMath::Clamp(LocalDirection.X * MoveAggression * Slowdown, -1.0f, 1.0f);
			RightValue = FMath::Clamp(LocalDirection.Y * MoveAggression * 0.65f * Slowdown, -1.0f, 1.0f);
			const float DesiredYawDegrees = FMath::RadiansToDegrees(FMath::Atan2(LocalDirection.Y, FMath::Max(LocalDirection.X, 0.05f)));
			TurnValue = FMath::Clamp(DesiredYawDegrees / 70.0f * TurnAggression, -1.0f, 1.0f);
			BotDebugState = TargetReason;
		}
	}

	Wizard->ApplyPrototypeLocalInput(ForwardValue, RightValue, TurnValue);
}

void UWizardStaffPlaytestBotComponent::UpdateStuckHandling(float DeltaTime)
{
	AWizardStaffWizardCharacter* Wizard = GetWizardOwner();
	AWizardStaffGameMode* GameMode = GetWizardGameMode();
	if (!Wizard || !GameMode)
	{
		return;
	}

	const FVector CurrentLocation = Wizard->GetActorLocation();
	const float ProgressDistance = FVector::Dist2D(CurrentLocation, LastProgressLocation);
	if (ReverseTimeRemaining > 0.0f || !bHasTarget)
	{
		StuckTime = 0.0f;
		LastProgressLocation = CurrentLocation;
		return;
	}

	if (ProgressDistance > 85.0f)
	{
		StuckTime = 0.0f;
		LastProgressLocation = CurrentLocation;
		return;
	}

	StuckTime += DeltaTime;
	if (StuckTime >= FMath::Max(GameMode->PlaytestBotTuning.BotStuckTimeBeforeRetarget, 0.1f))
	{
		ReverseTimeRemaining = 0.65f;
		ReverseTurnDirection = FMath::RandBool() ? 1.0f : -1.0f;
		TargetRefreshRemaining = 0.0f;
		bHasTarget = false;
		StuckTime = 0.0f;
		LastProgressLocation = CurrentLocation;
	}
}

void UWizardStaffPlaytestBotComponent::DrawDebug() const
{
	const AWizardStaffGameMode* GameMode = GetWizardGameMode();
	const AWizardStaffWizardCharacter* Wizard = GetWizardOwner();
	UWorld* World = GetWorld();
	if (!GameMode || !Wizard || !World || !GameMode->bShowPlaytestBotDebug || !AWizardStaffHUD::IsFullDebugMode(this))
	{
		return;
	}

	const FVector Start = Wizard->GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
	if (bHasTarget)
	{
		DrawDebugLine(World, Start, CurrentTargetLocation + FVector(0.0f, 0.0f, 40.0f), FColor::Cyan, false, 0.12f, 0, 2.0f);
		DrawDebugSphere(World, CurrentTargetLocation, 32.0f, 10, FColor::Cyan, false, 0.12f, 0, 1.5f);
	}
	DrawDebugString(World, Start + FVector(0.0f, 0.0f, 30.0f), FString::Printf(TEXT("BOT: %s"), *BotDebugState), nullptr, FColor::Cyan, 0.12f, false);
}

bool UWizardStaffPlaytestBotComponent::ChooseTarget(FVector& OutTargetLocation, FString& OutReason)
{
	AWizardStaffWizardCharacter* Wizard = GetWizardOwner();
	AWizardStaffGameMode* GameMode = GetWizardGameMode();
	if (!Wizard || !GameMode)
	{
		return false;
	}

	if (GameMode->IsPartyHallIntermissionActive())
	{
		return ChoosePartyHallTarget(GameMode, Wizard, OutTargetLocation, OutReason);
	}

	if (GameMode->GetPartyMatchState() == EWizardPartyMatchState::FinalRound)
	{
		return ChooseFinalRoundTarget(GameMode, Wizard, OutTargetLocation, OutReason);
	}

	if (GameMode->GetActiveTrialType() == EWizardTrialType::StaffsAtDawn && GameMode->IsStaffsAtDawnActive())
	{
		return ChooseStaffsAtDawnTarget(GameMode, Wizard, OutTargetLocation, OutReason);
	}

	if (GameMode->GetActiveTrialType() == EWizardTrialType::MugRun && GameMode->GetMugRunMatchState() == EWizardMugRunMatchState::Playing)
	{
		return ChooseMugRunTarget(GameMode, Wizard, OutTargetLocation, OutReason);
	}

	return ChooseWanderTarget(GameMode, OutTargetLocation, OutReason, TEXT("Waiting"));
}

bool UWizardStaffPlaytestBotComponent::ChoosePartyHallTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason)
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const int32 PlayerIndex = GetOwnerPlayerIndex();
	FVector ReadyBellLocation = FVector::ZeroVector;
	if (GameMode->PartyHallTuning.bEnableReadyBell
		&& PlayerIndex != INDEX_NONE
		&& !GameMode->IsPartyHallPlayerReady(PlayerIndex)
		&& Now >= ReadyBellAllowedTime
		&& GameMode->GetPartyHallReadyBellLocation(ReadyBellLocation))
	{
		OutTargetLocation = ReadyBellLocation;
		OutReason = TEXT("Ready Bell");
		return true;
	}

	if (AWizardStaffWizardCharacter* Opponent = FindNearestOpponent(Wizard, 520.0f))
	{
		OutTargetLocation = Opponent->GetActorLocation();
		OutReason = TEXT("Party Bonk");
		return true;
	}

	return ChooseWanderTarget(GameMode, OutTargetLocation, OutReason, TEXT("Party Wander"));
}

bool UWizardStaffPlaytestBotComponent::ChooseMugRunTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason)
{
	if (AWizardStaffManaMugPickup* Mug = FindNearestActiveMug(Wizard))
	{
		OutTargetLocation = Mug->GetActorLocation();
		OutReason = TEXT("Mug");
		return true;
	}

	if (AWizardStaffWizardCharacter* Opponent = FindNearestOpponent(Wizard, Wizard->GetQuickBonkRange() + 140.0f))
	{
		OutTargetLocation = Opponent->GetActorLocation();
		OutReason = TEXT("Mug Run Bonk");
		return true;
	}

	return ChooseWanderTarget(GameMode, OutTargetLocation, OutReason, TEXT("Mug Wander"));
}

bool UWizardStaffPlaytestBotComponent::ChooseStaffsAtDawnTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason)
{
	const FVector Location = Wizard->GetActorLocation();
	if (IsNearArenaEdge(GameMode, Location, 0.78f))
	{
		OutTargetLocation = GameMode->GetPlaytestBoundsCenter();
		OutReason = TEXT("Recover Center");
		return true;
	}

	if (AWizardStaffStaffsAtDawnPowerupPickup* Powerup = FindNearestActiveStaffsAtDawnPowerup(Wizard, 820.0f))
	{
		OutTargetLocation = Powerup->GetActorLocation();
		OutReason = TEXT("Mega Staff");
		return true;
	}

	if (AWizardStaffWizardCharacter* Opponent = FindNearestOpponent(Wizard))
	{
		OutTargetLocation = Opponent->GetActorLocation();
		OutReason = TEXT("Dawn Duel");
		return true;
	}

	return ChooseWanderTarget(GameMode, OutTargetLocation, OutReason, TEXT("Dawn Wander"));
}

bool UWizardStaffPlaytestBotComponent::ChooseFinalRoundTarget(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, FVector& OutTargetLocation, FString& OutReason)
{
	const int32 PlayerIndex = GetOwnerPlayerIndex();
	const FVector CircleCenter = GameMode->GetPlaytestFinalCircleCenter();
	if (PlayerIndex == GameMode->GetGrandWizardCandidatePlayerIndex())
	{
		OutTargetLocation = CircleCenter;
		OutReason = TEXT("Hold Circle");
		return true;
	}

	if (GameMode->IsGrandWizardCandidateVulnerable() || GameMode->GetGrandWizardStealPlayerIndex() == PlayerIndex)
	{
		OutTargetLocation = CircleCenter;
		OutReason = TEXT("Steal Circle");
		return true;
	}

	AWizardStaffWizardCharacter* Candidate = nullptr;
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AWizardStaffWizardCharacter> It(World); It; ++It)
		{
			AWizardStaffWizardCharacter* CandidateWizard = *It;
			const int32 CandidateIndex = GameMode->GetPlayerIndexForWizard(CandidateWizard);
			if (CandidateIndex == GameMode->GetGrandWizardCandidatePlayerIndex())
			{
				Candidate = CandidateWizard;
				break;
			}
		}
	}
	if (Candidate)
	{
		OutTargetLocation = Candidate->GetActorLocation();
		OutReason = TEXT("Bonk Candidate");
		return true;
	}

	OutTargetLocation = CircleCenter;
	OutReason = TEXT("Contest Circle");
	return true;
}

bool UWizardStaffPlaytestBotComponent::ChooseWanderTarget(AWizardStaffGameMode* GameMode, FVector& OutTargetLocation, FString& OutReason, const FString& Reason)
{
	if (!GameMode)
	{
		return false;
	}

	const FVector Center = GameMode->GetPlaytestBoundsCenter();
	const float HalfSize = FMath::Max(GameMode->GetPlaytestBoundsHalfSize() * 0.58f, 120.0f);
	OutTargetLocation = Center + FVector(FMath::FRandRange(-HalfSize, HalfSize), FMath::FRandRange(-HalfSize, HalfSize), 0.0f);
	OutReason = Reason;
	return true;
}

AWizardStaffWizardCharacter* UWizardStaffPlaytestBotComponent::FindNearestOpponent(const AWizardStaffWizardCharacter* Wizard, float MaxDistance) const
{
	if (!Wizard)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AWizardStaffWizardCharacter* BestOpponent = nullptr;
	float BestDistanceSquared = MaxDistance > 0.0f ? FMath::Square(MaxDistance) : TNumericLimits<float>::Max();
	for (TActorIterator<AWizardStaffWizardCharacter> It(World); It; ++It)
	{
		AWizardStaffWizardCharacter* Candidate = *It;
		if (!IsValid(Candidate) || Candidate == Wizard)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(Wizard->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestOpponent = Candidate;
		}
	}

	return BestOpponent;
}

AWizardStaffManaMugPickup* UWizardStaffPlaytestBotComponent::FindNearestActiveMug(const AWizardStaffWizardCharacter* Wizard) const
{
	if (!Wizard)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AWizardStaffManaMugPickup* BestMug = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	for (TActorIterator<AWizardStaffManaMugPickup> It(World); It; ++It)
	{
		AWizardStaffManaMugPickup* Mug = *It;
		if (!IsValid(Mug) || !Mug->IsPickupActive())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(Wizard->GetActorLocation(), Mug->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestMug = Mug;
		}
	}

	return BestMug;
}

AWizardStaffStaffsAtDawnPowerupPickup* UWizardStaffPlaytestBotComponent::FindNearestActiveStaffsAtDawnPowerup(const AWizardStaffWizardCharacter* Wizard, float MaxDistance) const
{
	if (!Wizard)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AWizardStaffStaffsAtDawnPowerupPickup* BestPowerup = nullptr;
	float BestDistanceSquared = MaxDistance > 0.0f ? FMath::Square(MaxDistance) : TNumericLimits<float>::Max();
	for (TActorIterator<AWizardStaffStaffsAtDawnPowerupPickup> It(World); It; ++It)
	{
		AWizardStaffStaffsAtDawnPowerupPickup* Powerup = *It;
		if (!IsValid(Powerup) || !Powerup->IsPickupActive())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(Wizard->GetActorLocation(), Powerup->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestPowerup = Powerup;
		}
	}

	return BestPowerup;
}

void UWizardStaffPlaytestBotComponent::MaybeUseActions(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard, float DeltaTime)
{
	if (!GameMode || !Wizard)
	{
		return;
	}

	const float ThinkChanceScale = FMath::Max(DeltaTime / FMath::Max(GameMode->PlaytestBotTuning.BotThinkInterval, 0.02f), 0.25f);
	if (Wizard->IsInStaffClash())
	{
		if (FMath::FRand() <= FMath::Clamp(GameMode->PlaytestBotTuning.BotStaffClashMashChance, 0.0f, 1.0f))
		{
			Wizard->QuickBonk();
		}
		return;
	}

	const bool bPartyBellTarget = TargetReason == TEXT("Ready Bell") && GetDistanceToTarget2D(Wizard) <= Wizard->GetQuickBonkRange() + GameMode->PlaytestBotTuning.BotBonkRangePadding;
	if (bPartyBellTarget)
	{
		Wizard->QuickBonk();
		return;
	}

	AWizardStaffWizardCharacter* NearbyOpponent = FindNearestOpponent(Wizard, Wizard->GetQuickBonkRange() + GameMode->PlaytestBotTuning.BotBonkRangePadding);
	if (NearbyOpponent)
	{
		FVector ToOpponent = NearbyOpponent->GetActorLocation() - Wizard->GetActorLocation();
		ToOpponent.Z = 0.0f;
		const float FacingDot = FVector::DotProduct(Wizard->GetActorForwardVector().GetSafeNormal2D(), ToOpponent.GetSafeNormal());
		if (FacingDot > 0.20f && FMath::FRand() <= GameMode->PlaytestBotTuning.BotBonkChance * ThinkChanceScale)
		{
			Wizard->QuickBonk();
		}
	}

	if (Wizard->HasCarriedBrewReward() && FMath::FRand() <= GameMode->PlaytestBotTuning.BotRewardUseChance * ThinkChanceScale)
	{
		if (!NearbyOpponent || FVector::DistSquared2D(Wizard->GetActorLocation(), NearbyOpponent->GetActorLocation()) <= FMath::Square(1450.0f))
		{
			Wizard->UseReward();
		}
	}

	if (UCharacterMovementComponent* MovementComponent = Wizard->GetCharacterMovement())
	{
		if (!MovementComponent->IsFalling() && FMath::FRand() <= GameMode->PlaytestBotTuning.BotJumpChance * ThinkChanceScale)
		{
			Wizard->BotPressJump();
		}
	}
}

void UWizardStaffPlaytestBotComponent::MaybeUseBroomRecovery(AWizardStaffGameMode* GameMode, AWizardStaffWizardCharacter* Wizard)
{
	if (!GameMode || !Wizard)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Wizard->GetCharacterMovement();
	if (!MovementComponent || !MovementComponent->IsFalling() || !Wizard->CanBotBroomBoost())
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (Now < LastBroomAttemptTime + 0.35f)
	{
		return;
	}

	const bool bNearEdge = IsNearArenaEdge(GameMode, Wizard->GetActorLocation(), 0.70f);
	const bool bFallingFast = MovementComponent->Velocity.Z < -120.0f;
	if ((bNearEdge || bFallingFast) && FMath::FRand() <= GameMode->PlaytestBotTuning.BotBroomBoostRecoveryChance)
	{
		LastBroomAttemptTime = Now;
		Wizard->BotPressJump();
	}
}

int32 UWizardStaffPlaytestBotComponent::GetOwnerPlayerIndex() const
{
	const AWizardStaffWizardCharacter* Wizard = GetWizardOwner();
	const AWizardStaffGameMode* GameMode = GetWizardGameMode();
	return GameMode && Wizard ? GameMode->GetPlayerIndexForWizard(Wizard) : INDEX_NONE;
}

float UWizardStaffPlaytestBotComponent::GetDistanceToTarget2D(const AWizardStaffWizardCharacter* Wizard) const
{
	return Wizard && bHasTarget ? FVector::Dist2D(Wizard->GetActorLocation(), CurrentTargetLocation) : TNumericLimits<float>::Max();
}

bool UWizardStaffPlaytestBotComponent::IsNearArenaEdge(const AWizardStaffGameMode* GameMode, const FVector& Location, float EdgeAlpha) const
{
	if (!GameMode)
	{
		return false;
	}

	const FVector Center = GameMode->GetPlaytestBoundsCenter();
	const float HalfSize = FMath::Max(GameMode->GetPlaytestBoundsHalfSize(), 1.0f);
	const FVector Local(Location.X - Center.X, Location.Y - Center.Y, 0.0f);
	return FMath::Max(FMath::Abs(Local.X), FMath::Abs(Local.Y)) >= HalfSize * FMath::Clamp(EdgeAlpha, 0.0f, 1.0f);
}
