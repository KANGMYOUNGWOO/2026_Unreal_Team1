#include "PBBallSkillProjectileBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBInstantDamageComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"

const FName APBBallSkillProjectileBase::SkillCollisionProfileName(TEXT("Skill"));

APBBallSkillProjectileBase::APBBallSkillProjectileBase()
{
	DamageComponent = CreateDefaultSubobject<UPBInstantDamageComponent>(TEXT("DamageComponent"));

	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->bSweepCollision = true;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	}

	CollisionSphere->SetCollisionProfileName(SkillCollisionProfileName);
}

void APBBallSkillProjectileBase::InitializeProjectile(
	const int32 InDamageAmount,
	const int32 InGroggyAmount)
{
	DamageAmount = FMath::Max(InDamageAmount, 0);
	DamageComponent->SetGroggyAmount(InGroggyAmount);
}

void APBBallSkillProjectileBase::ActivateProjectile()
{
	bIsDeactivating = false;
	bDeactivationCompleted = false;
	Super::ActivateProjectile();
	OnActivated();
}

void APBBallSkillProjectileBase::DeactivateProjectile()
{
	bIsDeactivating = true;
	Super::DeactivateProjectile();
	OnDeactivated();

	if (!bWaitForVisualCompletion)
	{
		CompleteDeactivation();
	}
}

void APBBallSkillProjectileBase::CompleteDeactivation()
{
	if (!bIsDeactivating || bDeactivationCompleted)
	{
		return;
	}

	bIsDeactivating = false;
	bDeactivationCompleted = true;
	OnProjectileDeactivationCompleted.Broadcast(this);
}

void APBBallSkillProjectileBase::HandleProjectileBeginOverlap(
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

	if (!IsValid(OtherActor)
		|| !OtherActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		return;
	}

	if (!DamageComponent->ApplyDamage(OtherActor, DamageAmount))
	{
		return;
	}

	const FVector HitLocation = IsFromSweep
		? FVector(
			SweepResult.ImpactPoint.X,
			SweepResult.ImpactPoint.Y,
			SweepResult.ImpactPoint.Z)
		: GetActorLocation();
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBDamageLogMessage Message;
		Message.Style = EPBDamageLogStyle::PlayerSkill;
		Message.DamageAmount = DamageAmount;
		Message.HitLocation = HitLocation;
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_UI_DamageLog_Requested,
			Message);
	}

	OnHit(OtherActor, DamageAmount, HitLocation);
	OnProjectileDamageApplied.Broadcast(OtherActor, DamageAmount, HitLocation);

	if (bReleaseOnFirstDamage)
	{
		OnProjectileReleaseRequested.Broadcast(this);
	}
}
