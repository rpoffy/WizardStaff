#pragma once

#include "CoreMinimal.h"
#include "WizardStaffStaffsAtDawnPowerupTypes.generated.h"

UENUM(BlueprintType)
enum class EWizardStaffsAtDawnPowerupType : uint8
{
	None UMETA(DisplayName = "None"),
	MegaStaffBrew UMETA(DisplayName = "Mega Staff Brew")
};
