#include "PBBossPatternTelegraph.h"

#include "Components/SceneComponent.h"

APBBossPatternTelegraph::APBBossPatternTelegraph()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);
}

void APBBossPatternTelegraph::InitTelegraph(float InDurationSeconds, const FVector& InScale)
{
	static_cast<void>(InDurationSeconds);

	SetActorScale3D(InScale);
}

void APBBossPatternTelegraph::DestroyTelegraph()
{
	Destroy();
}
