#include "PBPiercingSkillActor.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBInstantDamageComponent.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Table/StatusEffect/PBStatusEffectAssetIds.h"

APBPiercingSkillActor::APBPiercingSkillActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DamageComponent = CreateDefaultSubobject<UPBInstantDamageComponent>(TEXT("DamageComponent"));
}

void APBPiercingSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);
	DamageComponent->SetGroggyAmount(GetSkillGroggyAmount());
}

void APBPiercingSkillActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateEffectDirection();
}

void APBPiercingSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateAttackCollider();
	UnbindStatusEffectEvent();
	Super::EndPlay(EndPlayReason);
}

void APBPiercingSkillActor::EnterActiveState()
{
	APBBallBase* LeaderBall = ResolveLeaderBall();
	UPBStatusEffectComponent* StatusEffectComponent = IsValid(LeaderBall)
		? LeaderBall->GetStatusEffectComponent()
		: nullptr;
	if (!StatusEffectComponent)
	{
		StopSkill();
		return;
	}

	AttachToActor(LeaderBall, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorLocation(LeaderBall->GetActorLocation());
	if (!ActivateAttackCollider())
	{
		StopSkill();
		return;
	}

	StatusEffectComponent->OnStatusEffectRemoved.AddUniqueDynamic(
		this,
		&APBPiercingSkillActor::HandleStatusEffectRemoved);
	AppliedStatusEffectComponent = StatusEffectComponent;
	PiercingBall = LeaderBall;

	const int32 InitialStackCount = FMath::Max(GetSkillData().EffectValue, 1);
	if (!StatusEffectComponent->ApplyStatusEffectStacks(
		PBStatusEffectAssetIds::StatusEffect::Piercing,
		InitialStackCount))
	{
		DeactivateAttackCollider();
		UnbindStatusEffectEvent();
		StopSkill();
		return;
	}

	DashLeaderBallToBoss();
	SetActorTickEnabled(true);
	Super::EnterActiveState();
}

void APBPiercingSkillActor::EnterFinishingState()
{
	SetActorTickEnabled(false);
	DeactivateAttackCollider();
	UnbindStatusEffectEvent();
	Super::EnterFinishingState();
}

void APBPiercingSkillActor::EnterStoppingState()
{
	SetActorTickEnabled(false);
	DeactivateAttackCollider();
	if (UPBStatusEffectComponent* StatusEffectComponent = AppliedStatusEffectComponent.Get())
	{
		UnbindStatusEffectEvent();
		StatusEffectComponent->RemoveStatusEffect(PBStatusEffectAssetIds::StatusEffect::Piercing);
	}

	Super::EnterStoppingState();
}

APBBallBase* APBPiercingSkillActor::ResolveLeaderBall() const
{
	if (!IsValid(OwnerBall))
	{
		return nullptr;
	}

	if (OwnerBall->GetCombatRole() == EPBBallPartyRole::Leader)
	{
		return OwnerBall;
	}

	const APBCombatPartyController* PartyController =
		Cast<APBCombatPartyController>(OwnerBall->GetOwner());
	return IsValid(PartyController) ? PartyController->GetLeaderBall() : nullptr;
}

UPrimitiveComponent* APBPiercingSkillActor::ResolveAttackCollider() const
{
	const TArray<UActorComponent*> TaggedComponents = GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		AttackColliderTag);
	for (UActorComponent* Component : TaggedComponents)
	{
		if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
		{
			return PrimitiveComponent;
		}
	}

	return nullptr;
}

bool APBPiercingSkillActor::ActivateAttackCollider()
{
	AttackCollider = ResolveAttackCollider();
	if (!AttackCollider || !DamageComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PiercingSkill] Attack collider not found. Actor=%s RequiredComponentTag=%s"),
			*GetNameSafe(this),
			*AttackColliderTag.ToString());
		return false;
	}

	DamagedTargets.Reset();
	AttackCollider->SetCollisionProfileName(SkillCollisionProfileName);
	AttackCollider->SetGenerateOverlapEvents(true);
	AttackCollider->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&APBPiercingSkillActor::HandleAttackOverlap);
	AttackCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TArray<AActor*> InitialOverlappingActors;
	AttackCollider->GetOverlappingActors(InitialOverlappingActors);
	for (AActor* OverlappingActor : InitialOverlappingActors)
	{
		ApplyDamageToTarget(OverlappingActor);
	}

	return true;
}

void APBPiercingSkillActor::DeactivateAttackCollider()
{
	if (AttackCollider)
	{
		AttackCollider->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&APBPiercingSkillActor::HandleAttackOverlap);
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AttackCollider = nullptr;
	DamagedTargets.Reset();
}

void APBPiercingSkillActor::HandleAttackOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	const int32 OtherBodyIndex,
	const bool bFromSweep,
	const FHitResult& SweepResult)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComponent);
	static_cast<void>(OtherBodyIndex);
	static_cast<void>(bFromSweep);
	static_cast<void>(SweepResult);
	ApplyDamageToTarget(OtherActor);
}

void APBPiercingSkillActor::ApplyDamageToTarget(AActor* Target)
{
	const TWeakObjectPtr<AActor> TargetKey(Target);
	if (!IsValid(Target)
		|| !Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass())
		|| DamagedTargets.Contains(TargetKey)
		|| !DamageComponent
		|| !DamageComponent->ApplyDamage(Target, GetSkillDamageAmount()))
	{
		return;
	}

	DamagedTargets.Add(TargetKey);
	HandleDamageApplied(Target, GetSkillDamageAmount(), CalculateHitLocation(Target));
}

FVector APBPiercingSkillActor::CalculateHitLocation(const AActor* Target) const
{
	FVector HitLocation = AttackCollider
		? AttackCollider->GetComponentLocation()
		: GetActorLocation();
	FVector ClosestPoint;
	if (AttackCollider
		&& IsValid(Target)
		&& AttackCollider->GetClosestPointOnCollision(
			Target->GetActorLocation(),
			ClosestPoint) >= 0.0f)
	{
		HitLocation = ClosestPoint;
	}

	return HitLocation;
}

void APBPiercingSkillActor::DashLeaderBallToBoss()
{
	APBBallBase* LeaderBall = PiercingBall.Get();
	if (!IsValid(LeaderBall) || DashSpeed <= 0.0f)
	{
		return;
	}

	AActor* TargetBoss = FindTarget();
	UPBBallPhysicsComponent* PhysicsComponent =
		LeaderBall->FindComponentByClass<UPBBallPhysicsComponent>();
	if (!IsValid(TargetBoss) || !PhysicsComponent)
	{
		return;
	}
	AddTickPrerequisiteComponent(PhysicsComponent);

	FVector DashDirection = TargetBoss->GetActorLocation() - LeaderBall->GetActorLocation();
	DashDirection.Z = 0.0f;
	if (DashDirection.IsNearlyZero())
	{
		return;
	}

	PhysicsComponent->Launch(DashDirection, DashSpeed);
	UpdateEffectDirection();
}

void APBPiercingSkillActor::UpdateEffectDirection()
{
	APBBallBase* LeaderBall = PiercingBall.Get();
	const UPBBallPhysicsComponent* PhysicsComponent = IsValid(LeaderBall)
		? LeaderBall->FindComponentByClass<UPBBallPhysicsComponent>()
		: nullptr;
	if (!PhysicsComponent)
	{
		return;
	}

	FVector MovementDirection = PhysicsComponent->GetVelocity();
	MovementDirection.Z = 0.0f;
	if (MovementDirection.IsNearlyZero())
	{
		return;
	}

	SetActorRotation(MovementDirection.Rotation() + EffectRotationOffset);
}

void APBPiercingSkillActor::UnbindStatusEffectEvent()
{
	if (UPBStatusEffectComponent* StatusEffectComponent = AppliedStatusEffectComponent.Get())
	{
		StatusEffectComponent->OnStatusEffectRemoved.RemoveDynamic(
			this,
			&APBPiercingSkillActor::HandleStatusEffectRemoved);
	}

	AppliedStatusEffectComponent.Reset();
	PiercingBall.Reset();
}

void APBPiercingSkillActor::HandleStatusEffectRemoved(
	const FName RemovedStatusEffectId,
	const int32 StackCount)
{
	static_cast<void>(StackCount);
	if (RemovedStatusEffectId != PBStatusEffectAssetIds::StatusEffect::Piercing)
	{
		return;
	}

	UnbindStatusEffectEvent();
	FinishSkill();
}
