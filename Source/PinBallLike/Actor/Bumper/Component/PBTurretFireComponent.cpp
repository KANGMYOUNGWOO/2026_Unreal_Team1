// Fill out your copyright notice in the Description page of Project Settings.


#include "PBTurretFireComponent.h"

#include "Components/SceneComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/World.h"
#include "PinBallLike/Actor/Projectile/ProjectileBase.h"
#include "TimerManager.h"

UPBTurretFireComponent::UPBTurretFireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UPBTurretFireComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		TArray<UActorComponent*> TimelineComponents;
		Owner->GetComponents(UTimelineComponent::StaticClass(), TimelineComponents);
		for (UActorComponent* TimelineComponent : TimelineComponents)
		{
			if (IsValid(TimelineComponent))
			{
				AddTickPrerequisiteComponent(TimelineComponent);
			}
		}
	}

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
	SetComponentTickEnabled(false);
	AttackTarget.Reset();
	CachedAimPivot.Reset();
	ClearPool();

	Super::EndPlay(EndPlayReason);
}

void UPBTurretFireComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAimAtTargetBeforeFire || !AttackTarget.IsValid())
	{
		SetComponentTickEnabled(false);
		return;
	}

	AimAtTarget();
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
	if (ProjectileClass->IsChildOf(APBBumperProjectile::StaticClass())
		&& (!AttackTarget.IsValid()
			|| AttackPayload == EPBBumperProjectilePayload::None
			|| AttackPower <= 0
			|| (MaxAttackShotCount > 0
				&& FiredAttackShotCount >= MaxAttackShotCount)))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Bumper] Turret projectile skipped because attack payload is incomplete. Owner=%s Target=%s Power=%d"),
			*GetNameSafe(Owner),
			*GetNameSafe(AttackTarget.Get()),
			AttackPower);
		return nullptr;
	}

	if (bAimAtTargetBeforeFire)
	{
		AimAtTarget();
	}

	AProjectileBase* Projectile = IsUseObjectPool ? GetProjectileFromPool() : SpawnProjectileActor();
	if (!IsValid(Projectile))
	{
		return nullptr;
	}

	ActivateProjectile(Projectile, GetMuzzleTransform());
	if (MaxAttackShotCount > 0)
	{
		++FiredAttackShotCount;
	}

	OnTurretProjectileFired.Broadcast(Projectile);
	return Projectile;
}

void UPBTurretFireComponent::ConfigureAttack(
	AActor* InTargetActor,
	const EPBBumperProjectilePayload InPayload,
	const int32 InPower,
	const int32 InMaxShotCount,
	UNiagaraSystem* InDeliveryVfx,
	UNiagaraSystem* InImpactVfx)
{
	AttackTarget = InTargetActor;
	AttackPayload = InPayload;
	AttackPower = FMath::Max(InPower, 0);
	MaxAttackShotCount = FMath::Max(InMaxShotCount, 0);
	FiredAttackShotCount = 0;
	DeliveryVfx = InDeliveryVfx;
	ImpactVfx = InImpactVfx;

	const bool bShouldTrackTarget = bAimAtTargetBeforeFire && AttackTarget.IsValid();
	SetComponentTickEnabled(bShouldTrackTarget);
	if (bShouldTrackTarget)
	{
		AimAtTarget();
	}
}

bool UPBTurretFireComponent::AimAtTarget()
{
	AActor* Owner = GetOwner();
	AActor* Target = AttackTarget.Get();
	if (!IsValid(Owner) || !IsValid(Target))
	{
		return false;
	}

	USceneComponent* AimPivot = ResolveAimPivot();
	const FVector SourceLocation = IsValid(AimPivot)
		? AimPivot->GetComponentLocation()
		: Owner->GetActorLocation();
	const FRotator CurrentRotation = IsValid(AimPivot)
		? AimPivot->GetComponentRotation()
		: Owner->GetActorRotation();

	FRotator AimRotation = CurrentRotation;
	const FVector TargetDirection = Target->GetActorLocation() - SourceLocation;
	USceneComponent* MuzzleComponent = ResolveMuzzleComponent();
	const bool bResolvedFromMuzzle = IsValid(AimPivot)
		&& IsValid(MuzzleComponent)
		&& TryResolveAimRotationFromDirections(
			MuzzleComponent->GetComponentLocation() - SourceLocation,
			TargetDirection,
			CurrentRotation,
			bYawOnlyAim,
			AimYawOffsetDegrees,
			AimRotation);
	if (!bResolvedFromMuzzle
		&& !TryResolveAimRotation(
			SourceLocation,
			Target->GetActorLocation(),
			CurrentRotation,
			bYawOnlyAim,
			AimYawOffsetDegrees,
			AimRotation))
	{
		return false;
	}

	if (IsValid(AimPivot))
	{
		AimPivot->SetWorldRotation(AimRotation);
	}
	else
	{
		Owner->SetActorRotation(AimRotation);
	}
	return true;
}

bool UPBTurretFireComponent::TryResolveAimRotationFromDirections(
	const FVector& CurrentAimDirection,
	const FVector& TargetDirection,
	const FRotator& CurrentRotation,
	const bool bYawOnly,
	const float YawOffsetDegrees,
	FRotator& OutRotation)
{
	FVector CurrentDirection = CurrentAimDirection;
	FVector DesiredDirection = TargetDirection;
	if (bYawOnly)
	{
		CurrentDirection.Z = 0.0f;
		DesiredDirection.Z = 0.0f;
	}

	if (CurrentDirection.IsNearlyZero() || DesiredDirection.IsNearlyZero())
	{
		return false;
	}

	CurrentDirection.Normalize();
	DesiredDirection.Normalize();
	if (bYawOnly)
	{
		const float DeltaYaw = FMath::FindDeltaAngleDegrees(
			CurrentDirection.Rotation().Yaw,
			DesiredDirection.Rotation().Yaw);
		OutRotation = CurrentRotation;
		OutRotation.Yaw = FRotator::NormalizeAxis(
			CurrentRotation.Yaw + DeltaYaw + YawOffsetDegrees);
		return true;
	}

	const FQuat DirectionDelta = FQuat::FindBetweenNormals(
		CurrentDirection,
		DesiredDirection);
	OutRotation = (DirectionDelta * CurrentRotation.Quaternion()).Rotator();
	OutRotation.Yaw = FRotator::NormalizeAxis(OutRotation.Yaw + YawOffsetDegrees);
	return true;
}

bool UPBTurretFireComponent::TryResolveAimRotation(
	const FVector& SourceLocation,
	const FVector& TargetLocation,
	const FRotator& CurrentRotation,
	const bool bYawOnly,
	const float YawOffsetDegrees,
	FRotator& OutRotation)
{
	FVector Direction = TargetLocation - SourceLocation;
	if (bYawOnly)
	{
		Direction.Z = 0.0f;
	}

	if (Direction.IsNearlyZero())
	{
		return false;
	}

	OutRotation = Direction.Rotation();
	OutRotation.Yaw = FRotator::NormalizeAxis(OutRotation.Yaw + YawOffsetDegrees);
	if (bYawOnly)
	{
		OutRotation.Pitch = CurrentRotation.Pitch;
		OutRotation.Roll = CurrentRotation.Roll;
	}

	return true;
}

USceneComponent* UPBTurretFireComponent::ResolveAimPivot()
{
	if (CachedAimPivot.IsValid())
	{
		return CachedAimPivot.Get();
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || AimPivotTag.IsNone())
	{
		return nullptr;
	}

	const TArray<UActorComponent*> TaggedComponents = Owner->GetComponentsByTag(
		USceneComponent::StaticClass(),
		AimPivotTag);
	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		if (USceneComponent* AimPivot = Cast<USceneComponent>(TaggedComponent))
		{
			CachedAimPivot = AimPivot;
			return AimPivot;
		}
	}

	return nullptr;
}

USceneComponent* UPBTurretFireComponent::ResolveMuzzleComponent() const
{
	const AActor* Owner = GetOwner();
	if (!IsValid(Owner) || MuzzleTag.IsNone())
	{
		return nullptr;
	}

	const TArray<UActorComponent*> TaggedComponents = Owner->GetComponentsByTag(
		USceneComponent::StaticClass(),
		MuzzleTag);
	for (UActorComponent* TaggedComponent : TaggedComponents)
	{
		if (USceneComponent* MuzzleComponent = Cast<USceneComponent>(TaggedComponent))
		{
			return MuzzleComponent;
		}
	}

	return nullptr;
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

	if (const USceneComponent* MuzzleComponent = ResolveMuzzleComponent())
	{
		return MuzzleComponent->GetComponentTransform();
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

	if (APBBumperProjectile* BumperProjectile = Cast<APBBumperProjectile>(Projectile))
	{
		BumperProjectile->OnProjectileResolved.RemoveAll(this);
		BumperProjectile->ConfigureForTarget(
			AttackTarget.Get(),
			AttackPayload,
			AttackPower,
			false,
			0.0f,
			DeliveryVfx,
			ImpactVfx,
			nullptr,
			ProjectileMesh);
		BumperProjectile->OnProjectileResolved.AddUObject(
			this,
			&UPBTurretFireComponent::HandleBumperProjectileResolved);
	}

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
	if (APBBumperProjectile* BumperProjectile = Cast<APBBumperProjectile>(Projectile))
	{
		BumperProjectile->OnProjectileResolved.RemoveAll(this);
		BumperProjectile->ResetForPool();
	}
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

void UPBTurretFireComponent::HandleBumperProjectileResolved(
	APBBumperProjectile* Projectile,
	const bool bApplied)
{
	UE_LOG(LogTemp, Verbose,
		TEXT("[Bumper] Turret projectile resolved. Owner=%s Projectile=%s Applied=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(Projectile),
		bApplied ? TEXT("true") : TEXT("false"));
	ReleaseProjectile(Projectile);
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
