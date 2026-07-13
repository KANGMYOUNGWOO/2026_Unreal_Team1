#pragma once

#include "CoreMinimal.h"
#include "PBGolemBossPatternBase.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossTypes.h"
#include "PBGolemHandSlamPattern.generated.h"

class UPBGolemHandMovementComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBGolemHandSlamPattern : public UPBGolemBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool UsesHand(EPBGolemBossHandType TargetHandType) const override;
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern")
	EPBGolemBossHandType HandType = EPBGolemBossHandType::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern")
	FVector ReadyOffset = FVector(0.0f, 0.0f, 300.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern", meta = (ClampMin = "0"))
	float ReadyDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern", meta = (ClampMin = "0"))
	float SlamDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern", meta = (ClampMin = "0"))
	float SlamHoldDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern", meta = (ClampMin = "0"))
	float ReturnDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Slam Range", meta = (ClampMin = "0"))
	float SlamRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Slam Range")
	bool IsDrawDebugSlamRange = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Slam Range", meta = (ClampMin = "0"))
	float DebugSlamRangeDuration = 1.0f;

private:
	enum class EPBGolemHandSlamPhase : uint8
	{
		None,
		Preparing,
		Slamming,
		Holding,
		Returning
	};

	UFUNCTION()
	void HandleHandMoveFinished();

	void HandleSlamHoldFinished();

	bool TryStartSlam(APBBossBase* Boss);
	bool TryBindHandMoveFinished();
	void UnbindHandMoveFinished();
	void ClearSlamHoldTimer();
	void DrawDebugSlamRange(float Duration) const;
	void ReturnHandToStartTransform(APBGolemBoss* GolemBoss);
	void FinishHandSlamPattern();

	UPROPERTY(Transient)
	TObjectPtr<UPBGolemHandMovementComponent> BoundMovementComponent;

	FVector SlamTargetLocation = FVector::ZeroVector;
	FTransform StartHandTransform = FTransform::Identity;
	EPBGolemHandSlamPhase SlamPhase = EPBGolemHandSlamPhase::None;
	FTimerHandle SlamHoldTimerHandle;
};
