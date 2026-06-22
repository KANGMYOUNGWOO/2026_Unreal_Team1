// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PinBallController.generated.h"

class APinBallPlayer;

UCLASS()
class PINBALLLIKE_API APinBallController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<APinBallPlayer> PinBallPlayer;
};
