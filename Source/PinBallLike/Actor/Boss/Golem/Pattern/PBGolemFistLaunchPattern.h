#pragma once

#include "CoreMinimal.h"
#include "PBGolemBossPatternBase.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossTypes.h"
#include "PBGolemFistLaunchPattern.generated.h"

class APBBossChargeTelegraph;
class UPBGolemHandMovementComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBGolemFistLaunchPattern : public UPBGolemBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern")
	EPBGolemBossHandType HandType = EPBGolemBossHandType::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern", meta = (ClampMin = "0"))
	float LaunchDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern", meta = (ClampMin = "0"))
	float ReturnDuration = 0.35f;

private:
	enum class EPBGolemFistLaunchPhase : uint8
	{
		None,
		Launching,
		Returning
	};

	UFUNCTION()
	void HandleHandMoveFinished();

	UFUNCTION()
	void HandleFistTelegraphFinished(FVector TargetLocation, FVector Direction);

	void StartAiming(APBBossBase* Boss);
	float GetFistTelegraphDurationSeconds() const;
	void SpawnFistTelegraphs(APBBossBase* Boss);
	void UnbindFistTelegraphs();
	void ClearFistTelegraphs();
	bool TryBindHandMoveFinished();
	void UnbindHandMoveFinished();
	void ReturnHandToStartTransform(APBGolemBoss* GolemBoss);
	void UnlockPatternHand();
	void FinishFistLaunchPattern();

	UPROPERTY(Transient)
	TObjectPtr<UPBGolemHandMovementComponent> BoundMovementComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBossChargeTelegraph>> SpawnedFistChargeTelegraphs;

	FVector FistTargetLocation = FVector::ZeroVector;
	FVector FistTargetDirection = FVector::ForwardVector;
	FTransform StartHandTransform = FTransform::Identity;
	EPBGolemFistLaunchPhase FistLaunchPhase = EPBGolemFistLaunchPhase::None;
};

