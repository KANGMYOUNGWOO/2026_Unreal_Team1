#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "Components/ActorComponent.h"
#include "PBBallSkillComponent.generated.h"

class APBBallBase;
class APBBallSkillActorBase;
class AActor;
struct FPBBallInstanceData;

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBallSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBallSkillComponent();
	bool InitializeSkill(const FPBBallInstanceData& BallInstanceData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill")
	bool TryActivateSkill();

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill")
	APBBallSkillActorBase* ActivateSkillActor();

	UFUNCTION(BlueprintPure, Category = "Ball|Skill")
	APBBallSkillActorBase* GetActiveSkillActor() const { return ActiveSkillActor; }

private:
	UFUNCTION()
	void HandleActiveSkillActorDestroyed(AActor* DestroyedActor);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Skill", meta = (AllowPrivateAccess = "true"))
	FPBBallSkillTableRow SkillData;

	UPROPERTY(Transient)
	TObjectPtr<APBBallSkillActorBase> ActiveSkillActor;

	UPROPERTY(Transient)
	TSubclassOf<APBBallSkillActorBase> SkillActorClass;
};
