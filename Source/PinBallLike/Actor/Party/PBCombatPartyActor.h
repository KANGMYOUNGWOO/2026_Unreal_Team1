// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBCombatPartyActor.generated.h"

class APBBallBase;
class UPBBallDeckSubsystem;
class UPBBallDataAsset;
class UPBSnakeFormationComponent;
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
	bool bLaunchConsumed = false;
	
};
