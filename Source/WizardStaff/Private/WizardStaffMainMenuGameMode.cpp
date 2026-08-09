#include "WizardStaffMainMenuGameMode.h"

#include "WizardStaffMainMenuPlayerController.h"

AWizardStaffMainMenuGameMode::AWizardStaffMainMenuGameMode()
{
	PlayerControllerClass = AWizardStaffMainMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	bStartPlayersAsSpectators = true;
}

void AWizardStaffMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("Wizard Staff main menu initialized."));
}
