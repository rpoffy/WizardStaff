#include "WizardStaffInGameMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "InputCoreTypes.h"
#include "WizardStaffPlayerController.h"

namespace
{
UTextBlock* MakeInGameMenuText(UWidgetTree* WidgetTree, const FText& Text, float Scale, const FLinearColor& Color)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetRenderScale(FVector2D(Scale, Scale));
	return TextBlock;
}

void AddInGameMenuText(UWidgetTree* WidgetTree, UVerticalBox* Parent, const FText& Text, float Scale, const FLinearColor& Color, const FMargin& Padding)
{
	UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(MakeInGameMenuText(WidgetTree, Text, Scale, Color));
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(HAlign_Fill);
}

UButton* AddInGameMenuButton(UWidgetTree* WidgetTree, UVerticalBox* Parent, const FText& Label)
{
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetMinDesiredHeight(48.0f);
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetBackgroundColor(FLinearColor(0.12f, 0.18f, 0.28f, 1.0f));
	Button->SetContent(MakeInGameMenuText(WidgetTree, Label, 1.0f, FLinearColor::White));
	SizeBox->SetContent(Button);
	UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(SizeBox);
	Slot->SetPadding(FMargin(0.0f, 5.0f));
	Slot->SetHorizontalAlignment(HAlign_Fill);
	return Button;
}
}

TSharedRef<SWidget> UWizardStaffInGameMenuWidget::RebuildWidget()
{
	BuildWidgetTree();
	return Super::RebuildWidget();
}

void UWizardStaffInGameMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	ShowControlsPanel(false);
	SetSelectedActionIndex(0);
	SetKeyboardFocus();
}

FReply UWizardStaffInGameMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right)
	{
		if (bShowingControls)
		{
			ShowControlsPanel(false);
		}
		else
		{
			HandleResumeClicked();
		}
		return FReply::Handled();
	}

	if (!bShowingControls)
	{
		if (Key == EKeys::Up || Key == EKeys::W || Key == EKeys::Gamepad_DPad_Up)
		{
			MoveSelection(-1);
			return FReply::Handled();
		}
		if (Key == EKeys::Down || Key == EKeys::S || Key == EKeys::Gamepad_DPad_Down)
		{
			MoveSelection(1);
			return FReply::Handled();
		}
		if (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
		{
			ActivateSelectedAction();
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UWizardStaffInGameMenuWidget::BuildWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InGameMenuRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Backdrop->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.58f));
	UCanvasPanelSlot* BackdropSlot = RootCanvas->AddChildToCanvas(Backdrop);
	BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	BackdropSlot->SetOffsets(FMargin(0.0f));

	MenuPanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	MenuPanelBorder->SetBrushColor(FLinearColor(0.06f, 0.08f, 0.14f, 0.98f));
	UVerticalBox* MenuPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	MenuPanelBorder->SetContent(MenuPanel);
	UCanvasPanelSlot* MenuPanelSlot = RootCanvas->AddChildToCanvas(MenuPanelBorder);
	MenuPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	MenuPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	MenuPanelSlot->SetSize(FVector2D(420.0f, 330.0f));

	AddInGameMenuText(WidgetTree, MenuPanel, FText::FromString(TEXT("GAME MENU")), 1.45f, FLinearColor(0.95f, 0.82f, 0.27f, 1.0f), FMargin(18.0f, 30.0f, 18.0f, 24.0f));
	UButton* ResumeButton = AddInGameMenuButton(WidgetTree, MenuPanel, FText::FromString(TEXT("Resume")));
	UButton* ControlsButton = AddInGameMenuButton(WidgetTree, MenuPanel, FText::FromString(TEXT("Controls")));
	UButton* ReturnButton = AddInGameMenuButton(WidgetTree, MenuPanel, FText::FromString(TEXT("Return to Main Menu")));
	ActionButtons = { ResumeButton, ControlsButton, ReturnButton };

	ResumeButton->OnClicked.AddDynamic(this, &UWizardStaffInGameMenuWidget::HandleResumeClicked);
	ControlsButton->OnClicked.AddDynamic(this, &UWizardStaffInGameMenuWidget::HandleControlsClicked);
	ReturnButton->OnClicked.AddDynamic(this, &UWizardStaffInGameMenuWidget::HandleReturnToMenuClicked);

	ControlsPanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	ControlsPanelBorder->SetBrushColor(FLinearColor(0.06f, 0.08f, 0.14f, 0.98f));
	UVerticalBox* ControlsPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	ControlsPanelBorder->SetContent(ControlsPanel);
	UCanvasPanelSlot* ControlsPanelSlot = RootCanvas->AddChildToCanvas(ControlsPanelBorder);
	ControlsPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	ControlsPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ControlsPanelSlot->SetSize(FVector2D(560.0f, 500.0f));

	AddInGameMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("CONTROLS")), 1.38f, FLinearColor(0.95f, 0.82f, 0.27f, 1.0f), FMargin(18.0f, 26.0f, 18.0f, 18.0f));
	AddInGameMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Move: WASD / Arrow Keys (screen-relative) / Left Stick")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddInGameMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Aim staff: Mouse X or Q / E / Right Stick")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddInGameMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Jump + Broom Boost: Space or Bottom Face Button")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddInGameMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Quick Bonk: Left Mouse / F / Right Shoulder")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddInGameMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Use Brew Reward: Right Mouse / Left Shoulder")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddInGameMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Party Hall: Bonk the Ready Bell when you are ready.")), 0.78f, FLinearColor(0.55f, 0.85f, 1.0f, 1.0f), FMargin(18.0f, 18.0f, 18.0f, 16.0f));
	UButton* BackButton = AddInGameMenuButton(WidgetTree, ControlsPanel, FText::FromString(TEXT("Back")));
	BackButton->OnClicked.AddDynamic(this, &UWizardStaffInGameMenuWidget::HandleControlsBackClicked);
}

void UWizardStaffInGameMenuWidget::ShowControlsPanel(bool bShowControls)
{
	bShowingControls = bShowControls;
	if (MenuPanelBorder)
	{
		MenuPanelBorder->SetVisibility(bShowControls ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (ControlsPanelBorder)
	{
		ControlsPanelBorder->SetVisibility(bShowControls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (!bShowControls)
	{
		SetSelectedActionIndex(0);
		SetKeyboardFocus();
	}
}

void UWizardStaffInGameMenuWidget::SetSelectedActionIndex(int32 NewIndex)
{
	if (ActionButtons.IsEmpty())
	{
		return;
	}

	SelectedActionIndex = FMath::Clamp(NewIndex, 0, ActionButtons.Num() - 1);
	for (int32 Index = 0; Index < ActionButtons.Num(); ++Index)
	{
		if (UButton* Button = ActionButtons[Index])
		{
			Button->SetBackgroundColor(Index == SelectedActionIndex
				? FLinearColor(0.22f, 0.39f, 0.60f, 1.0f)
				: FLinearColor(0.12f, 0.18f, 0.28f, 1.0f));
		}
	}
}

void UWizardStaffInGameMenuWidget::MoveSelection(int32 Direction)
{
	if (!ActionButtons.IsEmpty())
	{
		SetSelectedActionIndex((SelectedActionIndex + Direction + ActionButtons.Num()) % ActionButtons.Num());
	}
}

void UWizardStaffInGameMenuWidget::ActivateSelectedAction()
{
	if (ActionButtons.IsValidIndex(SelectedActionIndex) && ActionButtons[SelectedActionIndex])
	{
		ActionButtons[SelectedActionIndex]->OnClicked.Broadcast();
	}
}

void UWizardStaffInGameMenuWidget::HandleResumeClicked()
{
	if (AWizardStaffPlayerController* WizardController = Cast<AWizardStaffPlayerController>(GetOwningPlayer()))
	{
		WizardController->CloseInGameMenu();
	}
}

void UWizardStaffInGameMenuWidget::HandleControlsClicked()
{
	ShowControlsPanel(true);
}

void UWizardStaffInGameMenuWidget::HandleControlsBackClicked()
{
	ShowControlsPanel(false);
}

void UWizardStaffInGameMenuWidget::HandleReturnToMenuClicked()
{
	if (AWizardStaffPlayerController* WizardController = Cast<AWizardStaffPlayerController>(GetOwningPlayer()))
	{
		WizardController->RequestReturnToMainMenu();
	}
}
