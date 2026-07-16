#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PBTurtleBoss.generated.h"

class APBBossMoveArea;
class UAnimationAsset;
class UBoxComponent;
class USkeletalMeshComponent;

UCLASS()
class PINBALLLIKE_API APBTurtleBoss : public APBBossBase
{
	GENERATED_BODY()

public:
	APBTurtleBoss();

	UFUNCTION(BlueprintPure, Category = "Boss|Turtle|Falling Rock")
	FVector GetRandomFallingRockLocation(float SpawnHeight = 0.0f) const;

	void PlayTurtleAnimation(UAnimationAsset* Animation);
	void RestoreTurtleAnimationMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void StartIdleState_Implementation() override;

	void InitializeMoveArea();
	void SelectNextMoveTarget();
	void MoveToTarget(float DeltaTime);
	APBBossMoveArea* FindNearestMoveArea() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Turtle")
	TObjectPtr<USkeletalMeshComponent> TurtleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Turtle|Falling Rock")
	TObjectPtr<UBoxComponent> FallingRockArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle")
	TObjectPtr<APBBossMoveArea> BossMoveArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle", meta = (ClampMin = "0.0"))
	float MoveSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle", meta = (ClampMin = "1.0"))
	float MoveAcceptanceRadius = 50.0f;

private:
	FVector MoveTargetLocation = FVector::ZeroVector;
	bool IsMoveTargetValid = false;
};
