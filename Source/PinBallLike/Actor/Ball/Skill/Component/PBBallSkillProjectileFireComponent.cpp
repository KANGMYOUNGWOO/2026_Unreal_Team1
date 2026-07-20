#include "PBBallSkillProjectileFireComponent.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "PinBallLike/Actor/Ball/Skill/Projectile/PBBallSkillProjectileBase.h"
#include "TimerManager.h"

UPBBallSkillProjectileFireComponent::UPBBallSkillProjectileFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallSkillProjectileFireComponent::InitializeProjectileFire(
	AActor* InDamageSource,
	const int32 InDamageAmount,
	const int32 InGroggyAmount)
{
	DamageSource = InDamageSource;
	DamageAmount = FMath::Max(InDamageAmount, 0);
	GroggyAmount = FMath::Max(InGroggyAmount, 0);
}

void UPBBallSkillProjectileFireComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bUseObjectPool)
	{
		return;
	}

	const int32 PoolSize = FMath::Clamp(InitialPoolSize, 0, MaxPoolSize);
	for (int32 Index = 0; Index < PoolSize; ++Index)
	{
		APBBallSkillProjectileBase* Projectile = SpawnProjectileActor();
		if (IsValid(Projectile))
		{
			Projectile->SetActorHiddenInGame(true);
			Projectile->SetActorEnableCollision(false);
			PooledProjectiles.Add(Projectile);
		}
	}
}

void UPBBallSkillProjectileFireComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearProjectiles();
	Super::EndPlay(EndPlayReason);
}

AActor* UPBBallSkillProjectileFireComponent::FireOnce(const FVector FireDirection)
{
	if (!ProjectileClass
		|| !IsValid(DamageSource)
		|| DamageAmount <= 0
		|| FireDirection.IsNearlyZero())
	{
		return nullptr;
	}

	APBBallSkillProjectileBase* Projectile = bUseObjectPool
		? GetProjectileFromPool()
		: SpawnProjectileActor();
	if (!IsValid(Projectile))
	{
		return nullptr;
	}

	FTransform SpawnTransform = GetMuzzleTransform();
	SpawnTransform.SetRotation(FireDirection.GetSafeNormal().ToOrientationQuat());
	ActivateProjectile(Projectile, SpawnTransform);
	OnProjectileFired.Broadcast(Projectile);
	return Projectile;
}

void UPBBallSkillProjectileFireComponent::ReleaseProjectile(
	AActor* Projectile)
{
	DeactivateProjectile(Cast<APBBallSkillProjectileBase>(Projectile));
}

FTransform UPBBallSkillProjectileFireComponent::GetMuzzleTransform() const
{
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return FTransform::Identity;
	}

	for (UActorComponent* Component : Owner->GetComponentsByTag(USceneComponent::StaticClass(), MuzzleTag))
	{
		if (const USceneComponent* Muzzle = Cast<USceneComponent>(Component))
		{
			return Muzzle->GetComponentTransform();
		}
	}

	return Owner->GetActorTransform();
}

APBBallSkillProjectileBase* UPBBallSkillProjectileFireComponent::GetProjectileFromPool()
{
	while (!PooledProjectiles.IsEmpty())
	{
		if (APBBallSkillProjectileBase* Projectile = PooledProjectiles.Pop())
		{
			return Projectile;
		}
	}

	if (PooledProjectiles.Num() + ActiveProjectiles.Num() + DeactivatingProjectiles.Num() >= MaxPoolSize)
	{
		return nullptr;
	}

	return SpawnProjectileActor();
}

APBBallSkillProjectileBase* UPBBallSkillProjectileFireComponent::SpawnProjectileActor()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!ProjectileClass || !IsValid(Owner) || !World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Owner->GetInstigator();
	APBBallSkillProjectileBase* Projectile = World->SpawnActor<APBBallSkillProjectileBase>(
		ProjectileClass,
		GetMuzzleTransform(),
		SpawnParameters);
	if (IsValid(Projectile))
	{
		Projectile->OnProjectileReleaseRequested.AddUniqueDynamic(
			this,
			&UPBBallSkillProjectileFireComponent::HandleProjectileReleaseRequested);
		Projectile->OnProjectileDeactivationCompleted.AddUniqueDynamic(
			this,
			&UPBBallSkillProjectileFireComponent::HandleProjectileDeactivationCompleted);
	}
	return Projectile;
}

void UPBBallSkillProjectileFireComponent::ActivateProjectile(
	APBBallSkillProjectileBase* Projectile,
	const FTransform& SpawnTransform)
{
	Projectile->InitializeProjectile(DamageAmount, GroggyAmount);
	Projectile->SetActorTransform(SpawnTransform);
	Projectile->SetActorHiddenInGame(false);
	Projectile->SetActorEnableCollision(true);
	ActiveProjectiles.AddUnique(Projectile);
	Projectile->ActivateProjectile();
	OnProjectileActivated.Broadcast(Projectile);

	if (ProjectileLifeTime > 0.0f)
	{
		FTimerHandle& TimerHandle = ProjectileLifeTimerHandles.FindOrAdd(Projectile);
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			FTimerDelegate::CreateUObject(
				this,
				&UPBBallSkillProjectileFireComponent::DeactivateProjectile,
				Projectile),
			ProjectileLifeTime,
			false);
	}
}

void UPBBallSkillProjectileFireComponent::DeactivateProjectile(
	APBBallSkillProjectileBase* Projectile)
{
	if (!IsValid(Projectile) || !ActiveProjectiles.Contains(Projectile))
	{
		return;
	}

	if (FTimerHandle* TimerHandle = ProjectileLifeTimerHandles.Find(Projectile))
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
	}
	ProjectileLifeTimerHandles.Remove(Projectile);

	ActiveProjectiles.Remove(Projectile);
	DeactivatingProjectiles.AddUnique(Projectile);
	Projectile->SetActorEnableCollision(false);
	Projectile->DeactivateProjectile();
}

void UPBBallSkillProjectileFireComponent::CompleteProjectileDeactivation(
	APBBallSkillProjectileBase* Projectile)
{
	if (!IsValid(Projectile) || !DeactivatingProjectiles.Contains(Projectile))
	{
		return;
	}

	DeactivatingProjectiles.Remove(Projectile);
	OnProjectileDeactivated.Broadcast(Projectile);

	if (!bUseObjectPool)
	{
		Projectile->Destroy();
		return;
	}

	Projectile->SetActorHiddenInGame(true);
	Projectile->SetActorEnableCollision(false);
	if (PooledProjectiles.Num() < MaxPoolSize)
	{
		PooledProjectiles.AddUnique(Projectile);
	}
	else
	{
		Projectile->Destroy();
	}
}

void UPBBallSkillProjectileFireComponent::ClearProjectiles()
{
	if (UWorld* World = GetWorld())
	{
		for (TPair<TWeakObjectPtr<APBBallSkillProjectileBase>, FTimerHandle>& Pair : ProjectileLifeTimerHandles)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	ProjectileLifeTimerHandles.Reset();

	for (APBBallSkillProjectileBase* Projectile : ActiveProjectiles)
	{
		if (IsValid(Projectile))
		{
			Projectile->Destroy();
		}
	}
	for (APBBallSkillProjectileBase* Projectile : PooledProjectiles)
	{
		if (IsValid(Projectile))
		{
			Projectile->Destroy();
		}
	}
	for (APBBallSkillProjectileBase* Projectile : DeactivatingProjectiles)
	{
		if (IsValid(Projectile))
		{
			Projectile->Destroy();
		}
	}

	ActiveProjectiles.Reset();
	PooledProjectiles.Reset();
	DeactivatingProjectiles.Reset();
}

void UPBBallSkillProjectileFireComponent::HandleProjectileReleaseRequested(
	AActor* Projectile)
{
	DeactivateProjectile(Cast<APBBallSkillProjectileBase>(Projectile));
}

void UPBBallSkillProjectileFireComponent::HandleProjectileDeactivationCompleted(
	AActor* Projectile)
{
	CompleteProjectileDeactivation(Cast<APBBallSkillProjectileBase>(Projectile));
}
