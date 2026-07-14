#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PBCircularBladeSkill.generated.h"

class APBBallBase;
class APBCircularBladeActor;

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class PINBALLLIKE_API UPBCircularBladeSkill : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Skill|Circular Blade")
	APBCircularBladeActor* Activate(
		APBBallBase* OwnerBall,
		TSubclassOf<APBCircularBladeActor> InBladeActorClass);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill|Circular Blade", meta = (ClampMin = "0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill|Circular Blade", meta = (ClampMin = "1"))
	int32 DamageCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill|Circular Blade", meta = (ClampMin = "0.01"))
	float Duration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Skill|Circular Blade")
	TSubclassOf<APBCircularBladeActor> BladeActorClass;

	UFUNCTION(BlueprintNativeEvent, Category = "Ball|Skill|Circular Blade")
	int32 GetBallAttackPower(const APBBallBase* OwnerBall) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Ball|Skill|Circular Blade")
	int32 CalculateFinalDamage(int32 BallAttackPower) const;
};
