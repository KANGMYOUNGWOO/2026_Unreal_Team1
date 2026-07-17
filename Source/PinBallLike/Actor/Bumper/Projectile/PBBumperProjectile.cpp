// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperVulnerabilityComponent.h"
#include "PinBallLike/Interface/BossInterface.h"

APBBumperProjectile::APBBumperProjectile()
{
	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
		ProjectileMovementComponent->MaxSpeed = ProjectileSpeed;
	}

	if (IsValid(CollisionSphere))
	{
		CollisionSphere->InitSphereRadius(12.0f);
		CollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(
			this,
			&APBBumperProjectile::HandleProjectileBeginOverlap);
	}

	if (IsValid(ProjectileMovementComponent))
	{
		// 빠른 탄환이 보스 충돌을 건너뛰지 않도록 이동 중 sweep을 사용합니다.
		ProjectileMovementComponent->bSweepCollision = true;
	}
}

APBBumperProjectile* APBBumperProjectile::SpawnForTarget(
	UObject* WorldContext,
	TSubclassOf<APBBumperProjectile> InProjectileClass,
	AActor* OwnerActor,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	AActor* InTargetActor,
	const EPBBumperProjectilePayload InPayload,
	const int32 InPower,
	const float InPayloadDuration,
	const float InLifetime)
{
	UWorld* World = IsValid(WorldContext) ? WorldContext->GetWorld() : nullptr;
	if (!IsValid(World)
		|| !InProjectileClass
		|| !IsValid(InTargetActor)
		|| InPayload == EPBBumperProjectilePayload::None
		|| InPower <= 0)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerActor;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APBBumperProjectile* Projectile = World->SpawnActor<APBBumperProjectile>(
		InProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (!IsValid(Projectile))
	{
		return nullptr;
	}

	Projectile->ConfigureForTarget(
		InTargetActor,
		InPayload,
		InPower,
		true,
		InPayloadDuration);
	Projectile->SetLifeSpan(FMath::Max(InLifetime, 0.1f));
	Projectile->ActivateProjectile();
	return Projectile;
}

void APBBumperProjectile::ConfigureForTarget(
	AActor* InTargetActor,
	const EPBBumperProjectilePayload InPayload,
	const int32 InPower,
	const bool bInDestroyOnResolved,
	const float InPayloadDuration)
{
	TargetActor = InTargetActor;
	Payload = InPayload;
	PayloadPower = FMath::Max(InPower, 0);
	PayloadDuration = FMath::Max(InPayloadDuration, 0.0f);
	bDestroyOnResolved = bInDestroyOnResolved;
	bHasResolved = false;

	if (!IsValid(ProjectileMovementComponent))
	{
		return;
	}

	USceneComponent* TargetComponent = IsValid(InTargetActor)
		? InTargetActor->GetRootComponent()
		: nullptr;
	ProjectileMovementComponent->bIsHomingProjectile = IsValid(TargetComponent);
	ProjectileMovementComponent->HomingTargetComponent = TargetComponent;
	ProjectileMovementComponent->HomingAccelerationMagnitude = FMath::Max(HomingAcceleration, 0.0f);

	if (IsValid(InTargetActor))
	{
		const FVector Direction = InTargetActor->GetActorLocation() - GetActorLocation();
		if (!Direction.IsNearlyZero())
		{
			SetActorRotation(Direction.Rotation());
		}
	}
}

void APBBumperProjectile::ResetForPool()
{
	OnProjectileResolved.Clear();
	TargetActor.Reset();
	Payload = EPBBumperProjectilePayload::None;
	PayloadPower = 0;
	PayloadDuration = 0.0f;
	bDestroyOnResolved = false;
	bHasResolved = false;

	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->bIsHomingProjectile = false;
		ProjectileMovementComponent->HomingTargetComponent = nullptr;
	}
}

void APBBumperProjectile::HandleProjectileBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool IsFromSweep,
	const FHitResult& SweepResult)
{
	Super::HandleProjectileBeginOverlap(
		OverlappedComponent,
		OtherActor,
		OtherComponent,
		OtherBodyIndex,
		IsFromSweep,
		SweepResult);

	if (bHasResolved || !IsValid(OtherActor) || OtherActor != TargetActor.Get())
	{
		return;
	}

	bHasResolved = true;
	const bool bApplied = ApplyPayload(OtherActor);
	OnProjectileResolved.Broadcast(this, bApplied);

	if (bDestroyOnResolved && IsValid(this))
	{
		SetActorEnableCollision(false);
		DeactivateProjectile();
		Destroy();
	}
}

bool APBBumperProjectile::ApplyPayload(AActor* Target) const
{
	if (!IsValid(Target)
		|| PayloadPower <= 0
		|| !Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		return false;
	}

	switch (Payload)
	{
	case EPBBumperProjectilePayload::BossDamage:
	{
		const UPBBumperVulnerabilityComponent* VulnerabilityComponent =
			Target->FindComponentByClass<UPBBumperVulnerabilityComponent>();
		const int32 FinalDamage = IsValid(VulnerabilityComponent)
			? VulnerabilityComponent->CalculateBumperProjectileDamage(PayloadPower)
			: PayloadPower;
		return IBossInterface::Execute_DamageToBoss(Target, FinalDamage);
	}

	case EPBBumperProjectilePayload::BossGroggy:
		IBossInterface::Execute_IncreaseGroggy(Target, PayloadPower);
		return true;

	case EPBBumperProjectilePayload::BossVulnerability:
	{
		UPBBumperVulnerabilityComponent* VulnerabilityComponent =
			UPBBumperVulnerabilityComponent::FindOrAddToActor(Target);
		return IsValid(VulnerabilityComponent)
			&& VulnerabilityComponent->ApplyVulnerability(
				static_cast<float>(PayloadPower),
				PayloadDuration);
	}

	default:
		return false;
	}
}
