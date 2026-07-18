// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperVulnerabilityComponent.h"
#include "PinBallLike/Actor/Bumper/Feedback/PBBumperVfxRuntimeComponent.h"
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
	const float InLifetime,
	UNiagaraSystem* InDeliveryVfx,
	UNiagaraSystem* InImpactVfx,
	UNiagaraSystem* InStatusVfx)
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
		InPayloadDuration,
		InDeliveryVfx,
		InImpactVfx,
		InStatusVfx);
	Projectile->SetLifeSpan(FMath::Max(InLifetime, 0.1f));
	Projectile->ActivateProjectile();
	return Projectile;
}

void APBBumperProjectile::ConfigureForTarget(
	AActor* InTargetActor,
	const EPBBumperProjectilePayload InPayload,
	const int32 InPower,
	const bool bInDestroyOnResolved,
	const float InPayloadDuration,
	UNiagaraSystem* InDeliveryVfx,
	UNiagaraSystem* InImpactVfx,
	UNiagaraSystem* InStatusVfx)
{
	TargetActor = InTargetActor;
	Payload = InPayload;
	PayloadPower = FMath::Max(InPower, 0);
	PayloadDuration = FMath::Max(InPayloadDuration, 0.0f);
	DeliveryVfx = InDeliveryVfx;
	ImpactVfx = InImpactVfx;
	StatusVfx = InStatusVfx;
	bDestroyOnResolved = bInDestroyOnResolved;
	bHasResolved = false;
	StartDeliveryVfx();

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
	StopDeliveryVfx();
	OnProjectileResolved.Clear();
	TargetActor.Reset();
	Payload = EPBBumperProjectilePayload::None;
	PayloadPower = 0;
	PayloadDuration = 0.0f;
	DeliveryVfx = nullptr;
	ImpactVfx = nullptr;
	StatusVfx = nullptr;
	bDestroyOnResolved = false;
	bHasResolved = false;

	if (IsValid(ProjectileMovementComponent))
	{
		ProjectileMovementComponent->bIsHomingProjectile = false;
		ProjectileMovementComponent->HomingTargetComponent = nullptr;
	}
}

void APBBumperProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopDeliveryVfx();
	Super::EndPlay(EndPlayReason);
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
	StopDeliveryVfx();
	if (bApplied)
	{
		PlayResolvedVfx(OtherActor);
	}
	OnProjectileResolved.Broadcast(this, bApplied);

	if (bDestroyOnResolved && IsValid(this))
	{
		SetActorEnableCollision(false);
		DeactivateProjectile();
		Destroy();
	}
}

void APBBumperProjectile::StartDeliveryVfx()
{
	StopDeliveryVfx();
	if (!IsValid(DeliveryVfx) || !IsValid(GetRootComponent()))
	{
		return;
	}

	DeliveryVfxComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		DeliveryVfx,
		GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		false,
		true,
		ENCPoolMethod::None,
		true);
}

void APBBumperProjectile::StopDeliveryVfx()
{
	if (IsValid(DeliveryVfxComponent))
	{
		DeliveryVfxComponent->DeactivateImmediate();
		DeliveryVfxComponent->DestroyComponent();
	}
	DeliveryVfxComponent = nullptr;
}

void APBBumperProjectile::PlayResolvedVfx(AActor* Target) const
{
	UPBBumperVfxRuntimeComponent::PlayImpact(this, ImpactVfx, Target);
	if (PayloadDuration <= 0.0f || !IsValid(StatusVfx) || !IsValid(Target))
	{
		return;
	}

	if (UPBBumperVfxRuntimeComponent* RuntimeVfx =
		UPBBumperVfxRuntimeComponent::FindOrAddToActor(Target))
	{
		RuntimeVfx->PlayAttached(
			FName(*FString::Printf(TEXT("BumperProjectileStatus_%d"), static_cast<int32>(Payload))),
			StatusVfx,
			PayloadDuration);
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
