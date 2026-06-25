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
	SetActorScale3D(InScale);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DestroyTimerHandle);

		if (InDurationSeconds <= 0.0f)
		{
			DestroyTelegraph();
			return;
		}

		World->GetTimerManager().SetTimer(
			DestroyTimerHandle,
			this,
			&APBBossPatternTelegraph::DestroyTelegraph,
			InDurationSeconds,
			false);
	}
}

void APBBossPatternTelegraph::DestroyTelegraph()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DestroyTimerHandle);
	}

	Destroy();
}
