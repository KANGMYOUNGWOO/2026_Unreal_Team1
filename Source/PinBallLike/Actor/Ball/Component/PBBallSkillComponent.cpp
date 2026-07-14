#include "PBBallSkillComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UPBBallSkillComponent::UPBBallSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBallSkillComponent::TryActivateSkill()
{
	return ActivateSkillActor() != nullptr;
}

APBBallSkillActorBase* UPBBallSkillComponent::ActivateSkillActor()
{
	if (IsValid(ActiveSkillActor))
	{
		return ActiveSkillActor;
	}

	APBBallBase* OwnerBall = Cast<APBBallBase>(GetOwner());
	if (!OwnerBall || !OwnerBall->GetWorld() || !SkillData.ActorClass)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(
		OwnerBall->GetActorRotation(),
		OwnerBall->GetActorLocation());
	ActiveSkillActor = OwnerBall->GetWorld()->SpawnActorDeferred<APBBallSkillActorBase>(
		SkillData.ActorClass,
		SpawnTransform,
		OwnerBall,
		OwnerBall->GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (ActiveSkillActor)
	{
		ActiveSkillActor->InitializeSkill(
			OwnerBall,
			CalculateFinalDamage(OwnerBall),
			SkillData.Duration,
			SkillData.DamageCount);
		ActiveSkillActor->OnDestroyed.AddUniqueDynamic(
			this,
			&UPBBallSkillComponent::HandleActiveSkillActorDestroyed);
		UGameplayStatics::FinishSpawningActor(ActiveSkillActor, SpawnTransform);
	}

	return ActiveSkillActor;
}

int32 UPBBallSkillComponent::CalculateFinalDamage(
	const APBBallBase* OwnerBall) const
{
	const UPBBaseStatComponent* StatComponent = OwnerBall
		? OwnerBall->FindComponentByClass<UPBBaseStatComponent>()
		: nullptr;
	const int32 BallAttackPower = StatComponent
		? StatComponent->GetStat(PBStatNames::Attack)
		: 0;

	return FMath::Max(
		0,
		FMath::RoundToInt(static_cast<float>(BallAttackPower) * SkillData.DamageMultiplier));
}

void UPBBallSkillComponent::HandleActiveSkillActorDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == ActiveSkillActor)
	{
		ActiveSkillActor = nullptr;
	}
}
