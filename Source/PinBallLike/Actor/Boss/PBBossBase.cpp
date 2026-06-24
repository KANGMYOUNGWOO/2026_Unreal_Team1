#include "PBBossBase.h"

#include "PinBallLike/Actor/Ball/BallBase.h"
#include "PinBallLike/Interface/BallDamageSource.h"
#include "Component/PBBossGroggyComponent.h"
#include "Component/PBBossPatternComponent.h"
#include "Component/PBBossStatComponent.h"
#include "Component/PBBossWeaknessComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StateTreeComponent.h"

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
	BossPatternComponent = CreateDefaultSubobject<UPBBossPatternComponent>(TEXT("BossPatternComponent"));
	BossWeaknessComponent = CreateDefaultSubobject<UPBBossWeaknessComponent>(TEXT("BossWeaknessComponent"));
	BossStateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("BossStateTreeComponent"));
}

UPBBossStatComponent* APBBossBase::GetBossStatComponent() const
{
	return BossStatComponent;
}

UPBBossGroggyComponent* APBBossBase::GetBossGroggyComponent() const
{
	return BossGroggyComponent;
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

void APBBossBase::BeginPlay()
{
	Super::BeginPlay();

	SetBossState(EPBBossState::Idle);
	BindBossCollisionEvents();
	SetWeaknessCollisionEnabled(false);
}

void APBBossBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BossWeaknessComponent)
	{
		BossWeaknessComponent->CloseWeakness();
	}
	SetWeaknessCollisionEnabled(false);

	ClearGroggyResetTimer();
	Super::EndPlay(EndPlayReason);
}

void APBBossBase::TakeBossDamage_Implementation(FName GroggyPointName, int32 DamageAmount)
{
	if (DamageAmount <= 0 || IsDead())
	{
		return;
	}

	if (IsWeaknessHitBlocked(GroggyPointName))
	{
		return;
	}

	BP_OnDamaged(GroggyPointName, DamageAmount);

	if (BossStatComponent)
	{
		const int32 FinalDamageAmount = BossWeaknessComponent
			? BossWeaknessComponent->CalculateWeaknessDamage(GroggyPointName, DamageAmount)
			: DamageAmount;

		BossStatComponent->ApplyBossDamage(GroggyPointName, FinalDamageAmount);
	}

	if (IsDead())
	{
		return;
	}

	if (BossGroggyComponent)
	{
		BossGroggyComponent->ApplyGroggyDamage(GroggyPointName);
	}
}

void APBBossBase::OnGroggyTriggered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("BossBase Groggy Started."));
	SetBossState(EPBBossState::Groggy);

	if (BossPatternComponent)
	{
		BossPatternComponent->CancelCurrentPattern();
	}

	if (BossWeaknessComponent)
	{
		BossWeaknessComponent->OpenWeakness();
	}
	SetWeaknessCollisionEnabled(true);

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

	if (BossWeaknessComponent)
	{
		BossWeaknessComponent->CloseWeakness();
	}
	SetWeaknessCollisionEnabled(false);

	ClearGroggyResetTimer();
	BP_OnDead();
}

void APBBossBase::TakeDamage(int32 Damage)
{
	IBossInterface::Execute_TakeBossDamage(this, DefaultGroggyPointName, Damage);
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
	if (!IsValidDamageSource(OtherActor, OtherComponent))
	{
		return;
	}

	const FName GroggyPointName = ResolveGroggyPointName(HitComponent);

	UE_LOG(LogTemp, Warning, TEXT("Boss Hit Component: %s, GroggyPoint: %s"),
		HitComponent ? *HitComponent->GetName() : TEXT("None"),
		*GroggyPointName.ToString());

	IBossInterface::Execute_TakeBossDamage(this, GroggyPointName, PinballHitDamage);
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

	if (BossWeaknessComponent)
	{
		BossWeaknessComponent->CloseWeakness();
	}
	SetWeaknessCollisionEnabled(false);

	BossGroggyComponent->ResetGroggy();
	SetBossState(EPBBossState::Idle);
}

void APBBossBase::SetWeaknessCollisionEnabled(bool IsEnabled)
{
	if (!BossWeaknessComponent)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent || !IsWeaknessCollisionComponent(PrimitiveComponent))
		{
			continue;
		}

		if (!WeaknessCollisionEnabledMap.Contains(PrimitiveComponent))
		{
			WeaknessCollisionEnabledMap.Add(PrimitiveComponent, PrimitiveComponent->GetCollisionEnabled());
		}

		if (IsEnabled)
		{
			const ECollisionEnabled::Type* OriginalCollisionEnabled = WeaknessCollisionEnabledMap.Find(PrimitiveComponent);
			PrimitiveComponent->SetCollisionEnabled(OriginalCollisionEnabled ? *OriginalCollisionEnabled : ECollisionEnabled::QueryAndPhysics);
			continue;
		}

		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

bool APBBossBase::IsWeaknessCollisionComponent(const UPrimitiveComponent* PrimitiveComponent) const
{
	if (!PrimitiveComponent || !BossWeaknessComponent)
	{
		return false;
	}

	for (const FName ComponentTag : PrimitiveComponent->ComponentTags)
	{
		if (BossWeaknessComponent->IsWeaknessPoint(ComponentTag))
		{
			return true;
		}
	}

	return false;
}

bool APBBossBase::IsWeaknessHitBlocked(FName HitPointName) const
{
	if (!BossWeaknessComponent || !BossWeaknessComponent->IsWeaknessPoint(HitPointName))
	{
		return false;
	}

	return !BossWeaknessComponent->IsWeaknessPointOpen(HitPointName);
}

bool APBBossBase::IsValidDamageSource(AActor* OtherActor, UPrimitiveComponent* OtherComponent) const
{
	if (!OtherActor)
	{
		return false;
	}

	if (Cast<ABallBase>(OtherActor))
	{
		return true;
	}

	if (OtherActor->GetClass()->ImplementsInterface(UBallDamageSource::StaticClass()))
	{
		return true;
	}

	if (DamageSourceTagName.IsNone())
	{
		return false;
	}

	const bool IsActorTagged = OtherActor->ActorHasTag(DamageSourceTagName);
	const bool IsComponentTagged = OtherComponent && OtherComponent->ComponentHasTag(DamageSourceTagName);

	return IsActorTagged || IsComponentTagged;
}

FName APBBossBase::ResolveGroggyPointName(UPrimitiveComponent* HitComponent) const
{
	if (!HitComponent || HitComponent->ComponentTags.IsEmpty())
	{
		return DefaultGroggyPointName;
	}

	return HitComponent->ComponentTags[0];
}
