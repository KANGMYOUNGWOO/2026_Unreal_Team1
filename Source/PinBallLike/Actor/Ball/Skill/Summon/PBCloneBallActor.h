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

protected:
	virtual void BeginPlay() override;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "Ball|Summon", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<APBBallBase> SourceBall;

private:
	void CopyStat(const UPBBaseStatComponent* SourceStatComponent, FName StatName);
};
