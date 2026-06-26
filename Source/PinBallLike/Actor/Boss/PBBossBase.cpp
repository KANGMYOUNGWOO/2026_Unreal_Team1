#include "PBBossBase.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBStatTypes.h"
#include "PinBallLike/Actor/Boss/UI/PBBossStatusWidget.h"
#include "PinBallLike/Interface/Comboable.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"
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

void APBBossBase::SetPinballCollisionDamageBlocked(bool IsBlocked)
{
	IsPinballCollisionDamageBlockedValue = IsBlocked;
}

bool APBBossBase::IsPinballCollisionDamageBlocked() const
{
	return IsPinballCollisionDamageBlockedValue;
}

void APBBossBase::BeginPlay()
{
	Super::BeginPlay();

	SetBossState(EPBBossState::Idle);
	BindBossCollisionEvents();
	SetWeaknessState(false);
	CreateBossStatusWidget();
}

void APBBossBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveBossStatusWidget();

	SetWeaknessState(false);

	ClearGroggyResetTimer();
	Super::EndPlay(EndPlayReason);
}

void APBBossBase::TakeBossDamage_Implementation(FName GroggyPointName, int32 DamageAmount)
{
	if (!CanApplyBossDamage(GroggyPointName, DamageAmount))
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
	if (!IsValidDamageSource(OtherActor, OtherComponent))
	{
		return;
	}

	const FName GroggyPointName = ResolveGroggyPointName(HitComponent);
	const int32 DamageAmount = GetPinballHitDamage(OtherActor);
	const bool IsDamageApplied = CanApplyBossDamage(GroggyPointName, DamageAmount)
		&& CanApplyDamageRateLimit(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("Boss Hit Component: %s, GroggyPoint: %s, Damage: %d"),
		HitComponent ? *HitComponent->GetName() : TEXT("None"),
		*GroggyPointName.ToString(),
		DamageAmount);

	if (!IsDamageApplied)
	{
		return;
	}

	IBossInterface::Execute_TakeBossDamage(this, GroggyPointName, DamageAmount);
	RecordDamageRateLimit(OtherActor);
	ApplyPinballHitImpulse(OtherActor, Hit);
	AddPinballCombo(OtherActor);
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

void APBBossBase::CreateBossStatusWidget()
{
	if (BossStatusWidget || !BossStatusWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	BossStatusWidget = CreateWidget<UPBBossStatusWidget>(PlayerController, BossStatusWidgetClass);
	if (!BossStatusWidget)
	{
		return;
	}

	BossStatusWidget->SetBoss(this);
	BossStatusWidget->AddToViewport(BossStatusWidgetZOrder);
}

void APBBossBase::RemoveBossStatusWidget()
{
	if (!BossStatusWidget)
	{
		return;
	}

	BossStatusWidget->ClearBoss();
	BossStatusWidget->RemoveFromParent();
	BossStatusWidget = nullptr;
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

	SetWeaknessCollisionEnabled(IsOpen);
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

		PrimitiveComponent->SetHiddenInGame(!IsEnabled, true);
		PrimitiveComponent->SetVisibility(IsEnabled, true);

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

bool APBBossBase::CanApplyBossDamage(FName GroggyPointName, int32 DamageAmount) const
{
	return DamageAmount > 0 && !IsDead() && !IsWeaknessHitBlocked(GroggyPointName);
}

bool APBBossBase::CanApplyDamageRateLimit(AActor* OtherActor) const
{
	if (MaxDamageCountPerFrame <= 0)
	{
		return false;
	}

	if (LastDamageFrameNumber == GFrameCounter && CurrentFrameDamageCount >= MaxDamageCountPerFrame)
	{
		return false;
	}

	if (!OtherActor || SameSourceHitCooldownSeconds <= 0.0f)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	const float* LastDamageTime = LastDamageTimeMap.Find(TObjectKey<AActor>(OtherActor));
	if (!LastDamageTime)
	{
		return true;
	}

	return World->GetTimeSeconds() - *LastDamageTime >= SameSourceHitCooldownSeconds;
}

bool APBBossBase::IsValidDamageSource(AActor* OtherActor, UPrimitiveComponent* OtherComponent) const
{
	if (!OtherActor)
	{
		return false;
	}

	if (Cast<APBBallBase>(OtherActor))
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

int32 APBBossBase::GetPinballHitDamage(AActor* OtherActor) const
{
	if (IsPinballCollisionDamageBlocked())
	{
		return 0;
	}

	if (!OtherActor)
	{
		return 0;
	}

	const UPBBaseStatComponent* StatComponent = OtherActor->FindComponentByClass<UPBBaseStatComponent>();
	return StatComponent ? StatComponent->GetStat(PBStatNames::Attack) : 0;
}

void APBBossBase::RecordDamageRateLimit(AActor* OtherActor)
{
	if (LastDamageFrameNumber != GFrameCounter)
	{
		LastDamageFrameNumber = GFrameCounter;
		CurrentFrameDamageCount = 0;
	}

	++CurrentFrameDamageCount;

	if (!OtherActor || SameSourceHitCooldownSeconds <= 0.0f)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		LastDamageTimeMap.FindOrAdd(TObjectKey<AActor>(OtherActor)) = World->GetTimeSeconds();
	}
}

void APBBossBase::ApplyPinballHitImpulse(AActor* OtherActor, const FHitResult& Hit) const
{
	if (!OtherActor || PinballHitImpulseStrength <= 0.0f)
	{
		return;
	}

	IMovable* Movable = PBInterfaceUtils::FindInterface<IMovable>(OtherActor);
	if (!Movable)
	{
		return;
	}

	FVector ImpulseDirection = OtherActor->GetActorLocation() - GetActorLocation();
	ImpulseDirection.Z = 0.0f;

	if (ImpulseDirection.IsNearlyZero())
	{
		ImpulseDirection = Hit.ImpactNormal;
		ImpulseDirection.Z = 0.0f;
	}

	ImpulseDirection = ImpulseDirection.GetSafeNormal();
	if (ImpulseDirection.IsNearlyZero())
	{
		return;
	}

	Movable->AddImpulse(ImpulseDirection * PinballHitImpulseStrength);
}

void APBBossBase::AddPinballCombo(AActor* OtherActor) const
{
	IComboable* Comboable = PBInterfaceUtils::FindInterface<IComboable>(OtherActor);
	if (!Comboable)
	{
		return;
	}

	Comboable->AddCombo(1);
}

FName APBBossBase::ResolveGroggyPointName(UPrimitiveComponent* HitComponent) const
{
	if (!HitComponent || HitComponent->ComponentTags.IsEmpty())
	{
		return DefaultGroggyPointName;
	}

	return HitComponent->ComponentTags[0];
}
