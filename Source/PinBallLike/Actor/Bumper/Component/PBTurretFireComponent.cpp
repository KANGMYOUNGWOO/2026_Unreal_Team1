// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTurretFireComponent.h"

#include "PinBallLike/Actor/Projectile/ProjectileBase.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPBTurretFireComponent::UPBTurretFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBTurretFireComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!IsUseObjectPool)
	{
		return;
	}

	const int32 PoolSize = FMath::Clamp(InitialPoolSize, 0, MaxPoolSize);
	for (int32 Index = 0; Index < PoolSize; ++Index)
	{
		AProjectileBase* Projectile = SpawnProjectileActor();
		if (IsValid(Projectile))
		{
			Projectile->SetActorHiddenInGame(true);
			Projectile->SetActorEnableCollision(false);
			Projectile->SetActorTickEnabled(false);
			PooledProjectiles.AddUnique(Projectile);
		}
	}
}

void UPBTurretFireComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPool();

	Super::EndPlay(EndPlayReason);
}

AActor* UPBTurretFireComponent::FireOnce()
{
	if (!ProjectileClass)
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!IsValid(Owner) || !IsValid(World))
	{
		return nullptr;
	}

	AProjectileBase* Projectile = IsUseObjectPool ? GetProjectileFromPool() : SpawnProjectileActor();
	if (!IsValid(Projectile))
	{
		return nullptr;
	}

	ActivateProjectile(Projectile, GetMuzzleTransform());

	OnTurretProjectileFired.Broadcast(Projectile);
	return Projectile;
}

void UPBTurretFireComponent::ReleaseProjectile(AActor* Projectile)
{
	DeactivateProjectile(Cast<AProjectileBase>(Projectile));
}

FTransform UPBTurretFireComponent::GetMuzzleTransform() const
{
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return FTransform::Identity;
	}

	const TArray<UActorComponent*> TaggedComponents = Owner->GetComponentsByTag(
		USceneComponent::StaticClass(),
		MuzzleTag);

	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		const USceneComponent* MuzzleComponent = Cast<USceneComponent>(TaggedComponent);
		if (IsValid(MuzzleComponent))
		{
			return MuzzleComponent->GetComponentTransform();
		}
	}

	return Owner->GetActorTransform();
}

AProjectileBase* UPBTurretFireComponent::GetProjectileFromPool()
{
	while (!PooledProjectiles.IsEmpty())
	{
		AProjectileBase* Projectile = PooledProjectiles.Pop();
		if (IsValid(Projectile))
		{
			return Projectile;
		}
	}

	const int32 TotalProjectileCount = PooledProjectiles.Num() + ActiveProjectiles.Num();
	if (TotalProjectileCount >= MaxPoolSize)
	{
		return nullptr;
	}

	return SpawnProjectileActor();
}

AProjectileBase* UPBTurretFireComponent::SpawnProjectileActor()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!ProjectileClass || !IsValid(Owner) || !IsValid(World))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;

	return World->SpawnActor<AProjectileBase>(
		ProjectileClass,
		GetMuzzleTransform(),
		SpawnParameters);
}

void UPBTurretFireComponent::ActivateProjectile(AProjectileBase* Projectile, const FTransform& SpawnTransform)
{
	if (!IsValid(Projectile))
	{
		return;
	}

	Projectile->SetActorTransform(SpawnTransform);
	Projectile->SetActorHiddenInGame(false);
	Projectile->SetActorEnableCollision(true);
	Projectile->SetActorTickEnabled(true);

	ActiveProjectiles.AddUnique(Projectile);
	Projectile->ActivateProjectile();
	OnTurretProjectileActivated.Broadcast(Projectile);

	if (ProjectileLifeTime <= 0.0f)
	{
		return;
	}

	FTimerHandle& LifeTimerHandle = ProjectileLifeTimerHandles.FindOrAdd(Projectile);
	GetWorld()->GetTimerManager().ClearTimer(LifeTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		LifeTimerHandle,
		FTimerDelegate::CreateUObject(this, &UPBTurretFireComponent::DeactivateProjectile, Projectile),
		ProjectileLifeTime,
		false);
}

void UPBTurretFireComponent::DeactivateProjectile(AProjectileBase* Projectile)
{
	if (!IsValid(Projectile))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (FTimerHandle* LifeTimerHandle = ProjectileLifeTimerHandles.Find(Projectile))
		{
			World->GetTimerManager().ClearTimer(*LifeTimerHandle);
		}
	}
	ProjectileLifeTimerHandles.Remove(Projectile);

	ActiveProjectiles.Remove(Projectile);
	Projectile->DeactivateProjectile();
	OnTurretProjectileDeactivated.Broadcast(Projectile);

	if (!IsUseObjectPool)
	{
		Projectile->Destroy();
		return;
	}

	Projectile->SetActorHiddenInGame(true);
	Projectile->SetActorEnableCollision(false);
	Projectile->SetActorTickEnabled(false);

	if (PooledProjectiles.Num() < MaxPoolSize)
	{
		PooledProjectiles.AddUnique(Projectile);
		return;
	}

	Projectile->Destroy();
}

void UPBTurretFireComponent::ClearPool()
{
	if (UWorld* World = GetWorld())
	{
		for (TPair<TWeakObjectPtr<AProjectileBase>, FTimerHandle>& TimerPair : ProjectileLifeTimerHandles)
		{
			World->GetTimerManager().ClearTimer(TimerPair.Value);
		}
	}
	ProjectileLifeTimerHandles.Reset();

	for (AProjectileBase* Projectile : ActiveProjectiles)
	{
		if (IsValid(Projectile))
		{
			Projectile->Destroy();
		}
	}

	for (AProjectileBase* Projectile : PooledProjectiles)
	{
		if (IsValid(Projectile))
		{
			Projectile->Destroy();
		}
	}

	ActiveProjectiles.Reset();
	PooledProjectiles.Reset();
}
