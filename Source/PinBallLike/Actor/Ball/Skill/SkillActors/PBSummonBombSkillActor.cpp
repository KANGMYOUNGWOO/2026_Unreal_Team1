#include "PBSummonBombSkillActor.h"

#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Summon/PBBombActor.h"

APBSummonBombSkillActor::APBSummonBombSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APBSummonBombSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);

	BombDamage = GetSkillDamageAmount();
	BombGroggy = GetSkillGroggyAmount();
	BombCount = FMath::Max(InSkillData.EffectValue, 1);
}

void APBSummonBombSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyBombs();
	Super::EndPlay(EndPlayReason);
}

void APBSummonBombSkillActor::EnterActiveState()
{
	if (!SpawnBombs())
	{
		StopSkill();
		return;
	}

	Super::EnterActiveState();
}

void APBSummonBombSkillActor::EnterStoppingState()
{
	DestroyBombs();
	Super::EnterStoppingState();
}

bool APBSummonBombSkillActor::SpawnBombs()
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(OwnerBall) || !BombClass || BombDamage <= 0)
	{
		return false;
	}

	SpawnedBombs.Reset();
	const FVector Center = OwnerBall->GetActorLocation();
	const UPBBallPhysicsComponent* OwnerPhysicsComponent =
		OwnerBall->FindComponentByClass<UPBBallPhysicsComponent>();
	const FVector OwnerVelocity = OwnerPhysicsComponent
		? OwnerPhysicsComponent->GetVelocity()
		: FVector::ZeroVector;

	for (int32 Index = 0; Index < BombCount; ++Index)
	{
		const float Angle =
			2.0f * UE_PI * static_cast<float>(Index) / static_cast<float>(BombCount);
		const FVector Direction(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
		const FTransform SpawnTransform(
			OwnerBall->GetActorRotation(),
			Center + Direction * SpawnRadius);

		APBBombActor* Bomb = World->SpawnActorDeferred<APBBombActor>(
			BombClass,
			SpawnTransform,
			OwnerBall,
			OwnerBall->GetInstigator(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Bomb)
		{
			continue;
		}

		Bomb->InitializeBomb(OwnerBall, BombDamage, BombGroggy);
		Bomb->OnBombFinished.AddUniqueDynamic(
			this,
			&APBSummonBombSkillActor::HandleBombFinished);
		UGameplayStatics::FinishSpawningActor(Bomb, SpawnTransform);
		Bomb->ActivateBomb(OwnerVelocity + Direction * OutwardVelocity);
		SpawnedBombs.Add(Bomb);
	}

	return !SpawnedBombs.IsEmpty();
}

void APBSummonBombSkillActor::DestroyBombs()
{
	for (const TWeakObjectPtr<APBBombActor>& Bomb : SpawnedBombs)
	{
		if (Bomb.IsValid())
		{
			Bomb->OnBombFinished.RemoveAll(this);
			Bomb->Destroy();
		}
	}

	SpawnedBombs.Reset();
}

void APBSummonBombSkillActor::HandleBombFinished(AActor* BombActor)
{
	SpawnedBombs.RemoveAll(
		[BombActor](const TWeakObjectPtr<APBBombActor>& Bomb)
		{
			return !Bomb.IsValid() || Bomb.Get() == BombActor;
		});

	if (SpawnedBombs.IsEmpty())
	{
		FinishSkill();
	}
}
