#include "WizardStaffPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "WizardStaffGameInstance.h"
#include "WizardStaffInGameMenuWidget.h"

void AWizardStaffPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InGameMenuWidget)
	{
		InGameMenuWidget->RemoveFromParent();
		InGameMenuWidget = nullptr;
	}
	bInGameMenuOpen = false;
	bReturningToMainMenu = false;

	Super::EndPlay(EndPlayReason);
}

void AWizardStaffPlayerController::ToggleInGameMenu()
{
	if (!IsLocalController() || bReturningToMainMenu)
	{
		return;
	}

	if (bInGameMenuOpen)
	{
		CloseInGameMenu();
	}
	else
	{
		OpenInGameMenu();
	}
}

void AWizardStaffPlayerController::OpenInGameMenu()
{
	if (!IsLocalController() || bInGameMenuOpen)
	{
		return;
	}

	InGameMenuWidget = CreateWidget<UWizardStaffInGameMenuWidget>(this, UWizardStaffInGameMenuWidget::StaticClass());
	if (!InGameMenuWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wizard Staff gameplay menu could not create its widget."));
		return;
	}

	InGameMenuWidget->AddToViewport(200);
	bInGameMenuOpen = true;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetWidgetToFocus(InGameMenuWidget->TakeWidget());
	SetInputMode(InputMode);
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff local gameplay menu opened."));
}

void AWizardStaffPlayerController::CloseInGameMenu()
{
	if (!IsLocalController() || !bInGameMenuOpen || bReturningToMainMenu)
	{
		return;
	}

	if (InGameMenuWidget)
	{
		InGameMenuWidget->RemoveFromParent();
		InGameMenuWidget = nullptr;
	}

	bInGameMenuOpen = false;
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff local gameplay menu closed."));
}

void AWizardStaffPlayerController::RequestReturnToMainMenu()
{
	if (!IsLocalController() || bReturningToMainMenu)
	{
		return;
	}

	bReturningToMainMenu = true;
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff gameplay menu requested return to main menu."));
	if (UWizardStaffGameInstance* WizardGameInstance = GetGameInstance<UWizardStaffGameInstance>())
	{
		WizardGameInstance->ReturnToMainMenu();
		return;
	}

	bReturningToMainMenu = false;
}
