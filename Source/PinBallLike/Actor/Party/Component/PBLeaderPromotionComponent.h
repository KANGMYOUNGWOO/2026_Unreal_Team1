// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBLeaderPromotionComponent.generated.h"

class APBBallBase;
class UPBSnakeFormationComponent;

struct FPBLeaderPromotionDependencies
{
	TFunction<void()> RebuildPartyRoles;
	TFunction<void()> ClearPartyRoles;
	TFunction<void()> ApplyPartyRoles;
	TFunction<TArray<APBBallBase*>()> GetValidPartyBalls;
};

DECLARE_MULTICAST_DELEGATE(FPBLeaderPromotionFinished);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBLeaderPromotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBLeaderPromotionComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeDependencies(FPBLeaderPromotionDependencies InDependencies, UPBSnakeFormationComponent* InSnakeFormationComponent);
	void StartLeaderPromotion(APBBallBase* NewLeaderBall, FVector TargetLocation, FVector InheritedVelocity);
	void StopPromotion();
	bool IsPromotionInProgress() const { return bPromotionInProgress; }

	FPBLeaderPromotionFinished OnPromotionFinished;

private:
	void FinishPromotion();

	FPBLeaderPromotionDependencies Dependencies;

	UPROPERTY(Transient)
	TObjectPtr<UPBSnakeFormationComponent> SnakeFormationComponent;

	UPROPERTY(EditAnywhere, Category = "Party|Death", meta = (ClampMin = "0.01"))
	float PromotionDuration = 0.25f;

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> PromotingLeaderBall = nullptr;

	bool bPromotionInProgress = false;
	float PromotionElapsedTime = 0.0f;
	FVector PromotionStartLocation = FVector::ZeroVector;
	FVector PromotionTargetLocation = FVector::ZeroVector;
	FVector InheritedVelocity = FVector::ZeroVector;
};
