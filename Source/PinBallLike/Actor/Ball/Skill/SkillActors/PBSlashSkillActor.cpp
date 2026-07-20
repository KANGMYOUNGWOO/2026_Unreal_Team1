#include "PBSlashSkillActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBInstantDamageComponent.h"
#include "PinBallLike/Interface/BossInterface.h"

APBSlashSkillActor::APBSlashSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	Pivot->SetupAttachment(Root);

	AttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackBox"));
	AttackBox->SetupAttachment(Pivot);
	AttackBox->SetBoxExtent(FVector(100.0f, 50.0f, 50.0f));
	AttackBox->SetCollisionProfileName(SkillCollisionProfileName);
	AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackBox->SetGenerateOverlapEvents(true);
	AttackBox->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBSlashSkillActor::HandleAttackOverlap);

	DamageComponent = CreateDefaultSubobject<UPBInstantDamageComponent>(TEXT("DamageComponent"));
}

void APBSlashSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);

	BallPhysicsComponent = IsValid(OwnerBall)
		? OwnerBall->FindComponentByClass<UPBBallPhysicsComponent>()
		: nullptr;
	AttackCount = FMath::Max(InSkillData.EffectValue, 1);
	DamageComponent->SetGroggyAmount(GetSkillGroggyAmount());
}

void APBSlashSkillActor::StartCharge()
{
	EndAttack();

	if (BallPhysicsComponent.IsValid())
	{
		BallPhysicsComponent->AddGravityDisableRequest(this);
		BallPhysicsComponent->StopMovementSmoothly(0.5f);
	}
}

bool APBSlashSkillActor::StartAttack()
{
	RestoreGravity();

	AActor* Target = FindTarget();
	if (!IsValid(OwnerBall) || !BallPhysicsComponent.IsValid() || !IsTargetValid(Target))
	{
		StopSkill();
		return false;
	}

	const FVector Direction =
		(Target->GetActorLocation() - OwnerBall->GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		StopSkill();
		return false;
	}

	DamagedTargets.Reset();
	SetActorRotation(Direction.Rotation());
	AttackBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BallPhysicsComponent->AddVelocity(Direction * DashVelocity);

	TArray<AActor*> OverlappingActors;
	AttackBox->GetOverlappingActors(OverlappingActors);
	for (AActor* OverlappingActor : OverlappingActors)
	{
		ApplyDamageToTarget(OverlappingActor);
	}

	return true;
}

void APBSlashSkillActor::EndAttack()
{
	AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamagedTargets.Reset();
}

void APBSlashSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreGravity();
	Super::EndPlay(EndPlayReason);
}

void APBSlashSkillActor::EnterPreparingState()
{
	StartCharge();
	Super::EnterPreparingState();
}

void APBSlashSkillActor::EnterFinishingState()
{
	RestoreGravity();
	EndAttack();
	Super::EnterFinishingState();
}

void APBSlashSkillActor::EnterStoppingState()
{
	RestoreGravity();
	EndAttack();
	Super::EnterStoppingState();
}

void APBSlashSkillActor::HandleAttackOverlap(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	int32,
	bool,
	const FHitResult&)
{
	ApplyDamageToTarget(OtherActor);
}

void APBSlashSkillActor::ApplyDamageToTarget(AActor* Target)
{
	const TWeakObjectPtr<AActor> TargetKey(Target);
	if (!IsValid(Target)
		|| !Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass())
		|| DamagedTargets.Contains(TargetKey)
		|| !DamageComponent->ApplyDamage(Target, GetSkillDamageAmount()))
	{
		return;
	}

	DamagedTargets.Add(TargetKey);
	HandleDamageApplied(Target, GetSkillDamageAmount(), CalculateHitLocation(Target));
}

FVector APBSlashSkillActor::CalculateHitLocation(const AActor* Target) const
{
	FVector HitLocation = AttackBox->GetComponentLocation();
	FVector ClosestPoint;
	if (IsValid(Target)
		&& AttackBox->GetClosestPointOnCollision(Target->GetActorLocation(), ClosestPoint) >= 0.0f)
	{
		HitLocation = ClosestPoint;
	}

	return HitLocation;
}

void APBSlashSkillActor::RestoreGravity()
{
	if (BallPhysicsComponent.IsValid())
	{
		BallPhysicsComponent->RemoveGravityDisableRequest(this);
	}
}
