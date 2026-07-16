#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WizardStaffHUD.generated.h"

class AWizardStaffWizardCharacter;

UENUM(BlueprintType)
enum class EWizardHudDisplayMode : uint8
{
	FullDebug UMETA(DisplayName = "Full Debug"),
	Playtest UMETA(DisplayName = "Playtest"),
	Minimal UMETA(DisplayName = "Minimal"),
	Hidden UMETA(DisplayName = "Hidden")
};

UENUM(BlueprintType)
enum class EWizardHudMessageCategory : uint8
{
	Gameplay UMETA(DisplayName = "Gameplay"),
	Scoring UMETA(DisplayName = "Scoring"),
	Powerup UMETA(DisplayName = "Powerup"),
	Debug UMETA(DisplayName = "Debug")
};

USTRUCT(BlueprintType)
struct FWizardHudFeedMessage
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard Staff HUD")
	FString Text;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard Staff HUD")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard Staff HUD")
	float ExpireTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard Staff HUD")
	EWizardHudMessageCategory Category = EWizardHudMessageCategory::Gameplay;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wizard Staff HUD")
	int32 RepeatCount = 1;
};

UCLASS(Blueprintable)
class WIZARDSTAFF_API AWizardStaffHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Exec, Category = "Wizard Staff HUD")
	void CycleWizardHudMode();

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff HUD")
	void SetWizardHudMode(EWizardHudDisplayMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Wizard Staff HUD")
	EWizardHudDisplayMode GetWizardHudMode() const { return HudDisplayMode; }

	UFUNCTION(BlueprintPure, Category = "Wizard Staff HUD")
	FString GetWizardHudModeName() const;

	UFUNCTION(BlueprintCallable, Category = "Wizard Staff HUD")
	void AddHudMessage(const FString& Message, FLinearColor MessageColor, float Lifetime, EWizardHudMessageCategory Category);

	static void PushGameplayMessage(const UObject* WorldContextObject, const FString& Message, const FColor& MessageColor, float Lifetime, EWizardHudMessageCategory Category);
	static bool IsFullDebugMode(const UObject* WorldContextObject);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	EWizardHudDisplayMode DefaultHudMode = EWizardHudDisplayMode::Playtest;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff HUD")
	EWizardHudDisplayMode HudDisplayMode = EWizardHudDisplayMode::Playtest;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	float PanelX = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	float PanelY = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	float RowWidth = 540.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	float RowHeight = 98.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	float BarWidth = 170.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	float BarHeight = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	float TextScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	FLinearColor PanelColor = FLinearColor(0.02f, 0.02f, 0.02f, 0.62f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD|Message Feed", meta = (ClampMin = "0.1"))
	float GameplayMessageDefaultLifetime = 2.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD|Message Feed", meta = (ClampMin = "1", ClampMax = "8"))
	int32 PlaytestMessageCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD|Message Feed", meta = (ClampMin = "1", ClampMax = "10"))
	int32 FullDebugMessageCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD|Message Feed", meta = (ClampMin = "120.0"))
	float MessageFeedWidth = 520.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD|Message Feed", meta = (ClampMin = "0.0"))
	float MessageFeedBottomPadding = 34.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wizard Staff HUD|Message Feed", meta = (ClampMin = "1"))
	int32 MaxStoredMessages = 12;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wizard Staff HUD|Message Feed")
	TArray<FWizardHudFeedMessage> GameplayMessageFeed;

protected:
	bool ShouldDrawSharedHUD() const;
	void DrawFullDebugHUD(const class AWizardStaffGameMode* GameMode);
	void DrawPlaytestHUD(const class AWizardStaffGameMode* GameMode);
	void DrawMinimalHUD(const class AWizardStaffGameMode* GameMode);
	void DrawMatchHeader(float X, float Y);
	void DrawPlayerRows(float X, float Y);
	void DrawPlayerRow(const AWizardStaffWizardCharacter* Wizard, float X, float Y);
	void DrawCompactPlayerRows(const class AWizardStaffGameMode* GameMode, float X, float Y, bool bMinimalRows);
	void DrawCompactValueMeter(const FString& Label, float Current, float Max, const FLinearColor& FillColor, float X, float Y, float Width);
	void DrawPlaytestEventPanel(const class AWizardStaffGameMode* GameMode, float SuggestedY);
	void DrawPlaytestMatchSummary(const class AWizardStaffGameMode* GameMode, float X, float Y);
	void DrawMinimalStatePanel(const class AWizardStaffGameMode* GameMode, float X, float Y);
	void DrawValueBar(const FString& Label, float Current, float Max, const FLinearColor& FillColor, float X, float Y);
	void DrawScoringClarityPanel();
	void DrawPartyHallStandingsPanel(const class AWizardStaffGameMode* GameMode);
	void DrawStaffsAtDawnScorePanel(const class AWizardStaffGameMode* GameMode);
	void DrawCauldronScorePanel(const class AWizardStaffGameMode* GameMode);
	void DrawTrialResultsPanel(const class AWizardStaffGameMode* GameMode);
	void DrawFinalIntroPanel(const class AWizardStaffGameMode* GameMode);
	void DrawCompactFinalHud(const class AWizardStaffGameMode* GameMode);
	void DrawWinnerMessage();
	void DrawMatchSummary();
	void DrawHudMessageFeed(int32 MaxVisibleMessages);
	void DrawReplicatedGameplayEventFeed(int32 MaxVisibleMessages);
	void PruneExpiredHudMessages();
	TArray<const AWizardStaffWizardCharacter*> GetVisibleWizards() const;
	FVector2D GetSidePanelOrigin(int32 PlayerCount, float BoxWidth) const;
	FString GetTimerText(const class AWizardStaffGameMode* GameMode) const;
};
