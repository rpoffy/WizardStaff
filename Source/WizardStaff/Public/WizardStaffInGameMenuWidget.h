#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "WizardStaffInGameMenuWidget.generated.h"

class UBorder;
class UButton;
class UVerticalBox;

UCLASS()
class WIZARDSTAFF_API UWizardStaffInGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void BuildWidgetTree();
	void ShowControlsPanel(bool bShowControls);
	void SetSelectedActionIndex(int32 NewIndex);
	void MoveSelection(int32 Direction);
	void ActivateSelectedAction();

	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleControlsClicked();

	UFUNCTION()
	void HandleControlsBackClicked();

	UFUNCTION()
	void HandleReturnToMenuClicked();

	UPROPERTY(Transient)
	TObjectPtr<UBorder> MenuPanelBorder;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ControlsPanelBorder;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> ActionButtons;

	bool bShowingControls = false;
	int32 SelectedActionIndex = 0;
};
