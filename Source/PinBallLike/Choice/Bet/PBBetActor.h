// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/PBChoiceNodeAction.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PBBetActor.generated.h"

class UPBBettingWidget;

UCLASS()
class PINBALLLIKE_API APBBetActor : public AActor , public IPBChoiceNodeAction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	void OpenAbility() override;
	
	APBBetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Bet")
	TSubclassOf<UPBBettingWidget> BetWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UPBBettingWidget> BetWidget;
	
	
	void HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message);

	FGameplayMessageListenerHandle ExitStartHandle;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
