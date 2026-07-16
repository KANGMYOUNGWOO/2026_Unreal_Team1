// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTurretSummonActor.h"

#include "EngineUtils.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/PBBossSpawner.h"
#include "PinBallLike/Actor/Bumper/Component/PBTurretFireComponent.h"

APBTurretSummonActor::APBTurretSummonActor()
{
}

void APBTurretSummonActor::SetAttackPayload(
	const EPBBumperProjectilePayload InPayload,
	const int32 InPower)
{
	AttackPayload = InPayload;
	AttackPower = FMath::Max(InPower, 0);
}

void APBTurretSummonActor::StartActionForActor(
	APBModularBumperBase* Bumper,
	AActor* InteractionActor)
{
	AActor* BossTarget = FindBossTarget();
	UPBTurretFireComponent* FireComponent = FindComponentByClass<UPBTurretFireComponent>();
	if (!IsValid(BossTarget)
		|| !IsValid(FireComponent)
		|| AttackPayload == EPBBumperProjectilePayload::None
		|| AttackPower <= 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Turret summon skipped because runtime attack data is incomplete. Turret=%s Boss=%s FireComponent=%s Power=%d"),
			*GetNameSafe(this),
			*GetNameSafe(BossTarget),
			*GetNameSafe(FireComponent),
			AttackPower);
		FinishAction();
		return;
	}

	FireComponent->ConfigureAttack(BossTarget, AttackPayload, AttackPower);
	Super::StartActionForActor(Bumper, InteractionActor);
	OnTurretActivatedForActor(Bumper, InteractionActor);

	if (APBBallBase* Ball = Cast<APBBallBase>(InteractionActor))
	{
		OnTurretActivated(Bumper, Ball);
	}
}

void APBTurretSummonActor::DeactivateSummon()
{
	if (UPBTurretFireComponent* FireComponent = FindComponentByClass<UPBTurretFireComponent>())
	{
		FireComponent->ConfigureAttack(nullptr, EPBBumperProjectilePayload::None, 0);
	}
	Super::DeactivateSummon();
}

AActor* APBTurretSummonActor::FindBossTarget() const
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	for (TActorIterator<APBBossSpawner> It(World); It; ++It)
	{
		if (APBBossBase* SpawnedBoss = It->GetSpawnedBoss())
		{
			return SpawnedBoss;
		}
	}

	for (TActorIterator<APBBossBase> It(World); It; ++It)
	{
		if (IsValid(*It))
		{
			return *It;
		}
	}

	return nullptr;
}
