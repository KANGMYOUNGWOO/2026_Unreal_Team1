// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperVulnerabilityComponent.h"
#include "PinBallLike/Actor/Bumper/Feedback/PBBumperVfxRuntimeComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"

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
	UNiagaraSystem* InStatusVfx,
	UStaticMesh* InVisualMesh)
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
		InStatusVfx,
		InVisualMesh);
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
	UNiagaraSystem* InStatusVfx,
	UStaticMesh* InVisualMesh)
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
	ApplyVisualMesh(InVisualMesh);
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

UStaticMeshComponent* APBBumperProjectile::ResolveVisualMeshComponent() const
{
	if (!VisualMeshComponentTag.IsNone())
	{
		const TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
			UStaticMeshComponent::StaticClass(),
			VisualMeshComponentTag);
		for (UActorComponent* TaggedComponent : TaggedComponents)
		{
			if (UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(TaggedComponent))
			{
				return MeshComponent;
			}
		}
	}

	return FindComponentByClass<UStaticMeshComponent>();
}

void APBBumperProjectile::ApplyVisualMesh(UStaticMesh* InVisualMesh) const
{
	if (!IsValid(InVisualMesh))
	{
		return;
	}

	if (UStaticMeshComponent* MeshComponent = ResolveVisualMeshComponent())
	{
		MeshComponent->SetStaticMesh(InVisualMesh);
		MeshComponent->SetRelativeRotation(CustomVisualMeshRotationOffset);
		MeshComponent->SetRelativeScale3D(FVector(FMath::Max(CustomVisualMeshScale, 0.01f)));
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
	int32 AppliedDamage = 0;
	const bool bApplied = ApplyPayload(OtherActor, AppliedDamage);
	StopDeliveryVfx();
	if (bApplied)
	{
		PlayResolvedVfx(OtherActor);
		if (AppliedDamage > 0)
		{
			const FVector HitLocation = IsFromSweep
				? FVector(
					SweepResult.ImpactPoint.X,
					SweepResult.ImpactPoint.Y,
					SweepResult.ImpactPoint.Z)
				: OtherActor->GetActorLocation();
			BroadcastDamageLog(AppliedDamage, HitLocation);
		}
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

bool APBBumperProjectile::ApplyPayload(AActor* Target, int32& OutAppliedDamage) const
{
	OutAppliedDamage = 0;
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
		const bool bApplied = IBossInterface::Execute_DamageToBoss(Target, FinalDamage);
		OutAppliedDamage = bApplied ? FinalDamage : 0;
		return bApplied;
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

void APBBumperProjectile::BroadcastDamageLog(
	const int32 AppliedDamage,
	const FVector& HitLocation) const
{
	if (AppliedDamage <= 0 || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBDamageLogMessage Message;
	Message.Style = EPBDamageLogStyle::PlayerSkill;
	Message.DamageAmount = AppliedDamage;
	Message.HitLocation = HitLocation;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_DamageLog_Requested,
		Message);
}
