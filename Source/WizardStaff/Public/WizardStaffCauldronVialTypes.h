#pragma once

#include "CoreMinimal.h"
#include "WizardStaffCauldronVialTypes.generated.h"

UENUM(BlueprintType)
enum class EWizardCauldronVialType : uint8
{
	None,
	Speed,
	BurdeningPower
};

inline FString GetWizardCauldronVialDisplayName(EWizardCauldronVialType VialType)
{
	switch (VialType)
	{
	case EWizardCauldronVialType::Speed:
		return TEXT("Speed");
	case EWizardCauldronVialType::BurdeningPower:
		return TEXT("Burdening Power");
	default:
		return TEXT("None");
	}
}

inline FLinearColor GetWizardCauldronVialColor(EWizardCauldronVialType VialType)
{
	return VialType == EWizardCauldronVialType::Speed
		? FLinearColor(0.08f, 0.68f, 1.0f)
		: (VialType == EWizardCauldronVialType::BurdeningPower
			? FLinearColor(1.0f, 0.22f, 0.04f)
			: FLinearColor::White);
}
