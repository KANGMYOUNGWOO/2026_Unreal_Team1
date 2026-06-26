#include "PBBossDamageReceiverComponent.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossDamageComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"

UPBBossDamageReceiverComponent::UPBBossDamageReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossDamageReceiverComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerBoss = Cast<APBBossBase>(GetOwner());
	if (!OwnerBoss)
	{
		return;
	}

	if (UPBBossDamageComponent* DamageComponent = OwnerBoss->GetBossDamageComponent())
	{
		DamageComponent->OnBossDamageApplied.AddUObject(this, &UPBBossDamageReceiverComponent::HandleBossDamageApplied);
	}
}

void UPBBossDamageReceiverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerBoss)
	{
		if (UPBBossDamageComponent* DamageComponent = OwnerBoss->GetBossDamageComponent())
		{
			DamageComponent->OnBossDamageApplied.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UPBBossDamageReceiverComponent::HandleBossDamageApplied(FName HitPointName, int32 DamageAmount)
{
	if (!OwnerBoss)
	{
		return;
	}

	OwnerBoss->NotifyBossDamaged(HitPointName, DamageAmount);

	if (UPBBossStatComponent* StatComponent = OwnerBoss->GetBossStatComponent())
	{
		StatComponent->ApplyBossDamage(HitPointName, DamageAmount);
	}

	if (!OwnerBoss->IsDead())
	{
		if (UPBBossGroggyComponent* GroggyComponent = OwnerBoss->GetBossGroggyComponent())
		{
			GroggyComponent->ApplyGroggyDamage(HitPointName);
		}
	}
}
