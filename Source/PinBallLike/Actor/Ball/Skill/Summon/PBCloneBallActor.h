// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBCloneBallActor.generated.h"

class APBBallBase;
class UPBBaseStatComponent;
class UPBBallHitReactionComponent;
class UPBBallPhysicsComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBCloneBallActor : public AActor
{
	GENERATED_BODY()

public:
	APBCloneBallActor();

	UFUNCTION(BlueprintCallable, Category = "Ball|Summon")
	void InitializeFromSourceBall(APBBallBase* InSourceBall);

	UFUNCTION(BlueprintCallable, Category = "Ball|Summon")
	void SetMaxHitCount(int32 InMaxHitCount);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Summon|Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Summon|Stat")
	TObjectPtr<UPBBaseStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Summon|Physics")
	TObjectPtr<UPBBallPhysicsComponent> PhysicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball|Summon|Damage")
	TObjectPtr<UPBBallHitReactionComponent> HitReactionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Summon", meta = (ClampMin = "0.0"))
	float LifeTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ball|Summon", meta = (ClampMin = "1"))
	int32 MaxHitCount = 5;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ball|Summon")
	int32 CurrentHitCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "Ball|Summon", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<APBBallBase> SourceBall;

private:
	UFUNCTION()
	void HandleMovementHit(const FHitResult& Hit);

	void CopyStat(const UPBBaseStatComponent* SourceStatComponent, FName StatName);
};
