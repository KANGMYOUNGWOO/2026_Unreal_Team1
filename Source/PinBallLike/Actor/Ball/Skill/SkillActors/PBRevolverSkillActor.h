#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBRevolverSkillActor.generated.h"

class APBBallBase;
class UPBBallSkillProjectileFireComponent;
class USceneComponent;
struct FPBBumperTriggeredMessage;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBRevolverSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBRevolverSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterActiveState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Revolver")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Revolver")
	TObjectPtr<UPBBallSkillProjectileFireComponent> ProjectileFireComponent;

	UFUNCTION(BlueprintImplementableEvent, Category = "Revolver")
	void OnFired(FVector FireDirection);

private:
	void RegisterBumperTriggeredListener();
	void UnregisterBumperTriggeredListener();
	void HandleDurationFinished();
	void HandleBumperTriggeredMessage(
		FGameplayTag Channel,
		const FPBBumperTriggeredMessage& Message);

	FGameplayMessageListenerHandle BumperTriggeredListenerHandle;
	FTimerHandle DurationTimerHandle;
	float SkillDuration = 0.0f;
};
