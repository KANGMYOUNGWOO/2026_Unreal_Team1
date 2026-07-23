#include "PBBossTurtleFallingRockPattern.h"

#include "Kismet/GameplayStatics.h"
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

	USoundBase* HitSFXToUse = IsValid(RockHitSFX) ? RockHitSFX.Get() : LoopingSFX.Get();

	for (int32 FallingRockIndex = 0; FallingRockIndex < FallingRockCount; ++FallingRockIndex)
	{
		const FVector SpawnLocation = TurtleBoss->GetRandomFallingRockLocation(SpawnHeight);
		const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
		APBTurtleFallingRock* FallingRock = World->SpawnActorDeferred<APBTurtleFallingRock>(
			FallingRockClass,
			SpawnTransform,
			TurtleBoss,
			TurtleBoss,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (FallingRock)
		{
			const FName SourcePatternName = PatternName.IsNone() ? GetClass()->GetFName() : PatternName;
			FallingRock->SetSourcePatternName(SourcePatternName);
			FallingRock->SetFallingSpeed(FallingSpeed);
			FallingRock->SetDamageAmount(DamageAmount);
			FallingRock->SetHitSFX(HitSFXToUse);
			UGameplayStatics::FinishSpawningActor(FallingRock, SpawnTransform);
		}
	}

	FinishPattern();
}
