#include "WizardStaffMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WizardStaffGameInstance.h"

namespace
{
UTextBlock* MakeMenuText(UWidgetTree* WidgetTree, const FText& Text, float Scale, const FLinearColor& Color)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetRenderScale(FVector2D(Scale, Scale));
	return TextBlock;
}

UButton* AddMenuButton(UWidgetTree* WidgetTree, UVerticalBox* Parent, const FText& Label)
{
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetMinDesiredHeight(48.0f);
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetBackgroundColor(FLinearColor(0.12f, 0.18f, 0.28f, 1.0f));
	Button->SetContent(MakeMenuText(WidgetTree, Label, 1.05f, FLinearColor::White));
	SizeBox->SetContent(Button);
	UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(SizeBox);
	Slot->SetPadding(FMargin(0.0f, 5.0f));
	Slot->SetHorizontalAlignment(HAlign_Fill);
	return Button;
}

void AddMenuText(UWidgetTree* WidgetTree, UVerticalBox* Parent, const FText& Text, float Scale, const FLinearColor& Color, const FMargin& Padding)
{
	UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(MakeMenuText(WidgetTree, Text, Scale, Color));
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(HAlign_Fill);
}
}

void UWizardStaffMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	ShowControlsPanel(false);
	RefreshMenuState();
	FocusPrimaryAction();
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff main menu widget initialized."));
}

TSharedRef<SWidget> UWizardStaffMainMenuWidget::RebuildWidget()
{
	BuildWidgetTree();
	return Super::RebuildWidget();
}

void UWizardStaffMainMenuWidget::NativeDestruct()
{
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff main menu widget destroyed."));
	Super::NativeDestruct();
}

void UWizardStaffMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshMenuState();
}

FReply UWizardStaffMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (bShowingControls && (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right))
	{
		ShowControlsPanel(false);
		return FReply::Handled();
	}

	if (!bShowingControls && (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right))
	{
		if (const UWizardStaffGameInstance* GameInstance = GetGameInstance<UWizardStaffGameInstance>())
		{
			const EWizardStaffFrontendState State = GameInstance->GetFrontendState();
			if (State == EWizardStaffFrontendState::SearchingSessions || State == EWizardStaffFrontendState::JoiningSession)
			{
				HandleCancelClicked();
				return FReply::Handled();
			}
		}
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

void UWizardStaffMainMenuWidget::BuildWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MainMenuRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Background->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.05f, 1.0f));
	UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(Background);
	BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
	BackgroundSlot->SetOffsets(FMargin(0.0f));

	MainMenuPanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	MainMenuPanelBorder->SetBrushColor(FLinearColor(0.06f, 0.08f, 0.14f, 0.96f));
	MainMenuPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	MainMenuPanelBorder->SetContent(MainMenuPanel);
	UCanvasPanelSlot* MainPanelSlot = RootCanvas->AddChildToCanvas(MainMenuPanelBorder);
	MainPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	MainPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	MainPanelSlot->SetSize(FVector2D(440.0f, 470.0f));

	AddMenuText(WidgetTree, MainMenuPanel, FText::FromString(TEXT("WIZARD STAFF")), 1.70f, FLinearColor(0.95f, 0.82f, 0.27f, 1.0f), FMargin(18.0f, 30.0f, 18.0f, 10.0f));
	AddMenuText(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Private Playtest")), 0.82f, FLinearColor(0.55f, 0.85f, 1.0f, 1.0f), FMargin(18.0f, 0.0f, 18.0f, 22.0f));

	PlayLocalButton = AddMenuButton(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Play Local")));
	HostOnlineButton = AddMenuButton(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Host Online Game")));
	JoinOnlineButton = AddMenuButton(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Join Online Game")));
	ControlsButton = AddMenuButton(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Controls")));
	CancelButton = AddMenuButton(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Cancel Search")));
	QuitButton = AddMenuButton(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Quit Game")));
	CancelButton->SetVisibility(ESlateVisibility::Collapsed);
	PrimaryActionButtons = { PlayLocalButton, HostOnlineButton, JoinOnlineButton, ControlsButton, QuitButton };

	StatusText = MakeMenuText(WidgetTree, FText::GetEmpty(), 0.82f, FLinearColor(0.70f, 0.90f, 1.0f, 1.0f));
	UVerticalBoxSlot* StatusSlot = MainMenuPanel->AddChildToVerticalBox(StatusText);
	StatusSlot->SetPadding(FMargin(18.0f, 14.0f, 18.0f, 18.0f));
	StatusSlot->SetHorizontalAlignment(HAlign_Fill);
	AddMenuText(WidgetTree, MainMenuPanel, FText::FromString(TEXT("Build: Private Steam Playtest")), 0.62f, FLinearColor(0.52f, 0.58f, 0.68f, 1.0f), FMargin(18.0f, 0.0f, 18.0f, 12.0f));

	PlayLocalButton->OnClicked.AddDynamic(this, &UWizardStaffMainMenuWidget::HandlePlayLocalClicked);
	HostOnlineButton->OnClicked.AddDynamic(this, &UWizardStaffMainMenuWidget::HandleHostOnlineClicked);
	JoinOnlineButton->OnClicked.AddDynamic(this, &UWizardStaffMainMenuWidget::HandleJoinOnlineClicked);
	ControlsButton->OnClicked.AddDynamic(this, &UWizardStaffMainMenuWidget::HandleControlsClicked);
	CancelButton->OnClicked.AddDynamic(this, &UWizardStaffMainMenuWidget::HandleCancelClicked);
	QuitButton->OnClicked.AddDynamic(this, &UWizardStaffMainMenuWidget::HandleQuitClicked);

	ControlsPanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	ControlsPanelBorder->SetBrushColor(FLinearColor(0.06f, 0.08f, 0.14f, 0.96f));
	ControlsPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	ControlsPanelBorder->SetContent(ControlsPanel);
	UCanvasPanelSlot* ControlsPanelSlot = RootCanvas->AddChildToCanvas(ControlsPanelBorder);
	ControlsPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	ControlsPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	ControlsPanelSlot->SetSize(FVector2D(560.0f, 500.0f));

	AddMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("CONTROLS")), 1.38f, FLinearColor(0.95f, 0.82f, 0.27f, 1.0f), FMargin(18.0f, 26.0f, 18.0f, 18.0f));
	AddMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Move: WASD / Arrow Keys (screen-relative) / Left Stick")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Aim staff: Mouse X or Q / E / Right Stick")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Jump + Broom Boost: Space or Bottom Face Button")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Quick Bonk: Left Mouse / F / Right Shoulder")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Use Brew Reward: Right Mouse / Left Shoulder")), 0.84f, FLinearColor::White, FMargin(18.0f, 4.0f));
	AddMenuText(WidgetTree, ControlsPanel, FText::FromString(TEXT("Party Hall: Bonk the Ready Bell when you are ready.")), 0.78f, FLinearColor(0.55f, 0.85f, 1.0f, 1.0f), FMargin(18.0f, 18.0f, 18.0f, 16.0f));
	UButton* BackButton = AddMenuButton(WidgetTree, ControlsPanel, FText::FromString(TEXT("Back")));
	BackButton->OnClicked.AddDynamic(this, &UWizardStaffMainMenuWidget::HandleBackClicked);
}

void UWizardStaffMainMenuWidget::ShowControlsPanel(bool bShowControls)
{
	bShowingControls = bShowControls;
	if (MainMenuPanelBorder)
	{
		MainMenuPanelBorder->SetVisibility(bShowControls ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (ControlsPanelBorder)
	{
		ControlsPanelBorder->SetVisibility(bShowControls ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (!bShowControls)
	{
		FocusPrimaryAction();
	}
}

void UWizardStaffMainMenuWidget::RefreshMenuState()
{
	const UWizardStaffGameInstance* GameInstance = GetGameInstance<UWizardStaffGameInstance>();
	if (!GameInstance || !StatusText)
	{
		return;
	}

	const int32 NewState = static_cast<int32>(GameInstance->GetFrontendState());
	const FText NewStatusText = GameInstance->GetFrontendStatusText();
	if (NewState == LastFrontendState && NewStatusText.EqualTo(LastStatusText))
	{
		return;
	}

	LastFrontendState = NewState;
	LastStatusText = NewStatusText;
	StatusText->SetText(NewStatusText);
	SetActionButtonsEnabled(!GameInstance->IsFrontendRequestBusy());
	const EWizardStaffFrontendState State = GameInstance->GetFrontendState();
	if (CancelButton)
	{
		CancelButton->SetVisibility((State == EWizardStaffFrontendState::SearchingSessions || State == EWizardStaffFrontendState::JoiningSession)
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	}
}

void UWizardStaffMainMenuWidget::SetActionButtonsEnabled(bool bEnabled)
{
	if (PlayLocalButton)
	{
		PlayLocalButton->SetIsEnabled(bEnabled);
	}
	if (HostOnlineButton)
	{
		HostOnlineButton->SetIsEnabled(bEnabled);
	}
	if (JoinOnlineButton)
	{
		JoinOnlineButton->SetIsEnabled(bEnabled);
	}
}

void UWizardStaffMainMenuWidget::FocusPrimaryAction()
{
	SetSelectedActionIndex(0);
	SetKeyboardFocus();
}

void UWizardStaffMainMenuWidget::SetSelectedActionIndex(int32 NewIndex)
{
	if (PrimaryActionButtons.IsEmpty())
	{
		return;
	}

	SelectedActionIndex = FMath::Clamp(NewIndex, 0, PrimaryActionButtons.Num() - 1);
	for (int32 Index = 0; Index < PrimaryActionButtons.Num(); ++Index)
	{
		if (UButton* Button = PrimaryActionButtons[Index])
		{
			Button->SetBackgroundColor(Index == SelectedActionIndex
				? FLinearColor(0.22f, 0.39f, 0.60f, 1.0f)
				: FLinearColor(0.12f, 0.18f, 0.28f, 1.0f));
		}
	}
}

void UWizardStaffMainMenuWidget::MoveSelection(int32 Direction)
{
	if (PrimaryActionButtons.IsEmpty())
	{
		return;
	}

	const int32 Count = PrimaryActionButtons.Num();
	SetSelectedActionIndex((SelectedActionIndex + Direction + Count) % Count);
}

void UWizardStaffMainMenuWidget::ActivateSelectedAction()
{
	if (!PrimaryActionButtons.IsValidIndex(SelectedActionIndex))
	{
		return;
	}

	if (UButton* Button = PrimaryActionButtons[SelectedActionIndex])
	{
		if (Button->GetIsEnabled())
		{
			Button->OnClicked.Broadcast();
		}
	}
}

void UWizardStaffMainMenuWidget::HandlePlayLocalClicked()
{
	if (UWizardStaffGameInstance* GameInstance = GetGameInstance<UWizardStaffGameInstance>())
	{
		GameInstance->StartLocalPrototypeFromMenu();
	}
}

void UWizardStaffMainMenuWidget::HandleHostOnlineClicked()
{
	if (UWizardStaffGameInstance* GameInstance = GetGameInstance<UWizardStaffGameInstance>())
	{
		GameInstance->StartSteamHostFromMenu();
	}
}

void UWizardStaffMainMenuWidget::HandleJoinOnlineClicked()
{
	if (UWizardStaffGameInstance* GameInstance = GetGameInstance<UWizardStaffGameInstance>())
	{
		GameInstance->StartSteamJoinFromMenu();
	}
}

void UWizardStaffMainMenuWidget::HandleControlsClicked()
{
	ShowControlsPanel(true);
}

void UWizardStaffMainMenuWidget::HandleBackClicked()
{
	ShowControlsPanel(false);
}

void UWizardStaffMainMenuWidget::HandleCancelClicked()
{
	if (UWizardStaffGameInstance* GameInstance = GetGameInstance<UWizardStaffGameInstance>())
	{
		GameInstance->CancelFrontendRequest();
	}
}

void UWizardStaffMainMenuWidget::HandleQuitClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff main menu quit requested."));
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
