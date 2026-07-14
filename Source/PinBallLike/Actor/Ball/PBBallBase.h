// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Ball/PBBallInstanceData.h"
#include "PinBallLike/Struct/Party/PBPartyTypes.h"
#include "PBBallBase.generated.h"

class UPBBaseStatComponent;
class UPBBaseResourceComponent;
class UPBBallComboComponent;
class UPBBallHitReactionComponent;
class UPBBallPhysicsComponent;
class UPBBallSkillComponent;
class APBBallSkillActorBase;
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

	void InitializeFromBallInstanceData(const FPBBallInstanceData& InBallInstanceData);
	void InitializeSkillActorClass(TSubclassOf<APBBallSkillActorBase> InSkillActorClass);

	UFUNCTION(BlueprintCallable, Category = "Ball|Combat")
	void SetCombatRole(EPBBallPartyRole NewCombatRole);

	UFUNCTION(BlueprintPure, Category = "Ball|Combat")
	EPBBallPartyRole GetCombatRole() const { return CombatRole; }

	UFUNCTION(BlueprintPure, Category = "Ball|Data")
	int32 GetBallInstanceId() const { return BallInstanceData.InstanceId; }

	UFUNCTION(BlueprintPure, Category = "Ball|Resource")
	UPBBaseResourceComponent* GetResourceComponent() const { return ResourceComponent; }

	UFUNCTION(BlueprintCallable, Category = "Ball|Skill")
	bool TryActivateSkill();
	
protected:
	virtual void BeginPlay() override;
	
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

	UPROPERTY(VisibleAnywhere, Category = "Ball|Skill")
	TObjectPtr<UPBBallSkillComponent> SkillComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ball|Collision")
	TObjectPtr<UPBBallHitReactionComponent> HitReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Combat", meta = (AllowPrivateAccess = "true"))
	EPBBallPartyRole CombatRole = EPBBallPartyRole::None;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Data", meta = (AllowPrivateAccess = "true"))
	FPBBallInstanceData BallInstanceData;
};
