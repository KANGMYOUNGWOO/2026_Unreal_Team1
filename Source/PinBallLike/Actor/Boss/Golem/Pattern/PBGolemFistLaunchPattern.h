#pragma once

#include "CoreMinimal.h"
#include "PBGolemBossPatternBase.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossTypes.h"
#include "PBGolemFistLaunchPattern.generated.h"

class APBBossChargeTelegraph;
class APBBallBase;
class UPBGolemHandMovementComponent;
class UPrimitiveComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBGolemFistLaunchPattern : public UPBGolemBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual bool UsesHand(EPBGolemBossHandType TargetHandType) const override;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern", meta = (ClampMin = "0.0"))
	float PunchDistanceExtension = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Hit", meta = (ClampMin = "0.0"))
	float PunchHitRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Hit")
	float PunchHitZOffset = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Hit", meta = (ClampMin = "0"))
	int32 PunchDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Hit", meta = (ClampMin = "0.0"))
	float PunchBounceVelocity = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Hit|Debug")
	bool IsDrawPunchHitRange = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Golem Pattern|Hit|Debug", meta = (ClampMin = "0.0"))
	float PunchHitRangeLineThickness = 5.0f;

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
	void CreatePunchHitCollision(APBGolemBossHand* GolemHand);
	void DestroyPunchHitCollision();
	void ApplyPunchHit(APBBallBase* Ball);
	void DrawPunchHitRange();

	UFUNCTION()
	void HandlePunchBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(Transient)
	TObjectPtr<UPBGolemHandMovementComponent> BoundMovementComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBossChargeTelegraph>> SpawnedFistChargeTelegraphs;

	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> PunchHitCollision;

	TSet<TObjectKey<APBBallBase>> DamagedBalls;
	FTimerHandle PunchHitRangeTimerHandle;

	FVector FistTargetLocation = FVector::ZeroVector;
	FVector FistTargetDirection = FVector::ForwardVector;
	FTransform StartHandTransform = FTransform::Identity;
	EPBGolemFistLaunchPhase FistLaunchPhase = EPBGolemFistLaunchPhase::None;
};

