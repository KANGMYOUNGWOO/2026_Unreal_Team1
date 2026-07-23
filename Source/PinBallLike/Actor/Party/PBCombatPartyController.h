// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBCombatPartyController.generated.h"

class APBBallBase;
class UPBLeaderPromotionComponent;
class UPBPartyBattleMessageComponent;
class UPBPartyDeathComponent;
class UPBPartyDeploymentComponent;
class UPBPartyLauncherComponent;
class UPBSnakeFormationComponent;
class UPBBallVoiceDataAsset;
class UParticleSystem;
class UPBRelicCalculator;
class USoundBase;

UCLASS()
class PINBALLLIKE_API APBCombatPartyController : public AActor
{
	GENERATED_BODY()

public:
	APBCombatPartyController();
	
	void InitializeFromDeck();

	UFUNCTION(BlueprintCallable, Category = "Party|Launch")
	bool LaunchPartyFromReadyPosition();

	UFUNCTION(BlueprintCallable, Category = "Party|Launch")
	void PrepareForDeployment();

	UFUNCTION(BlueprintCallable, Category = "Party|Launch")
	void SetLauncherActive(bool bNewLauncherActive);

	UFUNCTION(BlueprintPure, Category = "Party|Launch")
	bool IsLauncherActive() const;

	const TArray<TObjectPtr<APBBallBase>>& GetPartyBalls() const { return PartyBalls; }
	APBBallBase* GetLeaderBall() const { return LeaderBall.Get(); }
	void RemovePartyBall(APBBallBase* Ball);
	void RebuildPartyRoles();
	void ApplyPartyRoles();
	void ClearPartyRoles();
	bool ContainsPartyBall(APBBallBase* Ball) const;
	bool IsPartyEmpty() const { return PartyBalls.IsEmpty(); }
	TArray<APBBallBase*> GetValidPartyBalls() const;
	void RefreshPartyOrder();
	void RequestUseSkill(int32 SkillInputValue);
	void RequestDashToBoss();
	void SpawnBallDeathEffect(APBBallBase* DeadBall) const;
	void BroadcastPartyAllBallsDead();
	void SetPartyBalls(const TArray<TObjectPtr<APBBallBase>>& InPartyBalls);
	void DestroyPartyBalls();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	APBBallBase* FindPartyBallByInstanceId(int32 BallInstanceId) const;
	USoundBase* ResolveBallDeathSound(const APBBallBase* DeadBall) const;

	UPROPERTY(VisibleAnywhere, Category = "Party|Launch")
	TObjectPtr<UPBPartyLauncherComponent> PartyLauncherComponent;

	UPROPERTY(VisibleAnywhere, Category = "Party")
	TObjectPtr<UPBPartyBattleMessageComponent> PartyBattleMessageComponent;

	UPROPERTY(VisibleAnywhere, Category = "Party")
	TObjectPtr<UPBPartyDeathComponent> PartyDeathComponent;

	UPROPERTY(VisibleAnywhere, Category = "Party")
	TObjectPtr<UPBPartyDeploymentComponent> PartyDeploymentComponent;

	UPROPERTY(VisibleAnywhere, Category = "Party")
	TObjectPtr<UPBSnakeFormationComponent> SnakeFormationComponent;

	UPROPERTY(VisibleAnywhere, Category = "Party")
	TObjectPtr<UPBLeaderPromotionComponent> LeaderPromotionComponent;
	
	UPROPERTY(EditAnywhere, Category = "Party|Death")
	TObjectPtr<UParticleSystem> BallDeathEffect = nullptr;

	UPROPERTY(EditAnywhere, Category = "Party|Death")
	TObjectPtr<UPBBallVoiceDataAsset> BallVoiceData = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBallBase>> PartyBalls;

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> LeaderBall;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBallBase>> FollowerBalls;
	
	
	void BindRelicEvents();
	void UnbindRelicEvents();
	void HandleRelicsChanged();
	void RefreshPartyRelicStats();
	void ApplyActiveSynergyEffects();
};
