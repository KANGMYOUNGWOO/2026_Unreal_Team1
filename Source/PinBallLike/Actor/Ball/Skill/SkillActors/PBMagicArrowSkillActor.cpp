#include "PBMagicArrowSkillActor.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBBallSkillProjectileFireComponent.h"
#include "TimerManager.h"

APBMagicArrowSkillActor::APBMagicArrowSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ProjectileFireComponent = CreateDefaultSubobject<UPBBallSkillProjectileFireComponent>(
		TEXT("ProjectileFireComponent"));
}

void APBMagicArrowSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);
	SetActorRotation(FRotator::ZeroRotator);

	ProjectileFireComponent->InitializeProjectileFire(
		OwnerBall,
		GetSkillDamageAmount(),
		GetSkillGroggyAmount());
}

AActor* APBMagicArrowSkillActor::FireArrow()
{
	return ProjectileFireComponent->FireOnce(FVector::ForwardVector);
}

void APBMagicArrowSkillActor::CompleteFiring()
{
	bFiringCompleted = true;
	TryFinishSkill();
}

void APBMagicArrowSkillActor::BeginPlay()
{
	ProjectileFireComponent->OnProjectileActivated.AddUniqueDynamic(
		this,
		&APBMagicArrowSkillActor::HandleProjectileActivated);
	ProjectileFireComponent->OnProjectileDeactivated.AddUniqueDynamic(
		this,
		&APBMagicArrowSkillActor::HandleProjectileDeactivated);

	Super::BeginPlay();
}

void APBMagicArrowSkillActor::EnterActiveState()
{
	ActiveProjectileCount = 0;
	bFiringCompleted = false;
	Super::EnterActiveState();
}

void APBMagicArrowSkillActor::HandleProjectileActivated(AActor*)
{
	++ActiveProjectileCount;
}

void APBMagicArrowSkillActor::HandleProjectileDeactivated(
	AActor*)
{
	ActiveProjectileCount = FMath::Max(ActiveProjectileCount - 1, 0);
	TryFinishSkill();
}

void APBMagicArrowSkillActor::TryFinishSkill()
{
	if (!bFiringCompleted || ActiveProjectileCount > 0)
	{
		return;
	}

	GetWorldTimerManager().SetTimerForNextTick(
		this,
		&APBMagicArrowSkillActor::HandleProjectileFinished);
}

void APBMagicArrowSkillActor::HandleProjectileFinished()
{
	FinishSkill();
}
