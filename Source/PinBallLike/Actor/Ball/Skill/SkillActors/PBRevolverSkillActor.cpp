#include "PBRevolverSkillActor.h"

#include "Components/SceneComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/Component/PBBallSkillProjectileFireComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBumperTriggeredMessage.h"
#include "TimerManager.h"

APBRevolverSkillActor::APBRevolverSkillActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ProjectileFireComponent = CreateDefaultSubobject<UPBBallSkillProjectileFireComponent>(
		TEXT("ProjectileFireComponent"));
}

void APBRevolverSkillActor::InitializeSkill(
	APBBallBase* InOwnerBall,
	const FPBBallSkillTableRow& InSkillData)
{
	Super::InitializeSkill(InOwnerBall, InSkillData);

	SkillDuration = FMath::Max(InSkillData.LifeValue, 0.0f);
	ProjectileFireComponent->InitializeProjectileFire(
		OwnerBall,
		GetSkillDamageAmount(),
		GetSkillGroggyAmount());
}

void APBRevolverSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterBumperTriggeredListener();
	GetWorldTimerManager().ClearTimer(DurationTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APBRevolverSkillActor::EnterActiveState()
{
	if (SkillDuration <= 0.0f)
	{
		StopSkill();
		return;
	}

	RegisterBumperTriggeredListener();
	GetWorldTimerManager().SetTimer(
		DurationTimerHandle,
		this,
		&APBRevolverSkillActor::HandleDurationFinished,
		SkillDuration,
		false);

	Super::EnterActiveState();
}

void APBRevolverSkillActor::EnterStoppingState()
{
	UnregisterBumperTriggeredListener();
	GetWorldTimerManager().ClearTimer(DurationTimerHandle);
	Super::EnterStoppingState();
}

void APBRevolverSkillActor::RegisterBumperTriggeredListener()
{
	if (!UGameplayMessageSubsystem::HasInstance(this)
		|| BumperTriggeredListenerHandle.IsValid())
	{
		return;
	}

	BumperTriggeredListenerHandle =
		UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBumperTriggeredMessage>(
			GameplayTags::Event_Battle_Bumper_Triggered,
			this,
			&APBRevolverSkillActor::HandleBumperTriggeredMessage);
}

void APBRevolverSkillActor::UnregisterBumperTriggeredListener()
{
	if (BumperTriggeredListenerHandle.IsValid())
	{
		BumperTriggeredListenerHandle.Unregister();
		BumperTriggeredListenerHandle = FGameplayMessageListenerHandle();
	}
}

void APBRevolverSkillActor::HandleDurationFinished()
{
	UnregisterBumperTriggeredListener();
	FinishSkill();
}

void APBRevolverSkillActor::HandleBumperTriggeredMessage(
	FGameplayTag,
	const FPBBumperTriggeredMessage&)
{
	AActor* Target = FindTarget();
	if (!IsValid(Target))
	{
		return;
	}

	const FVector FireDirection =
		(Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	if (ProjectileFireComponent->FireOnce(FireDirection))
	{
		OnFired(FireDirection);
	}
}
