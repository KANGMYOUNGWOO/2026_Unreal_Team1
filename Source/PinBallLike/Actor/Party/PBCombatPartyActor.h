// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Actor.h"
#include "PBCombatPartyActor.generated.h"

class APBBallBase;
struct FPBBattlePartyDeploymentStartedMessage;
struct FPBBattlePartyLaunchApprovedMessage;
class UPBBaseResourceComponent;
class UPBBallDeckSubsystem;
class UPBBallDataAsset;
class UPBSnakeFormationComponent;
class UParticleSystem;
class UStaticMeshComponent;

UCLASS()
class PINBALLLIKE_API APBCombatPartyActor : public AActor
{
	GENERATED_BODY()

public:
	APBCombatPartyActor();
	
	void InitializeFromDeck();
	void BindDeckEvents();
	void UnbindDeckEvents();

	UFUNCTION(BlueprintCallable, Category = "Party|Launch")
	bool LaunchPartyFromReadyPosition();

	UFUNCTION(BlueprintCallable, Category = "Party|Launch")
	void PrepareForDeployment();

	UFUNCTION(BlueprintCallable, Category = "Party|Launch")
	void SetLauncherActive(bool bNewLauncherActive);

	UFUNCTION(BlueprintPure, Category = "Party|Launch")
	bool IsLauncherActive() const { return bLauncherActive; }
	
	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);
	UFUNCTION()
	void HandleDeploymentSlotsReordered();
	UFUNCTION()
	void HandleDeploymentSlotsRotated();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	void RegisterBattleMessageListeners();
	void UnregisterBattleMessageListeners();
	void HandlePartyDeploymentStartedMessage(FGameplayTag Channel, const FPBBattlePartyDeploymentStartedMessage& Message);
	void HandlePartyLaunchApprovedMessage(FGameplayTag Channel, const FPBBattlePartyLaunchApprovedMessage& Message);
	void BindPartyBallDeathEvents();
	void UnbindPartyBallDeathEvents();
	void UnbindPartyBallDeathEvent(APBBallBase* Ball);
	void HandlePartyBallResourceCurrentChanged(FName ResourceName, float CurrentValue);
	void HandleDeadPartyBalls();
	void HandleDeadPartyBall(APBBallBase* DeadBall);
	void RemoveDeadPartyBall(APBBallBase* DeadBall);
	void StartLeaderPromotion(APBBallBase* NewLeaderBall, FVector TargetLocation, FVector InheritedVelocity);
	void UpdateLeaderPromotion(float DeltaTime);
	void FinishLeaderPromotion();
	void RebuildPartyRolesFromPartyBalls();
	void SpawnBallDeathEffect(APBBallBase* DeadBall) const;
	bool AreAllPartyBallsDead() const;
	void BroadcastPartyAllBallsDead();
	void RefreshFromDeck();
	void HandlePartyOrderChanged();
	void UpdateLauncherMotion(float DeltaTime);
	void HidePartyBallsForLaunchReady();
	void SpawnPartyBallsAtLauncher();
	void ApplyPartyRoles();
	void ClearPartyRoles();
	void CompactPartyBalls();
	void DestroyPartyBalls();
	APBBallBase* SpawnBallFromInstanceId(int32 BallInstanceId);
	
	UPROPERTY(VisibleAnywhere, Category = "Party|Launch")
	TObjectPtr<UStaticMeshComponent> LauncherVisualComponent;

	UPROPERTY(VisibleAnywhere, Category = "Party")
	TObjectPtr<UPBSnakeFormationComponent> SnakeFormationComponent;

	UPROPERTY(EditAnywhere, Category = "Party|Launch")
	bool bLauncherActive = true;

	UPROPERTY(EditAnywhere, Category = "Party|Launch", meta = (ClampMin = "0", ClampMax = "200"))
	float LauncherMoveHalfRange = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Launch", meta = (ClampMin = "0", ClampMax = "1"))
	float LauncherMoveSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Launch")
	FVector LauncherMoveLocalAxis = FVector::RightVector;

	UPROPERTY(EditAnywhere, Category = "Party|Launch", meta = (ClampMin = "0"))
	float ReadyBallSpacing = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Death")
	TObjectPtr<UParticleSystem> BallDeathEffect = nullptr;

	UPROPERTY(EditAnywhere, Category = "Party|Death", meta = (ClampMin = "0.01"))
	float LeaderPromotionDuration = 0.25f;
	
	UPROPERTY()
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;
	UPROPERTY()
	TArray<TObjectPtr<APBBallBase>> PartyBalls;
	UPROPERTY()
	TObjectPtr<APBBallBase> LeaderBall;
	UPROPERTY()
	TArray<TObjectPtr<APBBallBase>> FollowerBalls;

	FVector LauncherBaseLocation = FVector::ZeroVector;
	float LauncherElapsedTime = 0.0f;
	bool bAllBallsDeadBroadcasted = false;
	bool bLeaderPromotionInProgress = false;
	float LeaderPromotionElapsedTime = 0.0f;
	FVector LeaderPromotionStartLocation = FVector::ZeroVector;
	FVector LeaderPromotionTargetLocation = FVector::ZeroVector;
	FVector LeaderPromotionInheritedVelocity = FVector::ZeroVector;
	TObjectPtr<APBBallBase> PromotingLeaderBall = nullptr;

	FGameplayMessageListenerHandle PartyDeploymentStartedListenerHandle;
	FGameplayMessageListenerHandle PartyLaunchApprovedListenerHandle;
	
};
