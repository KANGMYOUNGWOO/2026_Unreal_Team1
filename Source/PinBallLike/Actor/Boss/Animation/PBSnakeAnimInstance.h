#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PBSnakeAnimInstance.generated.h"

class ASnakeBoss;

UCLASS(Blueprintable, BlueprintType)
class PINBALLLIKE_API UPBSnakeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Animation")
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Animation")
	float TurnAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Animation")
	float MovePhase = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Animation")
	bool IsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Charge")
	bool IsChargePoseActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Charge")
	float ChargePoseAlpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Path")
	TArray<FVector> SnakeSplinePoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Snake Charge")
	TArray<FVector> SnakeChargeHeadSplinePoints;

private:
	void CacheOwnerSnakeBoss();
	void UpdateAnimationDataFromOwner();
	void ResetAnimationData();

	UPROPERTY(Transient)
	TObjectPtr<ASnakeBoss> OwnerSnakeBoss;
};
