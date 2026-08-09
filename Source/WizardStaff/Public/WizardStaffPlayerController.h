#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WizardStaffPlayerController.generated.h"

class UWizardStaffInGameMenuWidget;

UCLASS()
class WIZARDSTAFF_API AWizardStaffPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ToggleInGameMenu();
	void CloseInGameMenu();
	void RequestReturnToMainMenu();

private:
	void OpenInGameMenu();

	UPROPERTY(Transient)
	TObjectPtr<UWizardStaffInGameMenuWidget> InGameMenuWidget;

	bool bInGameMenuOpen = false;
	bool bReturningToMainMenu = false;
};
