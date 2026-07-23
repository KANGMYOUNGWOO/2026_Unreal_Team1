#include "PBCircularBladeActor.h"

#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBTimedAreaDamageComponent.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"

APBCircularBladeActor::APBCircularBladeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
	AttackSphere->SetupAttachment(Root);
	AttackSphere->InitSphereRadius(100.0f);
	AttackSphere->SetCollisionProfileName(SkillCollisionProfileName);
	AttackSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackSphere->SetGenerateOverlapEvents(true);

	TimedAreaDamageComponent = CreateDefaultSubobject<UPBTimedAreaDamageComponent>(TEXT("TimedAreaDamageComponent"));
}
void APBCircularBladeActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);

	if (TimedAreaDamageComponent)
	{
		BindDamageEvents(TimedAreaDamageComponent);
		TimedAreaDamageComponent->SetGroggyAmount(GetSkillGroggyAmount());
		TimedAreaDamageComponent->InitializeDamageArea(
			AttackSphere,
			OwnerBall);
		TimedAreaDamageComponent->ConfigureDamage(
			GetSkillDamageAmount(),
			InSkillData.LifeValue,
			InSkillData.EffectValue);
	}
}

void APBCircularBladeActor::EnterActiveState()
{
	if (!TimedAreaDamageComponent || !TimedAreaDamageComponent->ActivateEffect())
	{
		StopSkill();
		return;
	}

	LeaderPhysicsComponent = ResolveLeaderPhysicsComponent();
	TargetActor = FindTarget();
	ApplyTargetAcceleration();
	Super::EnterActiveState();
}

void APBCircularBladeActor::EnterFinishingState()
{
	DeactivateBlade();
	Super::EnterFinishingState();
}

void APBCircularBladeActor::EnterStoppingState()
{
	DeactivateBlade();
	Super::EnterStoppingState();
}

void APBCircularBladeActor::DeactivateBlade()
{
	if (TimedAreaDamageComponent)
	{
		TimedAreaDamageComponent->DeactivateEffect();
	}

	TargetActor.Reset();
	LeaderPhysicsComponent.Reset();
}

void APBCircularBladeActor::ApplyTargetAcceleration()
{
	const AActor* Target = TargetActor.Get();
	if (!IsValid(OwnerBall) || !IsValid(Target) || !LeaderPhysicsComponent.IsValid())
	{
		return;
	}

	const FVector Direction = (Target->GetActorLocation() - OwnerBall->GetActorLocation()).GetSafeNormal2D();
	LeaderPhysicsComponent->AddVelocity(Direction * TargetAcceleration);
}

APBBallBase* APBCircularBladeActor::ResolveLeaderBall() const
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

UPBBallPhysicsComponent* APBCircularBladeActor::ResolveLeaderPhysicsComponent() const
{
	APBBallBase* LeaderBall = ResolveLeaderBall();
	return IsValid(LeaderBall)
		? LeaderBall->FindComponentByClass<UPBBallPhysicsComponent>()
		: nullptr;
}
