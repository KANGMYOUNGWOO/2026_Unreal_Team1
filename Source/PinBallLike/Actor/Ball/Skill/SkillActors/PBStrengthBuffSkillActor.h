#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBStrengthBuffSkillActor.generated.h"

class APBCombatPartyController;
class UPBStatusEffectComponent;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBStrengthBuffSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBStrengthBuffSkillActor();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void EnterActiveState() override;
	virtual void EnterFinishingState() override;
	virtual void EnterStoppingState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Skill|Party Buff")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Skill|Party Buff")
	FName StatusEffectId = NAME_None;

private:
	APBCombatPartyController* GetPartyController() const;
	bool ApplyBuffToParty();
	void RemoveAppliedBuffs();
	void UnbindStatusEffectEvents();
	void RemoveInactiveComponents();

	UFUNCTION()
	void HandleStatusEffectRemoved(FName RemovedStatusEffectId, int32 StackCount);

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UPBStatusEffectComponent>> AppliedStatusEffectComponents;
};
