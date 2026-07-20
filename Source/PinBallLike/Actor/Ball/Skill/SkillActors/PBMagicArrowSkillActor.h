#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PBMagicArrowSkillActor.generated.h"

class APBBallBase;
class UPBBallSkillProjectileFireComponent;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBMagicArrowSkillActor : public APBBallSkillActorBase
{
	GENERATED_BODY()

public:
	APBMagicArrowSkillActor();

	virtual void InitializeSkill(
		APBBallBase* InOwnerBall,
		const FPBBallSkillTableRow& InSkillData) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	AActor* FireArrow();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void CompleteFiring();

protected:
	virtual void BeginPlay() override;
	virtual void EnterActiveState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UPBBallSkillProjectileFireComponent> ProjectileFireComponent;

private:
	UFUNCTION()
	void HandleProjectileActivated(AActor* Projectile);

	UFUNCTION()
	void HandleProjectileDeactivated(AActor* Projectile);

	void TryFinishSkill();
	void HandleProjectileFinished();

	int32 ActiveProjectileCount = 0;
	bool bFiringCompleted = false;
};
