#include "PBBossBase.h"

#include "Component/PBBossDamageComponent.h"
#include "Component/PBBossDamageReceiverComponent.h"
#include "Component/PBBossGroggyComponent.h"
#include "Component/PBBossPatternComponent.h"
#include "Component/PBBossPinballReactionComponent.h"
#include "Component/PBBossStatComponent.h"
#include "Component/PBBossUIComponent.h"
#include "Component/PBBossWeaknessComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StateTreeComponent.h"
#include "PinBallLike/Actor/Boss/UI/PBBossStatusWidget.h"

APBBossBase::APBBossBase()
{
	PrimaryActorTick.bCanEverTick = false;
	BossName = NSLOCTEXT("Boss", "DefaultBossName", "Boss");

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(100.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetGenerateOverlapEvents(false);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	BossStatComponent = CreateDefaultSubobject<UPBBossStatComponent>(TEXT("BossStatComponent"));
	BossGroggyComponent = CreateDefaultSubobject<UPBBossGroggyComponent>(TEXT("BossGroggyComponent"));
	BossDamageComponent = CreateDefaultSubobject<UPBBossDamageComponent>(TEXT("BossDamageComponent"));
	BossDamageReceiverComponent = CreateDefaultSubobject<UPBBossDamageReceiverComponent>(TEXT("BossDamageReceiverComponent"));
	BossPatternComponent = CreateDefaultSubobject<UPBBossPatternComponent>(TEXT("BossPatternComponent"));
	BossPinballReactionComponent = CreateDefaultSubobject<UPBBossPinballReactionComponent>(TEXT("BossPinballReactionComponent"));
	BossWeaknessComponent = CreateDefaultSubobject<UPBBossWeaknessComponent>(TEXT("BossWeaknessComponent"));
	BossStateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("BossStateTreeComponent"));
	BossUIComponent = CreateDefaultSubobject<UPBBossUIComponent>(TEXT("BossUIComponent"));
}

UPBBossStatComponent* APBBossBase::GetBossStatComponent() const
{
	return BossStatComponent;
}

UPBBossGroggyComponent* APBBossBase::GetBossGroggyComponent() const
{
	return BossGroggyComponent;
}

UPBBossDamageComponent* APBBossBase::GetBossDamageComponent() const
{
	return BossDamageComponent;
}

UPBBossPatternComponent* APBBossBase::GetBossPatternComponent() const
{
	return BossPatternComponent;
}

UPBBossWeaknessComponent* APBBossBase::GetBossWeaknessComponent() const
{
	return BossWeaknessComponent;
}

UStateTreeComponent* APBBossBase::GetBossStateTreeComponent() const
{
	return BossStateTreeComponent;
}

UPBBossUIComponent* APBBossBase::GetBossUIComponent() const
{
	return BossUIComponent;
}

void APBBossBase::SetBossState(EPBBossState NewBossState)
{
	BossState = NewBossState;
}

EPBBossState APBBossBase::GetBossState() const
{
	return BossState;
}

FText APBBossBase::GetBossName() const
{
	return BossName;
}

void APBBossBase::SetPinballCollisionDamageBlocked(bool IsBlocked)
{
	if (BossDamageComponent)
	{
		BossDamageComponent->SetPinballCollisionDamageBlocked(IsBlocked);
	}
}

bool APBBossBase::IsPinballCollisionDamageBlocked() const
{
	return BossDamageComponent && BossDamageComponent->IsPinballCollisionDamageBlocked();
}

void APBBossBase::NotifyBossDamaged(FName HitPointName, int32 DamageAmount)
{
	BP_OnDamaged(HitPointName, DamageAmount);
}

void APBBossBase::BeginPlay()
{
	Super::BeginPlay();

	SetBossState(EPBBossState::Idle);
	BindBossCollisionEvents();
	SetWeaknessState(false);

	if (BossUIComponent && BossStatusWidgetClass)
	{
		BossUIComponent->ConfigureBossStatusWidget(BossStatusWidgetClass, BossStatusWidgetZOrder);
		BossUIComponent->CreateBossStatusWidget();
	}
}

void APBBossBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetWeaknessState(false);

	ClearGroggyResetTimer();
	Super::EndPlay(EndPlayReason);
}

void APBBossBase::TakeBossDamage_Implementation(FName GroggyPointName, int32 DamageAmount)
{
	if (!BossDamageComponent)
	{
		return;
	}

	BossDamageComponent->ApplyPointDamage(GroggyPointName, DamageAmount);
}

void APBBossBase::OnGroggyTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Started."));
	SetBossState(EPBBossState::Groggy);

	if (BossPatternComponent)
	{
		if (BossPatternComponent->GetCurrentPattern())
		{
			UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Cancel Current Pattern."));
		}
		BossPatternComponent->PausePatternSystem();
	}

	SetWeaknessState(true);

	StartGroggyResetTimer();
	BP_OnGroggyStarted();
}

void APBBossBase::OnEnragedTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Enraged Started."));
	SetBossState(EPBBossState::Enraged);
	BP_OnEnragedStarted();
}

void APBBossBase::OnDeadTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Dead."));
	SetBossState(EPBBossState::Dead);

	if (BossPatternComponent)
	{
		BossPatternComponent->StopPatternSystem();
	}

	SetWeaknessState(false);

	ClearGroggyResetTimer();
	BP_OnDead();
}

bool APBBossBase::IsDead() const
{
	return BossStatComponent ? BossStatComponent->IsDead() : true;
}

void APBBossBase::HandleCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	static_cast<void>(NormalImpulse);

	if (BossDamageComponent && BossDamageComponent->IsValidDamageSource(OtherActor, OtherComponent))
	{
		BossDamageComponent->ApplyHitPartDamage(
			OtherActor,
			HitComponent,
			BossDamageComponent->GetPinballHitDamage(OtherActor),
			Hit);
	}
}

void APBBossBase::BindBossCollisionEvents()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetNotifyRigidBodyCollision(true);
		PrimitiveComponent->OnComponentHit.AddUniqueDynamic(this, &APBBossBase::HandleCollisionHit);
	}
}

void APBBossBase::StartGroggyResetTimer()
{
	if (!BossGroggyComponent)
	{
		return;
	}

	if (GroggyDurationSeconds <= 0.0f)
	{
		HandleGroggyDurationFinished();
		return;
	}

	GetWorldTimerManager().SetTimer(
		GroggyResetTimerHandle,
		this,
		&APBBossBase::HandleGroggyDurationFinished,
		GroggyDurationSeconds,
		false);
}

void APBBossBase::ClearGroggyResetTimer()
{
	GetWorldTimerManager().ClearTimer(GroggyResetTimerHandle);
}

void APBBossBase::HandleGroggyDurationFinished()
{
	if (!BossGroggyComponent || IsDead())
	{
		return;
	}

	SetWeaknessState(false);

	BossGroggyComponent->ResetGroggy();
	SetBossState(EPBBossState::Idle);

	if (BossPatternComponent && BossPatternComponent->ResumePatternSystem())
	{
		UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Finished. Restart Pattern System."));
	}
}

void APBBossBase::SetWeaknessState(bool IsOpen)
{
	if (BossWeaknessComponent)
	{
		if (IsOpen)
		{
			BossWeaknessComponent->OpenWeakness();
		}
		else
		{
			BossWeaknessComponent->CloseWeakness();
		}
	}
}
