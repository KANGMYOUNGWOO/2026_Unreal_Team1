#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "PBBossPatternTelegraph.h"
#include "UObject/ObjectKey.h"
#include "PBBossChargeTelegraph.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBBossChargeTelegraphFinishedSignature, FVector, TargetLocation, FVector, Direction);

UCLASS(Blueprintable, BlueprintType)
class PINBALLLIKE_API APBBossChargeTelegraph : public APBBossPatternTelegraph
{
	GENERATED_BODY()

public:
	APBBossChargeTelegraph();

	virtual void Tick(float DeltaSeconds) override;
	virtual void InitTelegraph(float InDurationSeconds, const FVector& InScale) override;
	virtual void DestroyTelegraph() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	void InitChargeTelegraph(
		float InDurationSeconds,
		const FVector& StartLocation,
		const FVector& Direction,
		float Length,
		const FVector& InScale);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	void UpdateChargeTelegraphTransform(const FVector& StartLocation, const FVector& Direction, float Length);

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern|Telegraph")
	FVector GetCurrentTargetLocation() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern|Telegraph")
	FVector GetCurrentDirection() const;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern|Telegraph")
	FPBBossChargeTelegraphFinishedSignature OnChargeTelegraphFinished;

private:
	UFUNCTION()
	void HandleTelegraphDurationFinished();

	void UpdateTrackedPinballTransform();
	void UpdateVisualComponentOffsets(float Length);
	AActor* FindPinballActor() const;
	FVector CalculateDirectionToTarget(const FVector& TargetLocation) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern|Telegraph", meta = (AllowPrivateAccess = "true"))
	bool IsVisualOffsetToPathCenter = true;

	FVector ChargeStartLocation = FVector::ZeroVector;
	FVector CurrentTargetLocation = FVector::ZeroVector;
	FVector CurrentDirection = FVector::ForwardVector;
	float CurrentLength = 0.0f;
	FTimerHandle TelegraphDurationTimerHandle;
	TMap<TObjectKey<USceneComponent>, FVector> InitialRelativeLocationMap;
};
