#include "PBCircularBladeSkill.h"

#include "PBCircularBladeActor.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "Engine/World.h"

APBCircularBladeActor* UPBCircularBladeSkill::Activate(
	APBBallBase* OwnerBall,
	TSubclassOf<APBCircularBladeActor> InBladeActorClass)
{
	if (!OwnerBall)
	{
		return nullptr;
	}

	UWorld* World = OwnerBall->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TSubclassOf<APBCircularBladeActor> ActorClass = InBladeActorClass
		? InBladeActorClass
		: BladeActorClass;
	if (!ActorClass)
	{
		ActorClass = APBCircularBladeActor::StaticClass();
	}

	const int32 BallAttackPower = GetBallAttackPower(OwnerBall);
	const int32 FinalDamage = CalculateFinalDamage(BallAttackPower);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerBall;
	SpawnParameters.Instigator = OwnerBall->GetInstigator();

	APBCircularBladeActor* BladeActor = World->SpawnActor<APBCircularBladeActor>(
		ActorClass,
		OwnerBall->GetActorLocation(),
		OwnerBall->GetActorRotation(),
		SpawnParameters);

	if (!BladeActor)
	{
		return nullptr;
	}

	BladeActor->InitializeBlade(
		OwnerBall,
		FinalDamage,
		Duration,
		DamageCount,
		AttackRadius);

	return BladeActor;
}

int32 UPBCircularBladeSkill::GetBallAttackPower_Implementation(const APBBallBase* OwnerBall) const
{
	const UPBBaseStatComponent* StatComponent = OwnerBall
		? OwnerBall->FindComponentByClass<UPBBaseStatComponent>()
		: nullptr;

	return StatComponent ? StatComponent->GetStat(PBStatNames::Attack) : 0;
}

int32 UPBCircularBladeSkill::CalculateFinalDamage_Implementation(int32 BallAttackPower) const
{
	return FMath::Max(0, FMath::RoundToInt(static_cast<float>(BallAttackPower) * DamageMultiplier));
}
