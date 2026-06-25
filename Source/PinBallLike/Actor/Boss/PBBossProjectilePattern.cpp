#include "PBBossProjectilePattern.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"

void UPBBossProjectilePattern::StartPattern_Implementation(APBBossBase* Boss)
{
	const FString PatternDisplayName = PatternName.IsNone()
		? GetClass()->GetName()
		: PatternName.ToString();

	UE_LOG(LogTemp, Warning, TEXT("Boss Projectile Pattern Activated: %s, Boss: %s"),
		*PatternDisplayName,
		*GetNameSafe(Boss));

	FinishPattern();
}
