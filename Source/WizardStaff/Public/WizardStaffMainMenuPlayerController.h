#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WizardStaffMainMenuPlayerController.generated.h"

class UWizardStaffMainMenuWidget;

UCLASS()
class WIZARDSTAFF_API AWizardStaffMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// The frontend uses UI-only focus. Restore normal controller input before any map travel.
	void PrepareForGameplayTravel();

private:
	UPROPERTY(Transient)
	TObjectPtr<UWizardStaffMainMenuWidget> MainMenuWidget;
};
