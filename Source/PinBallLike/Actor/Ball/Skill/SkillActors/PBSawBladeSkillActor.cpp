#include "PBSawBladeSkillActor.h"

#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Summon/PBSawBladeActor.h"

APBSawBladeSkillActor::APBSawBladeSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APBSawBladeSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);

	EffectDamage = GetSkillDamageAmount();
	EffectGroggy = GetSkillGroggyAmount();
	EffectDuration = FMath::Max(InSkillData.LifeValue, 0.0f);
	BounceCount = FMath::Max(FMath::RoundToInt(InSkillData.LifeValue), 1);
	SummonCount = FMath::Max(InSkillData.EffectValue, 1);
}

void APBSawBladeSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroySummonActors();
	Super::EndPlay(EndPlayReason);
}

void APBSawBladeSkillActor::EnterActiveState()
{
	if (!SpawnSummonActors())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();
}

void APBSawBladeSkillActor::EnterStoppingState()
{
	DestroySummonActors();
	Super::EnterStoppingState();
}

bool APBSawBladeSkillActor::SpawnSummonActors()
{
	if (!IsValid(OwnerBall)
		|| !SummonActorClass
		|| EffectDamage <= 0
		|| EffectDuration <= 0.0f
		|| SummonCount <= 0)
	{
		return false;
	}

	SpawnedSummonActors.Reset();
	for (int32 Index = 0; Index < SummonCount; ++Index)
	{
		const float Angle = 360.0f * static_cast<float>(Index) / static_cast<float>(SummonCount);
		const FVector Direction = GetActorForwardVector().RotateAngleAxis(Angle, FVector::UpVector);
		const FTransform SpawnTransform(Direction.Rotation(), OwnerBall->GetActorLocation());
		APBSawBladeActor* SummonActor = GetWorld()->SpawnActorDeferred<APBSawBladeActor>(
			SummonActorClass,
			SpawnTransform,
			OwnerBall,
			OwnerBall->GetInstigator(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!SummonActor)
		{
			continue;
		}

		SummonActor->InitializeEffect(
			OwnerBall,
			EffectDamage,
			EffectGroggy,
			EffectDuration,
			BounceCount);
		SummonActor->OnEffectFinished.AddUniqueDynamic(
			this,
			&APBSawBladeSkillActor::HandleSummonFinished);
		UGameplayStatics::FinishSpawningActor(SummonActor, SpawnTransform);
		SummonActor->ActivateEffect(Direction);
		SpawnedSummonActors.Add(SummonActor);
	}

	return !SpawnedSummonActors.IsEmpty();
}

void APBSawBladeSkillActor::DestroySummonActors()
{
	for (const TWeakObjectPtr<APBSawBladeActor>& SummonActor : SpawnedSummonActors)
	{
		if (SummonActor.IsValid())
		{
			SummonActor->OnEffectFinished.RemoveAll(this);
			SummonActor->Destroy();
		}
	}
	SpawnedSummonActors.Reset();
}

void APBSawBladeSkillActor::HandleSummonFinished(AActor* SummonActor)
{
	SpawnedSummonActors.RemoveAll(
		[SummonActor](const TWeakObjectPtr<APBSawBladeActor>& SpawnedActor)
		{
			return !SpawnedActor.IsValid() || SpawnedActor.Get() == SummonActor;
		});

	if (SpawnedSummonActors.IsEmpty())
	{
		FinishSkill();
	}
}
