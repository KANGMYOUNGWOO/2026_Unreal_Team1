// Fill out your copyright notice in the Description page of Project Settings.


#include "PBSummonBumperEffect.h"

#include "Engine/World.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"

void UPBSummonBumperEffect::Initialize(APBModularBumperBase* InOwnerBumper)
{
	Super::Initialize(InOwnerBumper);

	EnsureSummonActor(InOwnerBumper);
}

void UPBSummonBumperEffect::ActivateEffect(APBModularBumperBase* Bumper, APBBallBase* Ball)
{
	APBModularBumperBase* TargetBumper = IsValid(Bumper) ? Bumper : OwnerBumper.Get();
	if (!IsValid(TargetBumper) || !IsValid(Ball) || !EnsureSummonActor(TargetBumper))
	{
		FinishEffect();
		return;
	}

	SpawnedSummonActor->StartAction(TargetBumper, Ball);
}

void UPBSummonBumperEffect::FinishEffect()
{
	if (IsDestroyOnFinished)
	{
		DestroySummonActor();
	}
	else
	{
		DeactivateSummonActor();
	}

	Super::FinishEffect();
}

void UPBSummonBumperEffect::BeginDestroy()
{
	DestroySummonActor();

	Super::BeginDestroy();
}

void UPBSummonBumperEffect::HandleSummonActionFinished(APBBumperSummonActor* SummonActor)
{
	if (SummonActor != SpawnedSummonActor)
	{
		return;
	}

	FinishEffect();
}

bool UPBSummonBumperEffect::EnsureSummonActor(APBModularBumperBase* Bumper)
{
	if (IsValid(SpawnedSummonActor))
	{
		return true;
	}

	if (!IsValid(Bumper) || !SummonActorClass)
	{
		return false;
	}

	UWorld* World = Bumper->GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	const FTransform SpawnTransform = SpawnOffset * Bumper->GetActorTransform();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Bumper;

	// 범퍼가 생성될 때 소환 객체를 미리 배치해두고, 발동 시에는 동작만 실행한다.
	SpawnedSummonActor = World->SpawnActor<APBBumperSummonActor>(
		SummonActorClass,
		SpawnTransform,
		SpawnParameters);

	if (!IsValid(SpawnedSummonActor))
	{
		SpawnedSummonActor = nullptr;
		return false;
	}

	SpawnedSummonActor->AttachToActor(Bumper, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedSummonActor->OnSummonActionFinished.AddUniqueDynamic(
		this,
		&UPBSummonBumperEffect::HandleSummonActionFinished);

	return true;
}

void UPBSummonBumperEffect::DeactivateSummonActor() const
{
	if (IsValid(SpawnedSummonActor))
	{
		SpawnedSummonActor->DeactivateSummon();
	}
}

void UPBSummonBumperEffect::DestroySummonActor()
{
	if (!IsValid(SpawnedSummonActor))
	{
		SpawnedSummonActor = nullptr;
		return;
	}

	SpawnedSummonActor->OnSummonActionFinished.RemoveDynamic(
		this,
		&UPBSummonBumperEffect::HandleSummonActionFinished);
	SpawnedSummonActor->DeactivateSummon();
	SpawnedSummonActor->Destroy();

	SpawnedSummonActor = nullptr;
}
