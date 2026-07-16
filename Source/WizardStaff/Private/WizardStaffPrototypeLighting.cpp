#include "WizardStaffPrototypeLighting.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"

AWizardStaffPrototypeLighting::AWizardStaffPrototypeLighting()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetActorEnableCollision(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(SceneRoot);
	SunLight->SetMobility(EComponentMobility::Movable);
	SunLight->SetRelativeRotation(FRotator(-48.0f, -32.0f, 0.0f));
	SunLight->SetIntensity(5.0f);
	SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.86f));
	SunLight->bAtmosphereSunLight = true;
	SunLight->SetAtmosphereSunLightIndex(0);

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(SceneRoot);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(SceneRoot);
	SkyLight->SetMobility(EComponentMobility::Movable);
	SkyLight->SourceType = SLS_CapturedScene;
	SkyLight->SetIntensity(1.0f);
	SkyLight->SetRealTimeCapture(true);
}
