#include "PBSummonBallSkillActor.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Summon/PBCloneBallActor.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "TimerManager.h"

APBSummonBallSkillActor::APBSummonBallSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APBSummonBallSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);

	SummonedBallDamage = FMath::Max(GetSkillDamageAmount(), 1);
	SummonedBallDuration = FMath::Max(InSkillData.LifeValue, 0.0f);
	SummonedBallCount = FMath::Max(InSkillData.EffectValue, 1);
}

void APBSummonBallSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearDurationTimer();
	Super::EndPlay(EndPlayReason);
}

void APBSummonBallSkillActor::EnterActiveState()
{
	if (!SpawnSummonedBalls())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();

	GetWorld()->GetTimerManager().SetTimer(
		DurationTimerHandle,
		this,
		&APBSummonBallSkillActor::HandleDurationFinished,
		SummonedBallDuration,
		false);
}

void APBSummonBallSkillActor::EnterStoppingState()
{
	ClearDurationTimer();
	DestroySummonedBalls();
	Super::EnterStoppingState();
}

bool APBSummonBallSkillActor::SpawnSummonedBalls()
{
	UWorld* World = GetWorld();
	if (!IsValid(World)
		|| !IsValid(OwnerBall)
		|| !SummonedBallClass
		|| SummonedBallDamage <= 0
		|| SummonedBallDuration <= 0.0f
		|| SummonedBallCount <= 0)
	{
		return false;
	}

	SummonedBalls.Reset();
	const FVector Center = OwnerBall->GetActorLocation();
	const UPBBallPhysicsComponent* OwnerPhysicsComponent =
		OwnerBall->FindComponentByClass<UPBBallPhysicsComponent>();
	const FVector OwnerVelocity = OwnerPhysicsComponent
		? OwnerPhysicsComponent->GetVelocity()
		: FVector::ZeroVector;

	for (int32 Index = 0; Index < SummonedBallCount; ++Index)
	{
		const float Angle = 2.0f * UE_PI * static_cast<float>(Index) / static_cast<float>(SummonedBallCount);
		const FVector Direction(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
		const FTransform SpawnTransform(OwnerBall->GetActorRotation(), Center + Direction * SpawnRadius);

		UE_LOG(LogTemp, Warning, TEXT("SPAWN %d"), SummonedBallCount);
		
		APBCloneBallActor* SummonedBall = World->SpawnActorDeferred<APBCloneBallActor>(
			SummonedBallClass,
			SpawnTransform,
			OwnerBall,
			OwnerBall->GetInstigator(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!IsValid(SummonedBall))
		{
			continue;
		}

		SummonedBall->InitializeFromSourceBall(OwnerBall);
		UGameplayStatics::FinishSpawningActor(SummonedBall, SpawnTransform);

		if (UPBBaseStatComponent* StatComponent =
			SummonedBall->FindComponentByClass<UPBBaseStatComponent>())
		{
			StatComponent->SetStat(PBStatNames::Attack, SummonedBallDamage);
		}

		SummonedBall->SetLifeSpan(SummonedBallDuration);

		if (UPBBallPhysicsComponent* PhysicsComponent =
			SummonedBall->FindComponentByClass<UPBBallPhysicsComponent>())
		{
			PhysicsComponent->SetVelocity(OwnerVelocity + Direction * OutwardVelocity);
		}

		SummonedBalls.Add(SummonedBall);
	}

	return !SummonedBalls.IsEmpty();
}

void APBSummonBallSkillActor::DestroySummonedBalls()
{
	for (const TWeakObjectPtr<APBCloneBallActor>& SummonedBall : SummonedBalls)
	{
		if (SummonedBall.IsValid())
		{
			SummonedBall->Destroy();
		}
	}

	SummonedBalls.Reset();
}

void APBSummonBallSkillActor::ClearDurationTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
	}
}

void APBSummonBallSkillActor::HandleDurationFinished()
{
	FinishSkill();
}
