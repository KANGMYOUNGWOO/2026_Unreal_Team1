// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include  "PinBallLike/Interface/PBChoiceNodeAction.h"
#include "PBActionBattleActor.generated.h"

UCLASS()
class PINBALLLIKE_API APBActionBattleActor : public AActor , public IPBChoiceNodeAction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APBActionBattleActor();
	
    void OpenAbility() override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
