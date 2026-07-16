#include "PBBossTurtleFallingRockPattern.h"

#include "PinBallLike/Actor/Boss/Turtle/PBTurtleBoss.h"
#include "PinBallLike/Actor/Boss/Turtle/PBTurtleFallingRock.h"

bool UPBBossTurtleFallingRockPattern::CanExecute_Implementation(APBBossBase* Boss) const
{
	return Super::CanExecute_Implementation(Boss)
		&& Cast<APBTurtleBoss>(Boss)
		&& FallingRockClass
		&& FallingRockCount > 0;
}

void UPBBossTurtleFallingRockPattern::ExecutePattern_Implementation(APBBossBase* Boss)
{
	APBTurtleBoss* TurtleBoss = Cast<APBTurtleBoss>(Boss);
	UWorld* World = TurtleBoss ? TurtleBoss->GetWorld() : nullptr;
	if (!World || !FallingRockClass || FallingRockCount <= 0)
	{
		FinishPattern();
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = TurtleBoss;
	SpawnParameters.Instigator = TurtleBoss;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 FallingRockIndex = 0; FallingRockIndex < FallingRockCount; ++FallingRockIndex)
	{
		const FVector SpawnLocation = TurtleBoss->GetRandomFallingRockLocation(SpawnHeight);
		APBTurtleFallingRock* FallingRock = World->SpawnActor<APBTurtleFallingRock>(
			FallingRockClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParameters);

		if (FallingRock)
		{
			const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
			FallingRock->SetSourcePatternName(SourcePatternName);
			FallingRock->SetFallingSpeed(FallingSpeed);
		}
	}

	FinishPattern();
}
