#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PBGolemBossTypes.h"
#include "PBGolemBoss.generated.h"

class APBGolemBossHand;

UCLASS()
class PINBALLLIKE_API APBGolemBoss : public APBBossBase
{
	GENERATED_BODY()

public:
	APBGolemBoss();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem")
	void SpawnGolemHands();

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem")
	void MoveHandToOffset(EPBGolemBossHandType HandType, FVector TargetOffset, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem")
	void LaunchHandFistAtActor(EPBGolemBossHandType HandType, AActor* TargetActor, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem")
	void LaunchHandFistAtLocation(EPBGolemBossHandType HandType, FVector TargetWorldLocation, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem")
	void ReturnHandToDefaultOffset(EPBGolemBossHandType HandType, float Duration);

	UFUNCTION(BlueprintPure, Category = "Boss|Golem")
	APBGolemBossHand* GetGolemHand(EPBGolemBossHandType HandType) const;

	void HandleGolemHandDestroyed(APBGolemBossHand* DestroyedHand, int32 GroggyAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Animation")
	void ResetGolemIdleAnimationSyncTime();

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Animation")
	float GetGolemIdleAnimationElapsedTime() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Golem Animation")
	float GetSyncedGolemIdleAnimationPosition(float AnimationLength) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void StartIdleState_Implementation() override;
	virtual void StartPatternState() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Golem Animation")
	void BP_OnGolemIdleAnimationSyncRequested(float SyncedPosition);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand")
	TSubclassOf<APBGolemBossHand> LeftHandClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand")
	TSubclassOf<APBGolemBossHand> RightHandClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand")
	FVector LeftHandOffset = FVector(0.0f, -300.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Hand")
	FVector RightHandOffset = FVector(0.0f, 300.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Animation", meta = (ClampMin = "0.1"))
	float GolemIdleAnimationLength = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<APBGolemBossHand> LeftHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Golem Hand")
	TObjectPtr<APBGolemBossHand> RightHand;

private:
	APBGolemBossHand* SpawnGolemHand(EPBGolemBossHandType HandType, FVector HandOffset);
	void RequestIdleAnimationSync();
	void StartHandsAutonomousMove();
	void StopHandsAutonomousMove();
	void DestroyGolemHands();

	float GolemIdleAnimationSyncStartTime = 0.0f;
};
