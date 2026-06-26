// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBCombatPartyActor.generated.h"

class APBBallBase;
class UPBBallDeckSubsystem;
class UPBSnakeFormationComponent;

UCLASS()
class PINBALLLIKE_API APBCombatPartyActor : public AActor
{
	GENERATED_BODY()

public:
	APBCombatPartyActor();
	
	void InitializeFromDeck();
	void BindDeckEvents();
	void UnbindDeckEvents();
	
	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, APBBallBase* Ball);
	UFUNCTION()
	void HandleDeploymentSlotsReordered();
	UFUNCTION()
	void HandleDeploymentSlotsRotated();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	void RefreshFromDeck();
	void HandlePartyOrderChanged();
	void ApplyPartyRoles();
	void ClearPartyRoles();
	void CompactPartyBalls();
	
	UPROPERTY(VisibleAnywhere, Category = "Party")
	TObjectPtr<UPBSnakeFormationComponent> SnakeFormationComponent;

	UPROPERTY()
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;
	UPROPERTY()
	TArray<TObjectPtr<APBBallBase>> PartyBalls;
	UPROPERTY()
	TObjectPtr<APBBallBase> LeaderBall;
	UPROPERTY()
	TArray<TObjectPtr<APBBallBase>> FollowerBalls;

	
	
};
