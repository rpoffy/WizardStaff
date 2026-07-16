#include "WizardStaffHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "WizardStaffComponent.h"
#include "WizardStaffGameState.h"
#include "WizardStaffGameMode.h"
#include "WizardStaffPlayerState.h"
#include "WizardStaffWizardCharacter.h"

namespace
{
AWizardStaffHUD* GetSharedWizardStaffHudFromWorld(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	APlayerController* FirstPlayerController = World->GetFirstPlayerController();
	return FirstPlayerController ? Cast<AWizardStaffHUD>(FirstPlayerController->GetHUD()) : nullptr;
}

int32 GetHudPlayerIndex(const AWizardStaffGameMode* GameMode, const AWizardStaffWizardCharacter* Wizard)
{
	if (const AWizardStaffPlayerState* WizardPlayerState = Wizard ? Wizard->GetPlayerState<AWizardStaffPlayerState>() : nullptr)
	{
		const int32 DisplaySlot = WizardPlayerState->GetWizardDisplaySlot();
		if (DisplaySlot != INDEX_NONE)
		{
			return DisplaySlot;
		}
	}

	if (GameMode)
	{
		const int32 PlayerIndex = GameMode->GetPlayerIndexForWizard(Wizard);
		if (PlayerIndex != INDEX_NONE)
		{
			return PlayerIndex;
		}
	}

	const APlayerState* PlayerState = Wizard ? Wizard->GetPlayerState() : nullptr;
	return PlayerState ? FMath::Max(PlayerState->GetPlayerId(), 0) : 0;
}

const AWizardStaffGameState* GetHudGameState(const UWorld* World)
{
	return World ? World->GetGameState<AWizardStaffGameState>() : nullptr;
}

const AWizardStaffPlayerState* GetHudPlayerState(const AWizardStaffWizardCharacter* Wizard)
{
	return Wizard ? Wizard->GetPlayerState<AWizardStaffPlayerState>() : nullptr;
}

int32 GetHudRoundWins(const AWizardStaffGameMode* GameMode, const AWizardStaffWizardCharacter* Wizard, int32 PlayerIndex)
{
	if (const AWizardStaffPlayerState* WizardPlayerState = GetHudPlayerState(Wizard))
	{
		if (WizardPlayerState->GetWizardDisplaySlot() != INDEX_NONE)
		{
			return WizardPlayerState->GetRoundWins();
		}
	}

	return GameMode ? GameMode->GetPlayerRoundWins(PlayerIndex) : 0;
}

int32 GetHudGrandWizardFavor(const AWizardStaffGameMode* GameMode, const AWizardStaffWizardCharacter* Wizard, int32 PlayerIndex)
{
	if (const AWizardStaffPlayerState* WizardPlayerState = GetHudPlayerState(Wizard))
	{
		if (WizardPlayerState->GetWizardDisplaySlot() != INDEX_NONE)
		{
			return WizardPlayerState->GetGrandWizardFavor();
		}
	}

	return GameMode ? GameMode->GetPlayerGrandWizardFavor(PlayerIndex) : 0;
}

int32 GetHudStaffsAtDawnScore(const AWizardStaffGameMode* GameMode, const AWizardStaffWizardCharacter* Wizard, int32 PlayerIndex)
{
	if (const AWizardStaffPlayerState* WizardPlayerState = GetHudPlayerState(Wizard))
	{
		if (WizardPlayerState->GetWizardDisplaySlot() != INDEX_NONE)
		{
			return WizardPlayerState->GetStaffsAtDawnScore();
		}
	}

	return GameMode ? GameMode->GetStaffsAtDawnScore(PlayerIndex) : 0;
}

int32 GetHudCurrentTrialScore(const AWizardStaffGameMode* GameMode, const AWizardStaffWizardCharacter* Wizard, int32 PlayerIndex)
{
	if (const AWizardStaffPlayerState* WizardPlayerState = GetHudPlayerState(Wizard))
	{
		if (WizardPlayerState->GetWizardDisplaySlot() != INDEX_NONE)
		{
			return WizardPlayerState->GetCurrentTrialScore();
		}
	}
	return GameMode ? GameMode->GetCauldronScore(PlayerIndex) : 0;
}

bool IsHudPartyHallReady(const AWizardStaffGameMode* GameMode, const AWizardStaffWizardCharacter* Wizard, int32 PlayerIndex)
{
	if (const AWizardStaffPlayerState* WizardPlayerState = GetHudPlayerState(Wizard))
	{
		if (WizardPlayerState->GetWizardDisplaySlot() != INDEX_NONE)
		{
			return WizardPlayerState->IsPartyHallReady();
		}
	}

	return GameMode && GameMode->IsPartyHallPlayerReady(PlayerIndex);
}

bool IsHudPlaytestBot(const AWizardStaffWizardCharacter* Wizard)
{
	if (const AWizardStaffPlayerState* WizardPlayerState = GetHudPlayerState(Wizard))
	{
		if (WizardPlayerState->GetWizardDisplaySlot() != INDEX_NONE)
		{
			return WizardPlayerState->IsPlaytestBotSlot();
		}
	}

	return Wizard && Wizard->IsPlaytestBot();
}

FColor GetReplicatedGameplayEventHudColor(EWizardReplicatedGameplayEventType EventType)
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
}

void AWizardStaffHUD::BeginPlay()
{
	Super::BeginPlay();
	HudDisplayMode = DefaultHudMode;
}

void AWizardStaffHUD::DrawHUD()
{
	Super::DrawHUD();

	PruneExpiredHudMessages();

	if (!Canvas || !ShouldDrawSharedHUD() || HudDisplayMode == EWizardHudDisplayMode::Hidden)
	{
		return;
	}

	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	switch (HudDisplayMode)
	{
	case EWizardHudDisplayMode::FullDebug:
		DrawFullDebugHUD(GameMode);
		if (GameMode || GameplayMessageFeed.Num() > 0)
		{
			DrawHudMessageFeed(FullDebugMessageCount);
		}
		else
		{
			DrawReplicatedGameplayEventFeed(FullDebugMessageCount);
		}
		break;
	case EWizardHudDisplayMode::Minimal:
		DrawMinimalHUD(GameMode);
		if (!GameMode)
		{
			DrawReplicatedGameplayEventFeed(2);
		}
		break;
	case EWizardHudDisplayMode::Playtest:
	default:
		DrawPlaytestHUD(GameMode);
		if (GameMode || GameplayMessageFeed.Num() > 0)
		{
			DrawHudMessageFeed(PlaytestMessageCount);
		}
		else
		{
			DrawReplicatedGameplayEventFeed(PlaytestMessageCount);
		}
		break;
	}
}

void AWizardStaffHUD::CycleWizardHudMode()
{
	switch (HudDisplayMode)
	{
	case EWizardHudDisplayMode::Playtest:
		SetWizardHudMode(EWizardHudDisplayMode::FullDebug);
		break;
	case EWizardHudDisplayMode::FullDebug:
		SetWizardHudMode(EWizardHudDisplayMode::Minimal);
		break;
	case EWizardHudDisplayMode::Minimal:
		SetWizardHudMode(EWizardHudDisplayMode::Hidden);
		break;
	case EWizardHudDisplayMode::Hidden:
	default:
		SetWizardHudMode(EWizardHudDisplayMode::Playtest);
		break;
	}
}

void AWizardStaffHUD::SetWizardHudMode(EWizardHudDisplayMode NewMode)
{
	HudDisplayMode = NewMode;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1900701,
			1.8f,
			FColor::Cyan,
			FString::Printf(TEXT("HUD Mode: %s"), *GetWizardHudModeName()));
	}
}

FString AWizardStaffHUD::GetWizardHudModeName() const
{
	switch (HudDisplayMode)
	{
	case EWizardHudDisplayMode::FullDebug:
		return TEXT("Full Debug");
	case EWizardHudDisplayMode::Minimal:
		return TEXT("Minimal");
	case EWizardHudDisplayMode::Hidden:
		return TEXT("Hidden");
	case EWizardHudDisplayMode::Playtest:
	default:
		return TEXT("Playtest");
	}
}

void AWizardStaffHUD::AddHudMessage(const FString& Message, FLinearColor MessageColor, float Lifetime, EWizardHudMessageCategory Category)
{
	if (Message.IsEmpty())
	{
		return;
	}

	PruneExpiredHudMessages();

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float SafeLifetime = FMath::Max(Lifetime > 0.0f ? Lifetime : GameplayMessageDefaultLifetime, 0.1f);
	for (int32 MessageIndex = GameplayMessageFeed.Num() - 1; MessageIndex >= 0; --MessageIndex)
	{
		FWizardHudFeedMessage& ExistingMessage = GameplayMessageFeed[MessageIndex];
		if (ExistingMessage.Text == Message && ExistingMessage.Category == Category)
		{
			ExistingMessage.Color = MessageColor;
			ExistingMessage.ExpireTime = Now + SafeLifetime;
			++ExistingMessage.RepeatCount;
			if (MessageIndex != GameplayMessageFeed.Num() - 1)
			{
				const FWizardHudFeedMessage UpdatedMessage = ExistingMessage;
				GameplayMessageFeed.RemoveAt(MessageIndex);
				GameplayMessageFeed.Add(UpdatedMessage);
			}
			return;
		}
	}

	FWizardHudFeedMessage NewMessage;
	NewMessage.Text = Message.Left(120);
	NewMessage.Color = MessageColor;
	NewMessage.ExpireTime = Now + SafeLifetime;
	NewMessage.Category = Category;
	NewMessage.RepeatCount = 1;
	GameplayMessageFeed.Add(NewMessage);

	const int32 SafeMaxStoredMessages = FMath::Max(MaxStoredMessages, 1);
	while (GameplayMessageFeed.Num() > SafeMaxStoredMessages)
	{
		GameplayMessageFeed.RemoveAt(0);
	}
}

void AWizardStaffHUD::PushGameplayMessage(const UObject* WorldContextObject, const FString& Message, const FColor& MessageColor, float Lifetime, EWizardHudMessageCategory Category)
{
	if (!WorldContextObject || Message.IsEmpty())
	{
		return;
	}

	if (AWizardStaffHUD* WizardHud = GetSharedWizardStaffHudFromWorld(WorldContextObject->GetWorld()))
	{
		WizardHud->AddHudMessage(Message, FLinearColor(MessageColor), Lifetime, Category);
	}
}

bool AWizardStaffHUD::IsFullDebugMode(const UObject* WorldContextObject)
{
	const AWizardStaffHUD* WizardHud = WorldContextObject ? GetSharedWizardStaffHudFromWorld(WorldContextObject->GetWorld()) : nullptr;
	return WizardHud && WizardHud->GetWizardHudMode() == EWizardHudDisplayMode::FullDebug;
}

void AWizardStaffHUD::DrawFullDebugHUD(const AWizardStaffGameMode* GameMode)
{
	const bool bUseCompactFinalHud = GameMode
		&& GameMode->GetPartyMatchState() == EWizardPartyMatchState::FinalRound
		&& GameMode->GetGrandWizardWinnerMessage().IsEmpty();

	if (bUseCompactFinalHud)
	{
		DrawCompactFinalHud(GameMode);
		return;
	}

	DrawMatchHeader(PanelX, PanelY);
	DrawPlayerRows(PanelX, PanelY + 42.0f);
	DrawScoringClarityPanel();
	DrawWinnerMessage();
	DrawMatchSummary();
}

void AWizardStaffHUD::DrawPlaytestHUD(const AWizardStaffGameMode* GameMode)
{
	const AWizardStaffGameState* WizardGameState = GetHudGameState(GetWorld());
	const TArray<const AWizardStaffWizardCharacter*> Wizards = GetVisibleWizards();
	const float CompactWidth = 440.0f;
	const float HeaderHeight = 82.0f;
	const float RowStride = 38.0f;
	const float PlayerRowsY = PanelY + HeaderHeight + 8.0f;
	const float PlayerPanelHeight = static_cast<float>(Wizards.Num()) * RowStride;
	const bool bShowMatchSummary = GameMode && !GameMode->GetGrandWizardWinnerMessage().IsEmpty();

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.58f), PanelX - 8.0f, PanelY - 6.0f, CompactWidth, HeaderHeight);
	if (GameMode)
	{
		DrawText(FString::Printf(TEXT("%s  |  %s"), *GameMode->GetPartyMatchStateText(), *GameMode->GetActiveTrialName()), FColor::White, PanelX, PanelY, nullptr, 0.76f * TextScale);
		DrawText(GetTimerText(GameMode), FColor::Yellow, PanelX, PanelY + 21.0f, nullptr, 0.82f * TextScale);
		DrawText(FString::Printf(TEXT("Preset: %s"), *GameMode->GetActivePrototypeTuningPresetText()), FColor::Cyan, PanelX, PanelY + 44.0f, nullptr, 0.66f * TextScale);
	}
	else if (WizardGameState)
	{
		DrawText(FString::Printf(TEXT("%s  |  %s"), *WizardGameState->GetReplicatedPartyMatchStateText(), *WizardGameState->GetReplicatedActiveTrialName()), FColor::White, PanelX, PanelY, nullptr, 0.76f * TextScale);
		DrawText(GetTimerText(GameMode), FColor::Yellow, PanelX, PanelY + 21.0f, nullptr, 0.82f * TextScale);
		DrawText(FString::Printf(TEXT("Preset: %s"), *WizardGameState->GetReplicatedTuningPresetText()), FColor::Cyan, PanelX, PanelY + 44.0f, nullptr, 0.66f * TextScale);
	}
	else
	{
		DrawText(TEXT("Wizard's Staff"), FColor::White, PanelX, PanelY, nullptr, 0.76f * TextScale);
	}

	DrawCompactPlayerRows(GameMode, PanelX, PlayerRowsY, false);
	DrawPlaytestEventPanel(GameMode, PlayerRowsY + PlayerPanelHeight + 12.0f);

	if (bShowMatchSummary)
	{
		const float SummaryWidth = 500.0f;
		const float X = Canvas ? FMath::Max(PanelX, Canvas->SizeX - SummaryWidth - PanelX) : PanelX + CompactWidth + 24.0f;
		const float Y = Canvas && X < PanelX + CompactWidth + 24.0f ? PlayerRowsY + PlayerPanelHeight + 120.0f : PanelY + 154.0f;
		DrawPlaytestMatchSummary(GameMode, X, Y);
	}
}

void AWizardStaffHUD::DrawMinimalHUD(const AWizardStaffGameMode* GameMode)
{
	DrawMinimalStatePanel(GameMode, PanelX, PanelY);
}

bool AWizardStaffHUD::ShouldDrawSharedHUD() const
{
	const UWorld* World = GetWorld();
	return World && PlayerOwner && PlayerOwner == World->GetFirstPlayerController();
}

void AWizardStaffHUD::DrawMatchHeader(float X, float Y)
{
	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	const float RemainingTime = GameMode && GameMode->GetActiveTrialType() == EWizardTrialType::StaffsAtDawn
		? GameMode->GetStaffsAtDawnRemainingTime()
		: (GameMode && GameMode->GetActiveTrialType() == EWizardTrialType::CauldronCatastrophe
			? GameMode->GetCauldronRemainingTime()
			: (GameMode ? GameMode->GetMugRunRemainingTime() : 0.0f));
	FString HeaderText = GameMode
		? FString::Printf(TEXT("%s  |  Time: %.0fs"), *GameMode->GetActiveTrialName(), RemainingTime)
		: FString::Printf(TEXT("Mug Run  |  Time: %.0fs"), RemainingTime);
	if (GameMode)
	{
		const EWizardTrialState TrialState = GameMode->GetActiveTrialState();
		const bool bInPartyHall = GameMode->GetPartyMatchState() == EWizardPartyMatchState::PartyHall || GameMode->GetPartyMatchState() == EWizardPartyMatchState::Intermission;
		const bool bInFinalRound = GameMode->GetPartyMatchState() == EWizardPartyMatchState::FinalRound;
		const int32 TrialTarget = FMath::Max(GameMode->PartyMatchTuning.TrialsBeforeFinalRound, 1);
		const int32 DisplayTrialNumber = FMath::Clamp(GameMode->GetCompletedTrialCount() + 1, 1, TrialTarget);
		const TCHAR* TimerLabel = TEXT("Time");
		float DisplayTime = RemainingTime;

		if (bInFinalRound)
		{
			HeaderText = GameMode->GetGrandWizardWinnerMessage().IsEmpty()
				? FString::Printf(TEXT("Grand Wizard Final  |  Candidate: %s  |  Time %.0fs"), *GameMode->GetGrandWizardCandidateText(), GameMode->GetFinalRoundRemainingTime())
				: FString::Printf(TEXT("%s  |  Rematch %.0fs"), *GameMode->GetGrandWizardWinnerMessage(), GameMode->GetTrialResultsRemainingTime());
			if (GameMode->GetGrandWizardStealPlayerIndex() != INDEX_NONE)
			{
				HeaderText += FString::Printf(TEXT("  |  P%d stealing %.0f%%"), GameMode->GetGrandWizardStealPlayerIndex() + 1, GameMode->GetGrandWizardStealProgressAlpha() * 100.0f);
			}
		}
		else if (bInPartyHall && TrialState == EWizardTrialState::WaitingToStart)
		{
			TimerLabel = TEXT("Intermission");
			DisplayTime = GameMode->GetIntermissionRemainingTime();
		}
		else if (TrialState == EWizardTrialState::Countdown)
		{
			TimerLabel = TEXT("Countdown");
			DisplayTime = GameMode->GetTrialCountdownRemainingTime();
		}
		else if (TrialState == EWizardTrialState::Results)
		{
			TimerLabel = TEXT("Results");
			DisplayTime = GameMode->GetTrialResultsRemainingTime();
		}
		if (!bInFinalRound)
		{
			HeaderText = FString::Printf(
				TEXT("Party: %s  |  Trial %d/%d: %s  |  %s %.0fs"),
				*GameMode->GetPartyMatchStateText(),
				DisplayTrialNumber,
				TrialTarget,
				*GameMode->GetActiveTrialName(),
				TimerLabel,
				DisplayTime);
		}
	}

	DrawRect(PanelColor, X - 8.0f, Y - 6.0f, RowWidth, 34.0f);
	DrawText(HeaderText, FColor::White, X, Y, nullptr, TextScale);
}

void AWizardStaffHUD::DrawPlayerRows(float X, float Y)
{
	int32 RowIndex = 0;
	for (const AWizardStaffWizardCharacter* Wizard : GetVisibleWizards())
	{
		DrawPlayerRow(Wizard, X, Y + (static_cast<float>(RowIndex) * (RowHeight + 8.0f)));
		++RowIndex;
	}
}

void AWizardStaffHUD::DrawPlayerRow(const AWizardStaffWizardCharacter* Wizard, float X, float Y)
{
	if (!Wizard)
	{
		return;
	}

	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	const AWizardStaffGameState* WizardGameState = GetHudGameState(GetWorld());
	const int32 PlayerIndex = GetHudPlayerIndex(GameMode, Wizard);
	const FLinearColor PlayerColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex);
	const UWizardStaffComponent* StaffComponent = Wizard->StaffComponent;
	const int32 SegmentCount = StaffComponent ? StaffComponent->GetSegmentCount() : 0;
	const int32 RoundWins = GetHudRoundWins(GameMode, Wizard, PlayerIndex);
	const int32 Favor = GetHudGrandWizardFavor(GameMode, Wizard, PlayerIndex);
	const EWizardPartyMatchState PartyState = GameMode ? GameMode->GetPartyMatchState() : (WizardGameState ? WizardGameState->GetReplicatedPartyMatchState() : EWizardPartyMatchState::PartyHall);
	const bool bInFinalRound = PartyState == EWizardPartyMatchState::FinalRound;
	const int32 FinalCandidateIndex = GameMode ? GameMode->GetGrandWizardCandidatePlayerIndex() : (WizardGameState ? WizardGameState->GetReplicatedFinalCandidateIndex() : INDEX_NONE);
	const int32 FinalWinnerIndex = GameMode ? GameMode->GetGrandWizardWinnerPlayerIndex() : (WizardGameState ? WizardGameState->GetReplicatedFinalWinnerIndex() : INDEX_NONE);
	const bool bGrandWizardCandidate = bInFinalRound && FinalCandidateIndex == PlayerIndex;
	const bool bGrandWizardWinner = bInFinalRound && FinalWinnerIndex == PlayerIndex;
	const bool bStandingLeader = GameMode && !bInFinalRound && GameMode->GetCurrentStandingLeaderPlayerIndex() == PlayerIndex;
	const TCHAR* TitleText = bGrandWizardWinner ? TEXT("  GRAND WIZARD") : (bGrandWizardCandidate ? TEXT("  CANDIDATE") : (bStandingLeader ? TEXT("  FAVOR LEADER") : TEXT("")));
	const TCHAR* BotText = IsHudPlaytestBot(Wizard) ? TEXT("  BOT") : TEXT("");

	DrawRect(PanelColor, X - 8.0f, Y - 6.0f, RowWidth, RowHeight);
	DrawRect(PlayerColor, X, Y, 20.0f, 20.0f);

	FString PlayerText = FString::Printf(TEXT("P%d%s  Staff: %d  Favor: %d  Wins: %d%s"), PlayerIndex + 1, BotText, SegmentCount, Favor, RoundWins, TitleText);
	if (GameMode
		&& GameMode->GetActiveTrialType() == EWizardTrialType::StaffsAtDawn
		&& (GameMode->GetActiveTrialState() == EWizardTrialState::Active || GameMode->GetActiveTrialState() == EWizardTrialState::Results))
	{
		PlayerText = FString::Printf(TEXT("P%d%s  Score: %d  Staff: %d  Favor: %d  Wins: %d%s"), PlayerIndex + 1, BotText, GetHudStaffsAtDawnScore(GameMode, Wizard, PlayerIndex), SegmentCount, Favor, RoundWins, TitleText);
	}
	else if ((GameMode && GameMode->GetActiveTrialType() == EWizardTrialType::CauldronCatastrophe)
		|| (!GameMode && WizardGameState && WizardGameState->GetReplicatedActiveTrialType() == EWizardTrialType::CauldronCatastrophe))
	{
		const float InstabilityMultiplier = Wizard->GetReadableCauldronVialInstabilityMultiplier();
		const FString InstabilityText = InstabilityMultiplier > 1.0f
			? FString::Printf(TEXT("  Instability: %.2fx Stress"), InstabilityMultiplier)
			: FString();
		PlayerText = FString::Printf(TEXT("P%d%s  Vials: %d  %s%s  Staff: %d  Favor: %d  Wins: %d%s"), PlayerIndex + 1, BotText, Wizard->GetReadableCauldronVialCount(), *GetWizardCauldronVialDisplayName(Wizard->GetReadableActiveCauldronVial()), *InstabilityText, SegmentCount, Favor, RoundWins, TitleText);
	}
	DrawText(PlayerText, PlayerColor.ToFColor(true), X + 28.0f, Y - 1.0f, nullptr, TextScale);
	DrawText(FString::Printf(TEXT("Reward: %s"), *Wizard->GetCarriedBrewRewardName()), FColor::Cyan, X + 310.0f, Y + 52.0f, nullptr, 0.72f * TextScale);

	DrawValueBar(TEXT("Mana Slosh"), Wizard->GetReadableManaSlosh(), Wizard->GetReadableMaxManaSlosh(), FLinearColor(0.1f, 0.95f, 0.85f, 1.0f), X, Y + 30.0f);
	DrawValueBar(TEXT("Stress"), Wizard->GetReadableStaffStress(), Wizard->GetReadableMaxStaffStress(), FLinearColor(1.0f, 0.18f, 0.08f, 1.0f), X, Y + 52.0f);
}

void AWizardStaffHUD::DrawCompactPlayerRows(const AWizardStaffGameMode* GameMode, float X, float Y, bool bMinimalRows)
{
	const AWizardStaffGameState* WizardGameState = GetHudGameState(GetWorld());
	const TArray<const AWizardStaffWizardCharacter*> Wizards = GetVisibleWizards();
	if (Wizards.Num() <= 0)
	{
		return;
	}

	const float BoxWidth = bMinimalRows ? 330.0f : 520.0f;
	const float RowStride = bMinimalRows ? 22.0f : 38.0f;
	const float BoxHeight = static_cast<float>(Wizards.Num()) * RowStride + 10.0f;
	const EWizardPartyMatchState PartyState = GameMode ? GameMode->GetPartyMatchState() : (WizardGameState ? WizardGameState->GetReplicatedPartyMatchState() : EWizardPartyMatchState::PartyHall);
	const EWizardTrialState TrialState = GameMode ? GameMode->GetActiveTrialState() : (WizardGameState ? WizardGameState->GetReplicatedActiveTrialState() : EWizardTrialState::WaitingToStart);
	const EWizardTrialType TrialType = GameMode ? GameMode->GetActiveTrialType() : (WizardGameState ? WizardGameState->GetReplicatedActiveTrialType() : EWizardTrialType::MugRun);
	const bool bInPartyHall = PartyState == EWizardPartyMatchState::PartyHall || PartyState == EWizardPartyMatchState::Intermission;
	const bool bInFinalRound = PartyState == EWizardPartyMatchState::FinalRound;
	const bool bStaffsAtDawn = TrialType == EWizardTrialType::StaffsAtDawn && TrialState != EWizardTrialState::WaitingToStart;
	const bool bCauldronCatastrophe = TrialType == EWizardTrialType::CauldronCatastrophe && TrialState != EWizardTrialState::WaitingToStart;
	const int32 StandingLeaderIndex = GameMode ? GameMode->GetCurrentStandingLeaderPlayerIndex() : INDEX_NONE;
	const int32 FinalCandidateIndex = GameMode ? GameMode->GetGrandWizardCandidatePlayerIndex() : (WizardGameState ? WizardGameState->GetReplicatedFinalCandidateIndex() : INDEX_NONE);
	const int32 FinalWinnerIndex = GameMode ? GameMode->GetGrandWizardWinnerPlayerIndex() : (WizardGameState ? WizardGameState->GetReplicatedFinalWinnerIndex() : INDEX_NONE);
	const int32 FinalStealingPlayerIndex = GameMode ? GameMode->GetGrandWizardStealPlayerIndex() : (WizardGameState ? WizardGameState->GetReplicatedFinalStealingPlayerIndex() : INDEX_NONE);

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.50f), X - 8.0f, Y - 5.0f, BoxWidth, BoxHeight);

	float RowY = Y;
	for (const AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetHudPlayerIndex(GameMode, Wizard);
		const FLinearColor PlayerLinearColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex);
		const FColor PlayerColor = PlayerLinearColor.ToFColor(true);
		FString StatusTags;
		if (bInPartyHall)
		{
			StatusTags += IsHudPartyHallReady(GameMode, Wizard, PlayerIndex) ? TEXT(" READY") : TEXT(" NOT READY");
		}
		if (GameMode && PlayerIndex == StandingLeaderIndex && !bInFinalRound)
		{
			StatusTags += TEXT(" LEADER");
		}
		if (bInFinalRound && FinalCandidateIndex == PlayerIndex)
		{
			StatusTags += TEXT(" CANDIDATE");
		}
		if (bInFinalRound && FinalStealingPlayerIndex == PlayerIndex)
		{
			StatusTags += TEXT(" STEALING");
		}
		if (bInFinalRound && FinalWinnerIndex == PlayerIndex)
		{
			StatusTags += TEXT(" WINNER");
		}
		if (Wizard->IsMegaStaffBrewActive())
		{
			StatusTags += FString::Printf(TEXT(" MEGA %.0fs"), Wizard->GetMegaStaffRemainingTime());
		}
		if (Wizard->IsCauldronCursed())
		{
			StatusTags += FString::Printf(TEXT(" CURSED %.1fs"), Wizard->GetCauldronCurseRemainingTime());
		}
		if (bCauldronCatastrophe && Wizard->GetReadableCauldronVialInstabilityMultiplier() > 1.0f)
		{
			StatusTags += FString::Printf(TEXT(" INSTABILITY %.2fx"), Wizard->GetReadableCauldronVialInstabilityMultiplier());
		}
		if (IsHudPlaytestBot(Wizard))
		{
			StatusTags += TEXT(" BOT");
		}

		const FString RewardName = Wizard->GetCarriedBrewRewardName();
		const bool bHasReward = !RewardName.IsEmpty() && RewardName != TEXT("None");
		const int32 Favor = GetHudGrandWizardFavor(GameMode, Wizard, PlayerIndex);
		const int32 Wins = GetHudRoundWins(GameMode, Wizard, PlayerIndex);
		FString PlayerText = FString::Printf(TEXT("P%d  Staff %d  Favor %d  Wins %d%s"), PlayerIndex + 1, Wizard->GetStaffSegmentCount(), Favor, Wins, *StatusTags);
		if (bStaffsAtDawn)
		{
			PlayerText = FString::Printf(TEXT("P%d  Score %d  Staff %d  Favor %d  Wins %d%s"), PlayerIndex + 1, GetHudStaffsAtDawnScore(GameMode, Wizard, PlayerIndex), Wizard->GetStaffSegmentCount(), Favor, Wins, *StatusTags);
		}
		else if (bCauldronCatastrophe)
		{
			PlayerText = FString::Printf(TEXT("P%d  Vials %d %s  Staff %d  Favor %d  Wins %d%s"), PlayerIndex + 1, Wizard->GetReadableCauldronVialCount(), *GetWizardCauldronVialDisplayName(Wizard->GetReadableActiveCauldronVial()), Wizard->GetStaffSegmentCount(), Favor, Wins, *StatusTags);
		}
		if (!bMinimalRows && bHasReward)
		{
			PlayerText += FString::Printf(TEXT("  Reward: %s"), *RewardName);
		}

		DrawRect(PlayerLinearColor, X, RowY + 2.0f, 12.0f, 12.0f);
		DrawText(PlayerText, PlayerColor, X + 18.0f, RowY - 1.0f, nullptr, (bMinimalRows ? 0.60f : 0.66f) * TextScale);
		if (!bMinimalRows)
		{
			const float MeterY = RowY + 20.0f;
			DrawCompactValueMeter(TEXT("Slosh"), Wizard->GetReadableManaSlosh(), Wizard->GetReadableMaxManaSlosh(), FLinearColor(0.1f, 0.95f, 0.85f, 1.0f), X + 18.0f, MeterY, 160.0f);
			DrawCompactValueMeter(TEXT("Stress"), Wizard->GetReadableStaffStress(), Wizard->GetReadableMaxStaffStress(), FLinearColor(1.0f, 0.18f, 0.08f, 1.0f), X + 202.0f, MeterY, 178.0f);
		}
		RowY += RowStride;
	}
}

void AWizardStaffHUD::DrawCompactValueMeter(const FString& Label, float Current, float Max, const FLinearColor& FillColor, float X, float Y, float Width)
{
	const float SafeMax = FMath::Max(Max, 1.0f);
	const float Alpha = FMath::Clamp(Current / SafeMax, 0.0f, 1.0f);
	const float LabelWidth = 62.0f;
	const float BarWidthLocal = FMath::Max(Width - LabelWidth, 36.0f);
	const float BarHeightLocal = 6.0f;

	DrawText(FString::Printf(TEXT("%s %.0f%%"), *Label, Alpha * 100.0f), FColor::White, X, Y - 5.0f, nullptr, 0.48f * TextScale);
	DrawRect(FLinearColor(0.07f, 0.07f, 0.07f, 0.82f), X + LabelWidth, Y, BarWidthLocal, BarHeightLocal);
	DrawRect(FillColor, X + LabelWidth, Y, BarWidthLocal * Alpha, BarHeightLocal);
}

void AWizardStaffHUD::DrawPlaytestEventPanel(const AWizardStaffGameMode* GameMode, float SuggestedY)
{
	const AWizardStaffGameState* WizardGameState = GetHudGameState(GetWorld());
	if ((!GameMode && !WizardGameState) || !Canvas)
	{
		return;
	}

	TArray<FString> Lines;
	TArray<FColor> Colors;
	auto AddLine = [&Lines, &Colors](const FString& Text, const FColor& Color)
	{
		if (!Text.IsEmpty())
		{
			Lines.Add(Text);
			Colors.Add(Color);
		}
	};

	const EWizardPartyMatchState PartyState = GameMode ? GameMode->GetPartyMatchState() : WizardGameState->GetReplicatedPartyMatchState();
	const EWizardTrialState TrialState = GameMode ? GameMode->GetActiveTrialState() : WizardGameState->GetReplicatedActiveTrialState();
	const EWizardTrialType TrialType = GameMode ? GameMode->GetActiveTrialType() : WizardGameState->GetReplicatedActiveTrialType();
	const bool bInPartyHall = PartyState == EWizardPartyMatchState::PartyHall || PartyState == EWizardPartyMatchState::Intermission;
	const bool bInFinalRound = PartyState == EWizardPartyMatchState::FinalRound;
	float FinalStealProgressAlphaForHud = 0.0f;
	bool bShowFinalStealBarForHud = false;

	if (GameMode && bInPartyHall && TrialState == EWizardTrialState::WaitingToStart)
	{
		AddLine(FString::Printf(TEXT("Ready Bell: %d/%d ready"), GameMode->GetPartyHallReadyPlayerCount(), GetVisibleWizards().Num()), FColor::Cyan);
		AddLine(GameMode->GetPartyHallReadyFeedbackMessage(), FColor::Cyan);
	}
	else if (GameMode && PartyState == EWizardPartyMatchState::Results && TrialState == EWizardTrialState::Results)
	{
		AddLine(GameMode->GetMugRunWinnerMessage(), FColor::Yellow);
		AddLine(GameMode->GetGrandWizardFavorFeedbackMessage(), FColor::Green);
	}
	else if (GameMode && TrialType == EWizardTrialType::StaffsAtDawn && TrialState == EWizardTrialState::Active)
	{
		AddLine(GameMode->GetStaffsAtDawnFeedbackMessage(), FColor::Orange);
		AddLine(GameMode->GetGrandWizardFavorFeedbackMessage(), FColor::Green);
	}
	else if (TrialType == EWizardTrialType::CauldronCatastrophe && TrialState == EWizardTrialState::Active)
	{
		const int32 CursedPlayerIndex = GameMode ? GameMode->GetCauldronCursedPlayerIndex() : WizardGameState->GetReplicatedCauldronCursedPlayerIndex();
		const float CurseTime = GameMode ? GameMode->GetCauldronCurseRemainingTime() : WizardGameState->GetReplicatedCauldronCurseRemainingTime();
		const int32 BankingPlayerIndex = GameMode ? GameMode->GetCauldronBankingPlayerIndex() : WizardGameState->GetReplicatedCauldronBankingPlayerIndex();
		const int32 BankingTransferredCount = GameMode ? GameMode->GetCauldronBankingTransferredCount() : WizardGameState->GetReplicatedCauldronBankingTransferredCount();
		const float BankingProgressAlpha = GameMode ? GameMode->GetCauldronBankingProgressAlpha() : WizardGameState->GetReplicatedCauldronBankingProgressAlpha();
		if (BankingPlayerIndex != INDEX_NONE)
		{
			AddLine(FString::Printf(TEXT("BANKING P%d  %d banked  %.0f%%"), BankingPlayerIndex + 1, BankingTransferredCount, BankingProgressAlpha * 100.0f), FColor::Yellow);
		}
		if (CursedPlayerIndex != INDEX_NONE)
		{
			AddLine(FString::Printf(TEXT("P%d CURSED %.1fs - bonk to pass"), CursedPlayerIndex + 1, CurseTime), FColor::Red);
		}
	}
	else if (bInFinalRound)
	{
		if (GameMode)
		{
			const bool bStealActive = GameMode->GetGrandWizardStealPlayerIndex() != INDEX_NONE;
			const int32 ChallengerPlayerIndex = GameMode->GetGrandWizardCircleChallengerPlayerIndex();
			FinalStealProgressAlphaForHud = GameMode->GetGrandWizardStealProgressAlpha();
			bShowFinalStealBarForHud = bStealActive;
			AddLine(FString::Printf(TEXT("Candidate: %s  Favor %d"), *GameMode->GetGrandWizardCandidateText(), GameMode->GetPlayerGrandWizardFavor(GameMode->GetGrandWizardCandidatePlayerIndex())), FColor::Yellow);
			if (bStealActive)
			{
				AddLine(FString::Printf(TEXT("P%d stealing %.0f%%"), GameMode->GetGrandWizardStealPlayerIndex() + 1, GameMode->GetGrandWizardStealProgressAlpha() * 100.0f), FColor::Orange);
			}
			else if (GameMode->GetGrandWizardCandidateBonusRemainingTime() > 0.0f)
			{
				AddLine(FString::Printf(TEXT("Candidate protected %.1fs"), GameMode->GetGrandWizardCandidateBonusRemainingTime()), FColor::Yellow);
			}
			else if (GameMode->IsGrandWizardCandidateVulnerable())
			{
				AddLine(TEXT("VULNERABLE: hold the ritual circle"), FColor::Orange);
			}
			else if (ChallengerPlayerIndex != INDEX_NONE)
			{
				AddLine(FString::Printf(TEXT("P%d blocked: Candidate safe"), ChallengerPlayerIndex + 1), FColor::Cyan);
			}
			else
			{
				AddLine(TEXT("SAFE: bonk Candidate out"), FColor::Cyan);
			}
			AddLine(GameMode->GetGrandWizardFinalFeedbackMessage(), FColor::Yellow);
		}
		else if (WizardGameState)
		{
			const int32 CandidateIndex = WizardGameState->GetReplicatedFinalCandidateIndex();
			if (!WizardGameState->GetReplicatedResultMessage().IsEmpty())
			{
				AddLine(WizardGameState->GetReplicatedResultMessage(), FColor::Yellow);
			}
			else if (CandidateIndex != INDEX_NONE)
			{
				const bool bCandidateVulnerable = WizardGameState->IsReplicatedFinalCandidateVulnerable();
				const int32 StealingPlayerIndex = WizardGameState->GetReplicatedFinalStealingPlayerIndex();
				FinalStealProgressAlphaForHud = WizardGameState->GetReplicatedFinalStealProgressAlpha();
				bShowFinalStealBarForHud = StealingPlayerIndex != INDEX_NONE;
				AddLine(FString::Printf(TEXT("Candidate: P%d"), CandidateIndex + 1), FColor::Yellow);
				if (StealingPlayerIndex != INDEX_NONE)
				{
					AddLine(FString::Printf(TEXT("P%d stealing %.0f%%"), StealingPlayerIndex + 1, FinalStealProgressAlphaForHud * 100.0f), FColor::Orange);
				}
				else
				{
					AddLine(
						bCandidateVulnerable ? TEXT("VULNERABLE: hold the ritual circle") : TEXT("SAFE: bonk Candidate out"),
						bCandidateVulnerable ? FColor::Orange : FColor::Cyan);
				}
			}
		}
	}

	if (GameMode && !GameMode->GetGrandWizardWinnerMessage().IsEmpty())
	{
		AddLine(GameMode->GetGrandWizardWinnerMessage(), FColor::Yellow);
	}
	else if (!GameMode && WizardGameState && !bInFinalRound && !WizardGameState->GetReplicatedResultMessage().IsEmpty())
	{
		AddLine(WizardGameState->GetReplicatedResultMessage(), FColor::Yellow);
	}

	if (Lines.Num() <= 0)
	{
		return;
	}

	const float BoxWidth = FMath::Min(500.0f, Canvas->SizeX - (PanelX * 2.0f));
	float X = Canvas->SizeX - BoxWidth - PanelX;
	float Y = PanelY;
	if (X < PanelX + 470.0f)
	{
		X = PanelX;
		Y = SuggestedY;
	}

	const bool bDrawStealBar = bInFinalRound && bShowFinalStealBarForHud;
	const float LineHeight = 20.0f * TextScale;
	const float BoxHeight = 14.0f + (static_cast<float>(Lines.Num()) * LineHeight) + (bDrawStealBar ? 14.0f : 0.0f);
	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.54f), X - 8.0f, Y - 6.0f, BoxWidth, BoxHeight);
	for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
	{
		DrawText(Lines[LineIndex], Colors.IsValidIndex(LineIndex) ? Colors[LineIndex] : FColor::White, X, Y + (static_cast<float>(LineIndex) * LineHeight), nullptr, 0.66f * TextScale);
	}

	if (bDrawStealBar)
	{
		const float BarX = X;
		const float BarY = Y + (static_cast<float>(Lines.Num()) * LineHeight) + 2.0f;
		const float BarWidthSmall = BoxWidth - 18.0f;
		DrawRect(FLinearColor(0.08f, 0.08f, 0.08f, 0.92f), BarX, BarY, BarWidthSmall, 7.0f);
		DrawRect(FLinearColor(1.0f, 0.45f, 0.06f, 1.0f), BarX, BarY, BarWidthSmall * FinalStealProgressAlphaForHud, 7.0f);
	}
}

void AWizardStaffHUD::DrawPlaytestMatchSummary(const AWizardStaffGameMode* GameMode, float X, float Y)
{
	if (!GameMode)
	{
		return;
	}

	const int32 PlayerCount = GameMode->GetPlaytestStatsPlayerCount();
	if (PlayerCount <= 0)
	{
		return;
	}

	const float BoxWidth = 500.0f;
	const float LineHeight = 20.0f * TextScale;
	const float BoxHeight = 36.0f + (static_cast<float>(PlayerCount) * LineHeight);
	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.60f), X - 8.0f, Y - 6.0f, BoxWidth, BoxHeight);
	DrawText(TEXT("Match Summary (press H for Full Debug telemetry)"), FColor::Yellow, X, Y, nullptr, 0.68f * TextScale);

	float RowY = Y + 24.0f;
	for (int32 PlayerIndex = 0; PlayerIndex < PlayerCount; ++PlayerIndex)
	{
		const FWizardPlayerPlaytestStats Stats = GameMode->GetPlayerPlaytestStats(PlayerIndex);
		const FColor PlayerColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex).ToFColor(true);
		const FString WinnerText = Stats.bFinalWinner ? TEXT(" WINNER") : TEXT("");
		const FString SummaryText = FString::Printf(
			TEXT("P%d%s  Favor %d  Wins %d  Dawn %d  RO %d  Staff %d"),
			PlayerIndex + 1,
			*WinnerText,
			Stats.GrandWizardFavorEarned,
			Stats.RoundWins,
			Stats.StaffsAtDawnScore,
			Stats.StaffsAtDawnRingOutsCaused,
			Stats.FinalStaffSegmentCount);
		DrawText(SummaryText, PlayerColor, X, RowY, nullptr, 0.62f * TextScale);
		RowY += LineHeight;
	}
}

void AWizardStaffHUD::DrawMinimalStatePanel(const AWizardStaffGameMode* GameMode, float X, float Y)
{
	if (!Canvas)
	{
		return;
	}

	const AWizardStaffGameState* WizardGameState = GetHudGameState(GetWorld());
	TArray<FString> Lines;
	TArray<FColor> Colors;
	auto AddLine = [&Lines, &Colors](const FString& Text, const FColor& Color)
	{
		if (!Text.IsEmpty())
		{
			Lines.Add(Text);
			Colors.Add(Color);
		}
	};

	if (GameMode)
	{
		AddLine(FString::Printf(TEXT("%s | %s"), *GameMode->GetPartyMatchStateText(), *GameMode->GetActiveTrialName()), FColor::White);
		AddLine(GetTimerText(GameMode), FColor::Yellow);
		if (!GameMode->GetGrandWizardWinnerMessage().IsEmpty())
		{
			AddLine(GameMode->GetGrandWizardWinnerMessage(), FColor::Yellow);
		}
		else if (GameMode->GetPartyMatchState() == EWizardPartyMatchState::FinalRound)
		{
			AddLine(FString::Printf(TEXT("Candidate: %s"), *GameMode->GetGrandWizardCandidateText()), FColor::Yellow);
			if (GameMode->GetGrandWizardStealPlayerIndex() != INDEX_NONE)
			{
				AddLine(FString::Printf(TEXT("Steal %.0f%%"), GameMode->GetGrandWizardStealProgressAlpha() * 100.0f), FColor::Orange);
			}
		}
		else if (GameMode->GetActiveTrialType() == EWizardTrialType::StaffsAtDawn && GameMode->GetActiveTrialState() == EWizardTrialState::Active)
		{
			AddLine(GameMode->GetStaffsAtDawnFeedbackMessage(), FColor::Orange);
		}
		else if (GameMode->GetActiveTrialType() == EWizardTrialType::CauldronCatastrophe && GameMode->GetActiveTrialState() == EWizardTrialState::Active && GameMode->GetCauldronCursedPlayerIndex() != INDEX_NONE)
		{
			AddLine(FString::Printf(TEXT("P%d CURSED %.1fs"), GameMode->GetCauldronCursedPlayerIndex() + 1, GameMode->GetCauldronCurseRemainingTime()), FColor::Red);
		}
		else if (GameMode->GetPartyMatchState() == EWizardPartyMatchState::PartyHall || GameMode->GetPartyMatchState() == EWizardPartyMatchState::Intermission)
		{
			AddLine(FString::Printf(TEXT("Ready %d/%d"), GameMode->GetPartyHallReadyPlayerCount(), GetVisibleWizards().Num()), FColor::Cyan);
		}
	}
	else if (WizardGameState)
	{
		AddLine(FString::Printf(TEXT("%s | %s"), *WizardGameState->GetReplicatedPartyMatchStateText(), *WizardGameState->GetReplicatedActiveTrialName()), FColor::White);
		AddLine(GetTimerText(GameMode), FColor::Yellow);
		if (!WizardGameState->GetReplicatedResultMessage().IsEmpty())
		{
			AddLine(WizardGameState->GetReplicatedResultMessage(), FColor::Yellow);
		}
		else if (WizardGameState->GetReplicatedPartyMatchState() == EWizardPartyMatchState::FinalRound)
		{
			const int32 CandidateIndex = WizardGameState->GetReplicatedFinalCandidateIndex();
			if (CandidateIndex != INDEX_NONE)
			{
				const bool bCandidateVulnerable = WizardGameState->IsReplicatedFinalCandidateVulnerable();
				const int32 StealingPlayerIndex = WizardGameState->GetReplicatedFinalStealingPlayerIndex();
				AddLine(FString::Printf(TEXT("Candidate: P%d"), CandidateIndex + 1), FColor::Yellow);
				if (StealingPlayerIndex != INDEX_NONE)
				{
					AddLine(FString::Printf(TEXT("Steal %.0f%%"), WizardGameState->GetReplicatedFinalStealProgressAlpha() * 100.0f), FColor::Orange);
				}
				else
				{
					AddLine(bCandidateVulnerable ? TEXT("Vulnerable") : TEXT("Safe"), bCandidateVulnerable ? FColor::Orange : FColor::Cyan);
				}
			}
		}
	else if (WizardGameState->GetReplicatedActiveTrialType() == EWizardTrialType::CauldronCatastrophe)
	{
		if (WizardGameState->GetReplicatedCauldronBankingPlayerIndex() != INDEX_NONE)
		{
			AddLine(FString::Printf(TEXT("BANKING P%d  %d  %.0f%%"), WizardGameState->GetReplicatedCauldronBankingPlayerIndex() + 1, WizardGameState->GetReplicatedCauldronBankingTransferredCount(), WizardGameState->GetReplicatedCauldronBankingProgressAlpha() * 100.0f), FColor::Yellow);
		}
		else if (WizardGameState->GetReplicatedCauldronCursedPlayerIndex() != INDEX_NONE)
		{
			AddLine(FString::Printf(TEXT("P%d CURSED %.1fs"), WizardGameState->GetReplicatedCauldronCursedPlayerIndex() + 1, WizardGameState->GetReplicatedCauldronCurseRemainingTime()), FColor::Red);
		}
	}
	}
	else
	{
		AddLine(TEXT("Wizard's Staff"), FColor::White);
	}

	if (Lines.Num() <= 0)
	{
		return;
	}

	const float BoxWidth = 360.0f;
	const float LineHeight = 20.0f * TextScale;
	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.48f), X - 8.0f, Y - 6.0f, BoxWidth, 14.0f + (static_cast<float>(Lines.Num()) * LineHeight));
	for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
	{
		DrawText(Lines[LineIndex], Colors.IsValidIndex(LineIndex) ? Colors[LineIndex] : FColor::White, X, Y + (static_cast<float>(LineIndex) * LineHeight), nullptr, 0.64f * TextScale);
	}
}

void AWizardStaffHUD::DrawValueBar(const FString& Label, float Current, float Max, const FLinearColor& FillColor, float X, float Y)
{
	const float SafeMax = FMath::Max(Max, 1.0f);
	const float Alpha = FMath::Clamp(Current / SafeMax, 0.0f, 1.0f);

	DrawText(FString::Printf(TEXT("%s %.0f/%.0f"), *Label, Current, Max), FColor::White, X, Y - 4.0f, nullptr, 0.78f * TextScale);
	DrawRect(FLinearColor(0.08f, 0.08f, 0.08f, 0.9f), X + 108.0f, Y, BarWidth, BarHeight);
	DrawRect(FillColor, X + 108.0f, Y, BarWidth * Alpha, BarHeight);
}

void AWizardStaffHUD::DrawScoringClarityPanel()
{
	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}

	const EWizardPartyMatchState PartyState = GameMode->GetPartyMatchState();
	const EWizardTrialState TrialState = GameMode->GetActiveTrialState();
	const bool bInPartyHall = PartyState == EWizardPartyMatchState::PartyHall || PartyState == EWizardPartyMatchState::Intermission;
	if (bInPartyHall && TrialState == EWizardTrialState::WaitingToStart)
	{
		DrawPartyHallStandingsPanel(GameMode);
		return;
	}

	if (PartyState == EWizardPartyMatchState::Results && TrialState == EWizardTrialState::Results)
	{
		DrawTrialResultsPanel(GameMode);
		return;
	}

	if (PartyState == EWizardPartyMatchState::Trial
		&& TrialState == EWizardTrialState::Active
		&& GameMode->GetActiveTrialType() == EWizardTrialType::StaffsAtDawn)
	{
		DrawStaffsAtDawnScorePanel(GameMode);
		return;
	}

	if (PartyState == EWizardPartyMatchState::Trial
		&& TrialState == EWizardTrialState::Active
		&& GameMode->GetActiveTrialType() == EWizardTrialType::CauldronCatastrophe)
	{
		DrawCauldronScorePanel(GameMode);
		return;
	}

	if (PartyState == EWizardPartyMatchState::FinalRound && GameMode->GetGrandWizardWinnerMessage().IsEmpty())
	{
		DrawFinalIntroPanel(GameMode);
	}
}

void AWizardStaffHUD::DrawPartyHallStandingsPanel(const AWizardStaffGameMode* GameMode)
{
	if (!GameMode)
	{
		return;
	}

	const TArray<const AWizardStaffWizardCharacter*> Wizards = GetVisibleWizards();
	const float BoxWidth = 620.0f;
	const FVector2D Origin = GetSidePanelOrigin(Wizards.Num(), BoxWidth);
	const float LineHeight = 22.0f * TextScale;
	const FString FavorFeedback = GameMode->GetGrandWizardFavorFeedbackMessage();
	const bool bHasFavorFeedback = !FavorFeedback.IsEmpty();
	const FString ReadyFeedback = GameMode->GetPartyHallReadyFeedbackMessage();
	const bool bHasReadyFeedback = !ReadyFeedback.IsEmpty();
	const float FeedbackLines = (bHasFavorFeedback ? 1.0f : 0.0f) + (bHasReadyFeedback ? 1.0f : 0.0f);
	const float BoxHeight = 118.0f + (FeedbackLines * LineHeight) + (static_cast<float>(Wizards.Num()) * LineHeight);
	const int32 LeaderIndex = GameMode->GetCurrentStandingLeaderPlayerIndex();

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.74f), Origin.X - 8.0f, Origin.Y - 6.0f, BoxWidth, BoxHeight);
	DrawText(TEXT("Party Hall Standings"), FColor::Yellow, Origin.X, Origin.Y, nullptr, 0.95f * TextScale);
	DrawText(FString::Printf(TEXT("Preset: %s"), *GameMode->GetActivePrototypeTuningPresetText()), FColor::White, Origin.X, Origin.Y + LineHeight, nullptr, 0.76f * TextScale);
	const FString LeaderText = LeaderIndex == INDEX_NONE ? TEXT("Favor Leader: Tie / no clear leader") : FString::Printf(TEXT("Favor Leader: P%d  Favor %d"), LeaderIndex + 1, GameMode->GetPlayerGrandWizardFavor(LeaderIndex));
	DrawText(LeaderText, FColor::White, Origin.X, Origin.Y + (LineHeight * 2.0f), nullptr, 0.76f * TextScale);
	DrawText(FString::Printf(TEXT("Ready Bell: %d/%d ready"), GameMode->GetPartyHallReadyPlayerCount(), Wizards.Num()), FColor::Cyan, Origin.X, Origin.Y + (LineHeight * 3.0f), nullptr, 0.76f * TextScale);

	float RowY = Origin.Y + (LineHeight * 4.25f);
	if (bHasFavorFeedback)
	{
		DrawText(FavorFeedback, FColor::Green, Origin.X, RowY, nullptr, 0.74f * TextScale);
		RowY += LineHeight;
	}
	if (bHasReadyFeedback)
	{
		DrawText(ReadyFeedback, FColor::Cyan, Origin.X, RowY, nullptr, 0.74f * TextScale);
		RowY += LineHeight;
	}

	for (const AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetHudPlayerIndex(GameMode, Wizard);
		const FColor PlayerColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex).ToFColor(true);
		const FString ReadyText = GameMode->IsPartyHallPlayerReady(PlayerIndex) ? TEXT("READY") : TEXT("NOT READY");
		const FString PlayerText = FString::Printf(
			TEXT("P%d  %s  Favor %d  Wins %d  Staff %d%s"),
			PlayerIndex + 1,
			*ReadyText,
			GameMode->GetPlayerGrandWizardFavor(PlayerIndex),
			GameMode->GetPlayerRoundWins(PlayerIndex),
			Wizard->GetStaffSegmentCount(),
			PlayerIndex == LeaderIndex ? TEXT("  FAVOR LEADER") : TEXT(""));
		DrawText(PlayerText, PlayerColor, Origin.X, RowY, nullptr, 0.76f * TextScale);
		RowY += LineHeight;
	}
}

void AWizardStaffHUD::DrawStaffsAtDawnScorePanel(const AWizardStaffGameMode* GameMode)
{
	if (!GameMode)
	{
		return;
	}

	const TArray<const AWizardStaffWizardCharacter*> Wizards = GetVisibleWizards();
	const float BoxWidth = 520.0f;
	const FVector2D Origin = GetSidePanelOrigin(Wizards.Num(), BoxWidth);
	const float LineHeight = 22.0f * TextScale;
	const FString FeedbackMessage = GameMode->GetStaffsAtDawnFeedbackMessage();
	const bool bHasFeedback = !FeedbackMessage.IsEmpty();
	const FString FavorFeedback = GameMode->GetGrandWizardFavorFeedbackMessage();
	const bool bHasFavorFeedback = !FavorFeedback.IsEmpty();
	const float FeedbackHeight = bHasFeedback ? LineHeight * 1.25f : 0.0f;
	const float FavorFeedbackHeight = bHasFavorFeedback ? LineHeight * 1.05f : 0.0f;
	const float BoxHeight = 72.0f + FeedbackHeight + FavorFeedbackHeight + (static_cast<float>(Wizards.Num()) * LineHeight);

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.72f), Origin.X - 8.0f, Origin.Y - 6.0f, BoxWidth, BoxHeight);
	DrawText(TEXT("Staffs at Dawn"), FColor::Yellow, Origin.X, Origin.Y, nullptr, 0.95f * TextScale);
	const FString StaffRewardText = GameMode->StaffsAtDawnTuning.LandedBonksPerStaffSegment > 0
		? FString::Printf(TEXT("%d bonks = +1 staff"), GameMode->StaffsAtDawnTuning.LandedBonksPerStaffSegment)
		: FString(TEXT("bonk staff rewards off"));
	DrawText(
		FString::Printf(TEXT("Bonk +%d   Ring-out +%d   %s"), GameMode->StaffsAtDawnTuning.PointsPerBonk, GameMode->StaffsAtDawnTuning.PointsPerOutOfArena, *StaffRewardText),
		FColor::White,
		Origin.X,
		Origin.Y + LineHeight,
		nullptr,
		0.76f * TextScale);

	float RowY = Origin.Y + (LineHeight * 2.25f);
	if (bHasFeedback)
	{
		const FColor FeedbackColor = FeedbackMessage.Contains(TEXT("Ring-out")) ? FColor::Red : FColor::Orange;
		DrawText(FeedbackMessage, FeedbackColor, Origin.X, RowY, nullptr, 0.78f * TextScale);
		RowY += FeedbackHeight;
	}
	if (bHasFavorFeedback)
	{
		DrawText(FavorFeedback, FColor::Green, Origin.X, RowY, nullptr, 0.72f * TextScale);
		RowY += FavorFeedbackHeight;
	}

	for (const AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetHudPlayerIndex(GameMode, Wizard);
		const FColor PlayerColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex).ToFColor(true);
		const FString MegaStaffText = Wizard->IsMegaStaffBrewActive()
			? FString::Printf(TEXT("  MEGA %.0fs"), Wizard->GetMegaStaffRemainingTime())
			: FString();
		const FString PlayerText = FString::Printf(
			TEXT("P%d  Score %d  Staff %d  Favor %d%s"),
			PlayerIndex + 1,
			GameMode->GetStaffsAtDawnScore(PlayerIndex),
			Wizard->GetStaffSegmentCount(),
			GameMode->GetPlayerGrandWizardFavor(PlayerIndex),
			*MegaStaffText);
		DrawText(PlayerText, PlayerColor, Origin.X, RowY, nullptr, 0.76f * TextScale);
		RowY += LineHeight;
	}
}

void AWizardStaffHUD::DrawCauldronScorePanel(const AWizardStaffGameMode* GameMode)
{
	if (!GameMode)
	{
		return;
	}

	const TArray<const AWizardStaffWizardCharacter*> Wizards = GetVisibleWizards();
	const float BoxWidth = 520.0f;
	const FVector2D Origin = GetSidePanelOrigin(Wizards.Num(), BoxWidth);
	const float LineHeight = 22.0f * TextScale;
	const int32 BankingPlayerIndex = GameMode->GetCauldronBankingPlayerIndex();
	const bool bBankingActive = BankingPlayerIndex != INDEX_NONE;
	const float BoxHeight = (bBankingActive ? 98.0f : 76.0f) + (static_cast<float>(Wizards.Num()) * LineHeight);
	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.72f), Origin.X - 8.0f, Origin.Y - 6.0f, BoxWidth, BoxHeight);
	DrawText(TEXT("Cauldron Catastrophe"), FColor::Yellow, Origin.X, Origin.Y, nullptr, 0.95f * TextScale);
	const int32 CursedPlayerIndex = GameMode->GetCauldronCursedPlayerIndex();
	const FString ObjectiveText = CursedPlayerIndex == INDEX_NONE
		? TEXT("Quick Bonk the cauldron to bank your newest vials")
		: FString::Printf(TEXT("P%d CURSED %.1fs - bonk another wizard to pass"), CursedPlayerIndex + 1, GameMode->GetCauldronCurseRemainingTime());
	DrawText(ObjectiveText, CursedPlayerIndex == INDEX_NONE ? FColor::White : FColor::Red, Origin.X, Origin.Y + LineHeight, nullptr, 0.76f * TextScale);
	if (bBankingActive)
	{
		DrawText(FString::Printf(TEXT("Banking: P%d  Transferred: %d  Next: %.0f%%"), BankingPlayerIndex + 1, GameMode->GetCauldronBankingTransferredCount(), GameMode->GetCauldronBankingProgressAlpha() * 100.0f), FColor::Yellow, Origin.X, Origin.Y + (LineHeight * 1.9f), nullptr, 0.70f * TextScale);
	}

	float RowY = Origin.Y + (LineHeight * (bBankingActive ? 3.25f : 2.25f));
	for (const AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (!Wizard)
		{
			continue;
		}
		const int32 PlayerIndex = GetHudPlayerIndex(GameMode, Wizard);
		const FColor PlayerColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex).ToFColor(true);
		const float InstabilityMultiplier = Wizard->GetReadableCauldronVialInstabilityMultiplier();
		const FString InstabilityText = InstabilityMultiplier > 1.0f
			? FString::Printf(TEXT("  Instability %.2fx Stress"), InstabilityMultiplier)
			: FString();
		DrawText(FString::Printf(TEXT("P%d  Banked %d  Vials %d  Active %s%s%s"), PlayerIndex + 1, GameMode->GetCauldronScore(PlayerIndex), Wizard->GetReadableCauldronVialCount(), *GetWizardCauldronVialDisplayName(Wizard->GetReadableActiveCauldronVial()), *InstabilityText, PlayerIndex == CursedPlayerIndex ? TEXT("  CURSED") : TEXT("")), PlayerColor, Origin.X, RowY, nullptr, 0.76f * TextScale);
		RowY += LineHeight;
	}
}

void AWizardStaffHUD::DrawTrialResultsPanel(const AWizardStaffGameMode* GameMode)
{
	if (!GameMode)
	{
		return;
	}

	const TArray<const AWizardStaffWizardCharacter*> Wizards = GetVisibleWizards();
	const float BoxWidth = 680.0f;
	const FVector2D Origin = GetSidePanelOrigin(Wizards.Num(), BoxWidth);
	const float LineHeight = 22.0f * TextScale;
	const FString FavorFeedback = GameMode->GetGrandWizardFavorFeedbackMessage();
	const bool bHasFavorFeedback = !FavorFeedback.IsEmpty();
	const float BoxHeight = (bHasFavorFeedback ? 94.0f : 72.0f) + (static_cast<float>(Wizards.Num()) * LineHeight);
	const bool bStaffsAtDawnResults = GameMode->GetActiveTrialType() == EWizardTrialType::StaffsAtDawn;
	const bool bCauldronResults = GameMode->GetActiveTrialType() == EWizardTrialType::CauldronCatastrophe;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.80f), Origin.X - 8.0f, Origin.Y - 6.0f, BoxWidth, BoxHeight);
	DrawText(bStaffsAtDawnResults ? TEXT("Staffs at Dawn Results") : (bCauldronResults ? TEXT("Cauldron Catastrophe Results") : TEXT("Trial Results")), FColor::Yellow, Origin.X, Origin.Y, nullptr, 0.95f * TextScale);
	DrawText(GameMode->GetMugRunWinnerMessage(), FColor::White, Origin.X, Origin.Y + LineHeight, nullptr, 0.76f * TextScale);

	float RowY = Origin.Y + (LineHeight * 2.25f);
	if (bHasFavorFeedback)
	{
		DrawText(FavorFeedback, FColor::Green, Origin.X, RowY, nullptr, 0.74f * TextScale);
		RowY += LineHeight;
	}

	for (const AWizardStaffWizardCharacter* Wizard : Wizards)
	{
		if (!Wizard)
		{
			continue;
		}

		const int32 PlayerIndex = GetHudPlayerIndex(GameMode, Wizard);
		const FColor PlayerColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex).ToFColor(true);
		const FString PlayerText = bStaffsAtDawnResults
			? FString::Printf(TEXT("P%d  Score %d  Staff %d  Favor %d  Round Wins %d"), PlayerIndex + 1, GameMode->GetStaffsAtDawnScore(PlayerIndex), Wizard->GetStaffSegmentCount(), GameMode->GetPlayerGrandWizardFavor(PlayerIndex), GameMode->GetPlayerRoundWins(PlayerIndex))
			: (bCauldronResults
				? FString::Printf(TEXT("P%d  Banked %d  Staff %d  Favor %d  Round Wins %d"), PlayerIndex + 1, GameMode->GetCauldronScore(PlayerIndex), Wizard->GetStaffSegmentCount(), GameMode->GetPlayerGrandWizardFavor(PlayerIndex), GameMode->GetPlayerRoundWins(PlayerIndex))
				: FString::Printf(TEXT("P%d  Staff %d  Favor %d  Round Wins %d"), PlayerIndex + 1, Wizard->GetStaffSegmentCount(), GameMode->GetPlayerGrandWizardFavor(PlayerIndex), GameMode->GetPlayerRoundWins(PlayerIndex)));
		DrawText(PlayerText, PlayerColor, Origin.X, RowY, nullptr, 0.76f * TextScale);
		RowY += LineHeight;
	}
}

void AWizardStaffHUD::DrawFinalIntroPanel(const AWizardStaffGameMode* GameMode)
{
	if (!GameMode)
	{
		return;
	}

	const TArray<const AWizardStaffWizardCharacter*> Wizards = GetVisibleWizards();
	const float BoxWidth = 620.0f;
	const FVector2D Origin = GetSidePanelOrigin(Wizards.Num(), BoxWidth);
	const float LineHeight = 22.0f * TextScale;
	const FString StaffSetupText = GameMode->GetGrandWizardFinalStaffSetupMessage();
	const bool bHasStaffSetupText = !StaffSetupText.IsEmpty();
	const float BoxHeight = bHasStaffSetupText ? 188.0f : 166.0f;
	const bool bStealActive = GameMode->GetGrandWizardStealPlayerIndex() != INDEX_NONE;
	const float StealAlpha = GameMode->GetGrandWizardStealProgressAlpha();
	const int32 ChallengerPlayerIndex = GameMode->GetGrandWizardCircleChallengerPlayerIndex();
	const int32 CandidatePlayerIndex = GameMode->GetGrandWizardCandidatePlayerIndex();

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.78f), Origin.X - 8.0f, Origin.Y - 6.0f, BoxWidth, BoxHeight);
	DrawText(TEXT("Grand Wizard Final"), FColor::Yellow, Origin.X, Origin.Y, nullptr, 0.95f * TextScale);
	DrawText(FString::Printf(TEXT("Timer %.0fs  |  Candidate %s  |  Favor %d"), GameMode->GetFinalRoundRemainingTime(), *GameMode->GetGrandWizardCandidateText(), GameMode->GetPlayerGrandWizardFavor(CandidatePlayerIndex)), FColor::White, Origin.X, Origin.Y + LineHeight, nullptr, 0.82f * TextScale);
	DrawText(GameMode->GetGrandWizardCandidateIntroMessage(), FColor::White, Origin.X, Origin.Y + (LineHeight * 2.0f), nullptr, 0.68f * TextScale);

	FString StatusText;
	FColor StatusColor = FColor::Cyan;
	if (bStealActive)
	{
		StatusText = FString::Printf(TEXT("P%d is stealing the title"), GameMode->GetGrandWizardStealPlayerIndex() + 1);
		StatusColor = FColor::Orange;
	}
	else if (GameMode->GetGrandWizardCandidateBonusRemainingTime() > 0.0f)
	{
		StatusText = FString::Printf(TEXT("Candidate opening bonus %.1fs"), GameMode->GetGrandWizardCandidateBonusRemainingTime());
		StatusColor = FColor::Yellow;
	}
	else if (GameMode->IsGrandWizardCandidateVulnerable())
	{
		StatusText = TEXT("VULNERABLE: hold the ritual circle");
		StatusColor = FColor::Orange;
	}
	else if (ChallengerPlayerIndex != INDEX_NONE)
	{
		StatusText = FString::Printf(TEXT("P%d blocked: Candidate safe"), ChallengerPlayerIndex + 1);
	}
	else
	{
		StatusText = TEXT("SAFE: bonk Candidate out, then hold circle");
	}

	DrawText(StatusText, StatusColor, Origin.X, Origin.Y + (LineHeight * 3.25f), nullptr, 0.78f * TextScale);
	const float BarX = Origin.X;
	const float BarY = Origin.Y + (LineHeight * 4.35f);
	const float BarW = BoxWidth - 28.0f;
	const float BarH = 14.0f;
	DrawText(TEXT("Steal Progress"), FColor::White, BarX, BarY - 19.0f, nullptr, 0.66f * TextScale);
	DrawRect(FLinearColor(0.08f, 0.08f, 0.08f, 0.92f), BarX + 118.0f, BarY - 12.0f, BarW - 118.0f, BarH);
	DrawRect(bStealActive ? FLinearColor(1.0f, 0.45f, 0.06f, 1.0f) : FLinearColor(0.18f, 0.18f, 0.18f, 1.0f), BarX + 118.0f, BarY - 12.0f, (BarW - 118.0f) * StealAlpha, BarH);
	DrawText(TEXT("Winner: Candidate when timer ends"), FColor::White, Origin.X, Origin.Y + (LineHeight * 5.2f), nullptr, 0.68f * TextScale);
	if (bHasStaffSetupText)
	{
		DrawText(StaffSetupText, FColor::Yellow, Origin.X, Origin.Y + (LineHeight * 6.1f), nullptr, 0.64f * TextScale);
	}
}

void AWizardStaffHUD::DrawCompactFinalHud(const AWizardStaffGameMode* GameMode)
{
	if (!GameMode || !Canvas)
	{
		return;
	}

	const bool bStealActive = GameMode->GetGrandWizardStealPlayerIndex() != INDEX_NONE;
	const float StealAlpha = GameMode->GetGrandWizardStealProgressAlpha();
	const int32 ChallengerPlayerIndex = GameMode->GetGrandWizardCircleChallengerPlayerIndex();
	const int32 CandidatePlayerIndex = GameMode->GetGrandWizardCandidatePlayerIndex();
	const FString StaffSetupText = GameMode->GetGrandWizardFinalStaffSetupMessage();
	const bool bHasStaffSetupText = !StaffSetupText.IsEmpty();
	FString StatusText;
	FColor StatusColor = FColor::Cyan;
	if (bStealActive)
	{
		StatusText = FString::Printf(TEXT("P%d stealing %.0f%%"), GameMode->GetGrandWizardStealPlayerIndex() + 1, StealAlpha * 100.0f);
		StatusColor = FColor::Orange;
	}
	else if (GameMode->GetGrandWizardCandidateBonusRemainingTime() > 0.0f)
	{
		StatusText = FString::Printf(TEXT("Candidate protected %.1fs"), GameMode->GetGrandWizardCandidateBonusRemainingTime());
		StatusColor = FColor::Yellow;
	}
	else if (GameMode->IsGrandWizardCandidateVulnerable())
	{
		StatusText = TEXT("VULNERABLE: hold the circle");
		StatusColor = FColor::Orange;
	}
	else if (ChallengerPlayerIndex != INDEX_NONE)
	{
		StatusText = FString::Printf(TEXT("P%d blocked: Candidate safe"), ChallengerPlayerIndex + 1);
	}
	else
	{
		StatusText = TEXT("SAFE: bonk Candidate out");
	}

	const FString FeedbackText = GameMode->GetGrandWizardFinalFeedbackMessage();
	const bool bShowCompactStaffSetup = bHasStaffSetupText && !FeedbackText.IsEmpty();
	const float BoxWidth = 690.0f;
	const float BoxHeight = FeedbackText.IsEmpty() ? 66.0f : (bShowCompactStaffSetup ? 104.0f : 84.0f);
	const float X = (Canvas->SizeX - BoxWidth) * 0.5f;
	const float Y = 14.0f;
	const FString FinalText = FString::Printf(
		TEXT("Grand Wizard Final %.0fs  |  Candidate %s  |  Favor %d"),
		GameMode->GetFinalRoundRemainingTime(),
		*GameMode->GetGrandWizardCandidateText(),
		GameMode->GetPlayerGrandWizardFavor(CandidatePlayerIndex));

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.64f), X, Y, BoxWidth, BoxHeight);
	DrawText(FinalText, FColor::White, X + 12.0f, Y + 8.0f, nullptr, 0.78f * TextScale);
	DrawText(StatusText, StatusColor, X + 12.0f, Y + 29.0f, nullptr, 0.72f * TextScale);

	const float BarX = X + 12.0f;
	const float BarY = Y + 53.0f;
	const float BarLabelWidth = 102.0f;
	const float BarWidthSmall = BoxWidth - BarLabelWidth - 24.0f;
	const float BarHeightSmall = 8.0f;
	DrawText(bStealActive ? TEXT("Stealing") : TEXT("Steal"), FColor::White, BarX, BarY - 7.0f, nullptr, 0.58f * TextScale);
	DrawRect(FLinearColor(0.08f, 0.08f, 0.08f, 0.92f), BarX + BarLabelWidth, BarY, BarWidthSmall, BarHeightSmall);
	DrawRect(bStealActive ? FLinearColor(1.0f, 0.45f, 0.06f, 1.0f) : FLinearColor(0.16f, 0.16f, 0.16f, 1.0f), BarX + BarLabelWidth, BarY, BarWidthSmall * StealAlpha, BarHeightSmall);

	if (!FeedbackText.IsEmpty())
	{
		DrawText(FeedbackText, FColor::Yellow, X + 12.0f, Y + 66.0f, nullptr, 0.66f * TextScale);
	}
	if (bShowCompactStaffSetup)
	{
		DrawText(StaffSetupText, FColor::White, X + 12.0f, Y + 84.0f, nullptr, 0.56f * TextScale);
	}
}

void AWizardStaffHUD::DrawWinnerMessage()
{
	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}

	const FString WinnerText = !GameMode->GetGrandWizardWinnerMessage().IsEmpty()
		? GameMode->GetGrandWizardWinnerMessage()
		: GameMode->GetMugRunWinnerMessage();
	if (WinnerText.IsEmpty())
	{
		return;
	}
	const float BoxWidth = 560.0f;
	const float BoxHeight = 58.0f;
	const float X = (Canvas->SizeX - BoxWidth) * 0.5f;
	const float Y = 78.0f;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.82f), X, Y, BoxWidth, BoxHeight);
	DrawText(WinnerText, FColor::Yellow, X + 18.0f, Y + 18.0f, nullptr, 1.12f * TextScale);
}

void AWizardStaffHUD::DrawMatchSummary()
{
	const AWizardStaffGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AWizardStaffGameMode>() : nullptr;
	if (!GameMode || GameMode->GetGrandWizardWinnerMessage().IsEmpty())
	{
		return;
	}

	const int32 PlayerCount = GameMode->GetPlaytestStatsPlayerCount();
	if (PlayerCount <= 0)
	{
		return;
	}

	float X = PanelX + RowWidth + 28.0f;
	float Y = PanelY + 42.0f;
	float BoxWidth = 650.0f;
	if (Canvas && X + BoxWidth + PanelX > Canvas->SizeX)
	{
		X = PanelX;
		Y = PanelY + 42.0f + (static_cast<float>(PlayerCount) * (RowHeight + 8.0f)) + 18.0f;
		BoxWidth = FMath::Max(520.0f, Canvas->SizeX - (PanelX * 2.0f));
	}

	const float LineHeight = 22.0f * TextScale;
	const float HeaderHeight = 56.0f;
	const float PlayerBlockHeight = 88.0f * TextScale;
	const float BoxHeight = HeaderHeight + (static_cast<float>(PlayerCount) * PlayerBlockHeight) + 16.0f;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.78f), X - 8.0f, Y - 6.0f, BoxWidth, BoxHeight);
	DrawText(TEXT("Playtest Match Summary"), FColor::Yellow, X, Y, nullptr, 0.95f * TextScale);
	DrawText(
		FString::Printf(
			TEXT("Staffs at Dawn: Ring-outs %d  Rounds %d  Avg Respawns/Round %.1f"),
			GameMode->GetStaffsAtDawnTelemetryRingOuts(),
			GameMode->GetStaffsAtDawnTelemetryRoundsCompleted(),
			GameMode->GetAverageStaffsAtDawnRespawnsPerRound()),
		FColor::Orange,
		X,
		Y + LineHeight,
		nullptr,
		0.70f * TextScale);

	float RowY = Y + HeaderHeight;
	for (int32 PlayerIndex = 0; PlayerIndex < PlayerCount; ++PlayerIndex)
	{
		const FWizardPlayerPlaytestStats Stats = GameMode->GetPlayerPlaytestStats(PlayerIndex);
		const FColor PlayerColor = AWizardStaffWizardCharacter::GetPrototypePlayerColor(PlayerIndex).ToFColor(true);
		const FString WinnerText = Stats.bFinalWinner ? TEXT(" WINNER") : TEXT("");
		const FString LineOne = FString::Printf(
			TEXT("P%d%s  Favor %d  Mugs %d  Dawn %d  DawnRO %d  DawnSeg+ %d  Staff %d  Wins %d"),
			PlayerIndex + 1,
			*WinnerText,
			Stats.GrandWizardFavorEarned,
			Stats.MugsCollected,
			Stats.StaffsAtDawnScore,
			Stats.StaffsAtDawnRingOutsCaused,
			Stats.StaffsAtDawnCombatSegmentsGained,
			Stats.FinalStaffSegmentCount,
			Stats.RoundWins);
		const FString LineTwo = FString::Printf(
			TEXT("Seg+ %d  Snap %d  Bonks %d/%d  Clash S/W/T/RO %d/%d/%d/%d  HitBy %d  Respawn %d  Candidate %.0fs"),
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
			Stats.TimeAsGrandWizardCandidate);
		const FString LineThree = FString::Printf(
			TEXT("Broom U/S/F %d/%d/%d  Mega P/Seg/Snap/RO %d/%d/%d/%d"),
			Stats.BroomBoostsUsed,
			Stats.BroomBoostRingOutSaves,
			Stats.BroomBoostFailedRingOutRecoveries,
			Stats.MegaStaffPickupsCollected,
			Stats.MegaStaffSegmentsGranted,
			Stats.MegaStaffSegmentsSnappedDuringEffect,
			Stats.MegaStaffRingOutsCaused);
		const FString LineFour = FString::Printf(
			TEXT("Pinball R/C %d/%d  Hit/Self %d/%d  Bnc %d  Stress C/H %.0f/%.0f  Loose H/S/T/P %d/%.0f/%d/%d"),
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

		DrawText(LineOne, PlayerColor, X, RowY, nullptr, 0.76f * TextScale);
		DrawText(LineTwo, FColor::White, X + 18.0f, RowY + LineHeight, nullptr, 0.70f * TextScale);
		DrawText(LineThree, FColor::Cyan, X + 18.0f, RowY + (LineHeight * 2.0f), nullptr, 0.66f * TextScale);
		DrawText(LineFour, FColor::Magenta, X + 18.0f, RowY + (LineHeight * 3.0f), nullptr, 0.66f * TextScale);
		RowY += PlayerBlockHeight;
	}
}

void AWizardStaffHUD::DrawHudMessageFeed(int32 MaxVisibleMessages)
{
	if (!Canvas)
	{
		return;
	}

	PruneExpiredHudMessages();

	const int32 VisibleCount = FMath::Min(FMath::Max(MaxVisibleMessages, 0), GameplayMessageFeed.Num());
	if (VisibleCount <= 0)
	{
		return;
	}

	const float LineHeight = 21.0f * TextScale;
	const float BoxWidth = FMath::Min(MessageFeedWidth, Canvas->SizeX - (PanelX * 2.0f));
	const float BoxHeight = 12.0f + (static_cast<float>(VisibleCount) * LineHeight);
	const float X = PanelX;
	const float Y = FMath::Max(PanelY, Canvas->SizeY - BoxHeight - MessageFeedBottomPadding);
	const int32 FirstMessageIndex = GameplayMessageFeed.Num() - VisibleCount;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.52f), X - 8.0f, Y - 6.0f, BoxWidth, BoxHeight);
	for (int32 VisibleIndex = 0; VisibleIndex < VisibleCount; ++VisibleIndex)
	{
		const FWizardHudFeedMessage& Message = GameplayMessageFeed[FirstMessageIndex + VisibleIndex];
		FString DisplayText = Message.Text;
		if (Message.RepeatCount > 1)
		{
			DisplayText += FString::Printf(TEXT(" x%d"), Message.RepeatCount);
		}

		DrawText(DisplayText, Message.Color.ToFColor(true), X, Y + (static_cast<float>(VisibleIndex) * LineHeight), nullptr, 0.66f * TextScale);
	}
}

void AWizardStaffHUD::DrawReplicatedGameplayEventFeed(int32 MaxVisibleMessages)
{
	if (!Canvas)
	{
		return;
	}

	const AWizardStaffGameState* WizardGameState = GetHudGameState(GetWorld());
	if (!WizardGameState)
	{
		return;
	}

	TArray<const FWizardReplicatedGameplayEvent*> CurrentGenerationEvents;
	const int32 CurrentGeneration = WizardGameState->GetReplicatedMatchSessionGeneration();
	for (const FWizardReplicatedGameplayEvent& Event : WizardGameState->GetReplicatedGameplayEvents())
	{
		if (Event.MatchSessionGeneration == CurrentGeneration && !Event.DisplayText.IsEmpty())
		{
			CurrentGenerationEvents.Add(&Event);
		}
	}

	const int32 VisibleCount = FMath::Min(FMath::Max(MaxVisibleMessages, 0), CurrentGenerationEvents.Num());
	if (VisibleCount <= 0)
	{
		return;
	}

	const float LineHeight = 21.0f * TextScale;
	const float BoxWidth = FMath::Min(MessageFeedWidth, Canvas->SizeX - (PanelX * 2.0f));
	const float BoxHeight = 12.0f + (static_cast<float>(VisibleCount) * LineHeight);
	const float X = PanelX;
	const float Y = FMath::Max(PanelY, Canvas->SizeY - BoxHeight - MessageFeedBottomPadding);
	const int32 FirstMessageIndex = CurrentGenerationEvents.Num() - VisibleCount;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.52f), X - 8.0f, Y - 6.0f, BoxWidth, BoxHeight);
	for (int32 VisibleIndex = 0; VisibleIndex < VisibleCount; ++VisibleIndex)
	{
		const FWizardReplicatedGameplayEvent* Event = CurrentGenerationEvents[FirstMessageIndex + VisibleIndex];
		if (!Event)
		{
			continue;
		}

		DrawText(
			Event->DisplayText,
			GetReplicatedGameplayEventHudColor(Event->EventType),
			X,
			Y + (static_cast<float>(VisibleIndex) * LineHeight),
			nullptr,
			0.66f * TextScale);
	}
}

void AWizardStaffHUD::PruneExpiredHudMessages()
{
	if (GameplayMessageFeed.Num() <= 0)
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (int32 MessageIndex = GameplayMessageFeed.Num() - 1; MessageIndex >= 0; --MessageIndex)
	{
		if (GameplayMessageFeed[MessageIndex].ExpireTime <= Now)
		{
			GameplayMessageFeed.RemoveAt(MessageIndex);
		}
	}
}

TArray<const AWizardStaffWizardCharacter*> AWizardStaffHUD::GetVisibleWizards() const
{
	TArray<const AWizardStaffWizardCharacter*> Wizards;
	const UWorld* World = GetWorld();
	if (!World)
	{
		return Wizards;
	}

	if (World->GetNetMode() == NM_Client)
	{
		for (TActorIterator<AWizardStaffWizardCharacter> It(World); It; ++It)
		{
			const AWizardStaffWizardCharacter* Wizard = *It;
			if (IsValid(Wizard) && Wizard->GetPlayerState<AWizardStaffPlayerState>())
			{
				Wizards.Add(Wizard);
			}
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

			const AWizardStaffWizardCharacter* Wizard = Cast<AWizardStaffWizardCharacter>(PlayerController->GetPawn());
			if (Wizard)
			{
				Wizards.Add(Wizard);
			}
		}
	}

	Wizards.Sort([](const AWizardStaffWizardCharacter& Left, const AWizardStaffWizardCharacter& Right)
	{
		return GetHudPlayerIndex(nullptr, &Left) < GetHudPlayerIndex(nullptr, &Right);
	});

	return Wizards;
}

FVector2D AWizardStaffHUD::GetSidePanelOrigin(int32 PlayerCount, float BoxWidth) const
{
	float X = PanelX + RowWidth + 28.0f;
	float Y = PanelY + 42.0f;
	if (Canvas && X + BoxWidth + PanelX > Canvas->SizeX)
	{
		X = PanelX;
		Y = PanelY + 42.0f + (static_cast<float>(PlayerCount) * (RowHeight + 8.0f)) + 18.0f;
	}

	return FVector2D(X, Y);
}

FString AWizardStaffHUD::GetTimerText(const AWizardStaffGameMode* GameMode) const
{
	if (!GameMode)
	{
		const AWizardStaffGameState* WizardGameState = GetHudGameState(GetWorld());
		if (!WizardGameState)
		{
			return TEXT("Time 0s");
		}

		const EWizardPartyMatchState PartyState = WizardGameState->GetReplicatedPartyMatchState();
		const EWizardTrialState TrialState = WizardGameState->GetReplicatedActiveTrialState();
		if (PartyState == EWizardPartyMatchState::FinalRound)
		{
			return WizardGameState->GetReplicatedResultMessage().IsEmpty()
				? FString::Printf(TEXT("Final %.0fs"), WizardGameState->GetReplicatedFinalRoundRemainingTime())
				: FString::Printf(TEXT("Rematch %.0fs"), WizardGameState->GetReplicatedTrialResultsRemainingTime());
		}

		if ((PartyState == EWizardPartyMatchState::PartyHall || PartyState == EWizardPartyMatchState::Intermission)
			&& TrialState == EWizardTrialState::WaitingToStart)
		{
			return FString::Printf(TEXT("Intermission %.0fs"), WizardGameState->GetReplicatedIntermissionRemainingTime());
		}

		if (TrialState == EWizardTrialState::Countdown)
		{
			return FString::Printf(TEXT("Countdown %.0fs"), WizardGameState->GetReplicatedTrialCountdownRemainingTime());
		}

		if (TrialState == EWizardTrialState::Results)
		{
			return FString::Printf(TEXT("Results %.0fs"), WizardGameState->GetReplicatedTrialResultsRemainingTime());
		}

		return FString::Printf(TEXT("Time %.0fs"), WizardGameState->GetActiveReplicatedTimer());
	}

	const EWizardPartyMatchState PartyState = GameMode->GetPartyMatchState();
	const EWizardTrialState TrialState = GameMode->GetActiveTrialState();
	if (PartyState == EWizardPartyMatchState::FinalRound)
	{
		return GameMode->GetGrandWizardWinnerMessage().IsEmpty()
			? FString::Printf(TEXT("Final %.0fs"), GameMode->GetFinalRoundRemainingTime())
			: FString::Printf(TEXT("Rematch %.0fs"), GameMode->GetTrialResultsRemainingTime());
	}

	if ((PartyState == EWizardPartyMatchState::PartyHall || PartyState == EWizardPartyMatchState::Intermission)
		&& TrialState == EWizardTrialState::WaitingToStart)
	{
		return FString::Printf(TEXT("Intermission %.0fs"), GameMode->GetIntermissionRemainingTime());
	}

	if (TrialState == EWizardTrialState::Countdown)
	{
		return FString::Printf(TEXT("Countdown %.0fs"), GameMode->GetTrialCountdownRemainingTime());
	}

	if (TrialState == EWizardTrialState::Results)
	{
		return FString::Printf(TEXT("Results %.0fs"), GameMode->GetTrialResultsRemainingTime());
	}

	const float RemainingTime = GameMode->GetActiveTrialType() == EWizardTrialType::StaffsAtDawn
		? GameMode->GetStaffsAtDawnRemainingTime()
		: (GameMode->GetActiveTrialType() == EWizardTrialType::CauldronCatastrophe ? GameMode->GetCauldronRemainingTime() : GameMode->GetMugRunRemainingTime());
	return FString::Printf(TEXT("Time %.0fs"), RemainingTime);
}
