#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardStaffPrototypeLighting.generated.h"

class UDirectionalLightComponent;
class USceneComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;

UCLASS()
class WIZARDSTAFF_API AWizardStaffPrototypeLighting : public AActor
{
	GENERATED_BODY()

public:
	AWizardStaffPrototypeLighting();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Lighting")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Lighting")
	TObjectPtr<UDirectionalLightComponent> SunLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Lighting")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype Lighting")
	TObjectPtr<USkyLightComponent> SkyLight;
};
