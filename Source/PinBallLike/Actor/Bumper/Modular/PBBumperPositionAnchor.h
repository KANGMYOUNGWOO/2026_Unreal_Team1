// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "PBBumperPositionAnchor.generated.h"

class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperPositionAnchor : public AActor
{
	GENERATED_BODY()

public:
	APBBumperPositionAnchor();

	UFUNCTION(BlueprintPure, Category = "Bumper|Position")
	EPBBumperPositionId GetPositionId() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Position")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Position")
	EPBBumperPositionId PositionId = EPBBumperPositionId::None;
};
