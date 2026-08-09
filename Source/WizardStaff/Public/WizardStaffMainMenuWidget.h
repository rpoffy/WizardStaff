#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "WizardStaffMainMenuWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;
class UVerticalBox;

UCLASS()
class WIZARDSTAFF_API UWizardStaffMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void BuildWidgetTree();
	void ShowControlsPanel(bool bShowControls);
	void RefreshMenuState();
	void SetActionButtonsEnabled(bool bEnabled);
	void FocusPrimaryAction();
	void SetSelectedActionIndex(int32 NewIndex);
	void MoveSelection(int32 Direction);
	void ActivateSelectedAction();

	UFUNCTION()
	void HandlePlayLocalClicked();

	UFUNCTION()
	void HandleHostOnlineClicked();

	UFUNCTION()
	void HandleJoinOnlineClicked();

	UFUNCTION()
	void HandleControlsClicked();

	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleCancelClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> MainMenuPanel;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> MainMenuPanelBorder;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ControlsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ControlsPanelBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PlayLocalButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> HostOnlineButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> JoinOnlineButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ControlsButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> PrimaryActionButtons;

	bool bShowingControls = false;
	int32 SelectedActionIndex = 0;
	int32 LastFrontendState = INDEX_NONE;
	FText LastStatusText;
};
