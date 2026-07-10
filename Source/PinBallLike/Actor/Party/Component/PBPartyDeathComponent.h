// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBPartyDeathComponent.generated.h"

class APBBallBase;

struct FPBPartyDeathDependencies
{
	TFunction<const TArray<TObjectPtr<APBBallBase>>&()> GetPartyBalls;
	TFunction<bool()> IsLauncherActive;
	TFunction<bool(APBBallBase*)> ContainsPartyBall;
	TFunction<APBBallBase*()> GetLeaderBall;
	TFunction<void(APBBallBase*)> RemovePartyBall;
	TFunction<bool()> IsPartyEmpty;
	TFunction<void()> RefreshPartyOrder;
	TFunction<void(APBBallBase*, FVector, FVector)> StartLeaderPromotion;
	TFunction<void()> StopLeaderPromotion;
	TFunction<void(APBBallBase*)> SpawnBallDeathEffect;
	TFunction<void()> BroadcastPartyAllBallsDead;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBPartyDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBPartyDeathComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void InitializeDependencies(FPBPartyDeathDependencies InDependencies);
	void BindPartyBalls();
	void UnbindPartyBalls();
	void UnbindPartyBall(APBBallBase* Ball);
	void ResetPartyDeathState();
	void DetectDeadPartyBalls();

private:
	const TArray<TObjectPtr<APBBallBase>>& GetPartyBalls() const;
	void HandleDeadPartyBall(APBBallBase* DeadBall);
	void HandlePartyBallResourceCurrentChanged(FName ResourceName, float CurrentValue);

	FPBPartyDeathDependencies Dependencies;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBallBase>> ObservedPartyBalls;

	TArray<TObjectPtr<APBBallBase>> EmptyPartyBalls;
	bool bAllBallsDeadBroadcasted = false;
};
