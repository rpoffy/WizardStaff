#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WizardStaffMainMenuGameMode.generated.h"

UCLASS()
class WIZARDSTAFF_API AWizardStaffMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWizardStaffMainMenuGameMode();
	virtual void BeginPlay() override;
};
