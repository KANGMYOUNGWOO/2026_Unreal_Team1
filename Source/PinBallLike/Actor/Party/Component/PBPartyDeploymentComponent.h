// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Struct/Ball/PBBallInstanceData.h"
#include "PBPartyDeploymentComponent.generated.h"

class APBBallBase;
class AActor;
class UPBBallDeckSubsystem;

struct FPBPartyDeploymentDependencies
{
	TFunction<void()> UnbindPartyBalls;
	TFunction<void()> ResetPartyDeathState;
	TFunction<void()> StopLeaderPromotion;
	TFunction<void(bool)> SetLauncherActive;
	TFunction<void()> ClearSnakeFormation;
	TFunction<void()> ClearPartyRoles;
	TFunction<void()> DestroyPartyBalls;
	TFunction<void(const TArray<TObjectPtr<APBBallBase>>&)> SetPartyBalls;
	TFunction<void()> RefreshPartyOrder;
	TFunction<FTransform()> GetSpawnTransform;
	TFunction<AActor*()> GetSpawnOwner;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBPartyDeploymentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBPartyDeploymentComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializeDependencies(FPBPartyDeploymentDependencies InDependencies);
	void PrepareForDeployment();
	void RebuildPartyFromDeployment();

private:
	void CacheDeckSubsystem();
	void ScheduleInitialRebuild();
	void BindDeckEvents();
	void UnbindDeckEvents();
	void ResetCurrentParty();
	TArray<TObjectPtr<APBBallBase>> SpawnDeploymentPartyBalls();
	APBBallBase* SpawnBallFromDeckInstance(
		int32 BallInstanceId,
		const FTransform& SpawnTransform);
	bool BuildBallInstanceData(int32 BallInstanceId, FPBBallInstanceData& OutBallInstanceData) const;

	FPBPartyDeploymentDependencies Dependencies;

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);
	UFUNCTION()
	void HandleDeploymentChanged();

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;
};
