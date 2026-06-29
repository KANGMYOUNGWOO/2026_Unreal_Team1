// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBDeploymentWidget.generated.h"

class UPBBallSlotWidget;
/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBDeploymentWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UPBBallSlotWidget> WBP_BallSlot1;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UPBBallSlotWidget> WBP_BallSlot2;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UPBBallSlotWidget> WBP_BallSlot3;
	
};
