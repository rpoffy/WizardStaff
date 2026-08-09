#include "WizardStaffMainMenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "WizardStaffMainMenuWidget.h"

void AWizardStaffMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController() || MainMenuWidget)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UWizardStaffMainMenuWidget>(this, UWizardStaffMainMenuWidget::StaticClass());
	if (!MainMenuWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wizard Staff main menu could not create its frontend widget."));
		return;
	}

	MainMenuWidget->AddToViewport(100);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
	SetInputMode(InputMode);
}

void AWizardStaffMainMenuPlayerController::PrepareForGameplayTravel()
{
	if (!IsLocalController())
	{
		return;
	}

	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}
