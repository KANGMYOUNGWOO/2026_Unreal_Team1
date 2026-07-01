// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Common/PBResourceData.h"
#include "PinBallLike/Struct/Common/PBStatData.h"
#include "PinBallLike/Struct/Party/PBPartyTypes.h"
#include "PBBallBase.generated.h"

class UPBBallDataAsset;
class UPBBaseStatComponent;
class UPBBaseResourceComponent;
class UPBBallComboComponent;
class UPBBallHitReactionComponent;
class UPBBallPhysicsComponent;
class USphereComponent;

UCLASS()
class PINBALLLIKE_API APBBallBase : public AActor
{
	GENERATED_BODY()

public:
	APBBallBase();

	UFUNCTION(BlueprintCallable, Category = "Ball|Stat")
	void ApplyStatData(const TArray<FPBStatData>& StatData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Resource")
	void ApplyResourceData(const TArray<FPBResourceData>& ResourceData);

	void InitializeFromBallData(UPBBallDataAsset* InBallData, int32 InStarLevel);

	UFUNCTION(BlueprintCallable, Category = "Ball|Combat")
	void SetCombatRole(EPBBallPartyRole NewCombatRole);

	UFUNCTION(BlueprintPure, Category = "Ball|Combat")
	EPBBallPartyRole GetCombatRole() const { return CombatRole; }
	
protected:
	virtual void BeginPlay() override;
	
private:
	void InitializeStatsFromBallData();
	void InitializeResourcesFromBallData();

	UPROPERTY(VisibleAnywhere, Category = "Ball|Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Ball|Stat")
	TObjectPtr<UPBBaseStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Resource")
	TObjectPtr<UPBBaseResourceComponent> ResourceComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Combo")
	TObjectPtr<UPBBallComboComponent> ComboComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Collision")
	TObjectPtr<UPBBallHitReactionComponent> HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Combat", meta = (AllowPrivateAccess = "true"))
	EPBBallPartyRole CombatRole = EPBBallPartyRole::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPBBallDataAsset> BallData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Progression", meta = (AllowPrivateAccess = "true", ClampMin = "1", UIMin = "1"))
	int32 CurrentStarLevel = 1;
};
