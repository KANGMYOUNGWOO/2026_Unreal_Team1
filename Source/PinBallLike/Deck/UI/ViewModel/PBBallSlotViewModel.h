// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallSlotViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBallSlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "BallDeck|SlotViewModel")
	void SetSlotContext(EPBBallDeckSlotType InSlotType);

	UFUNCTION(BlueprintCallable, Category = "BallDeck|SlotViewModel")
	void SetHasBall(bool bInHasBall);

	UFUNCTION(BlueprintCallable, Category = "BallDeck|SlotViewModel")
	void ClearBall();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|SlotViewModel")
	bool bHasBall = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "BallDeck|SlotViewModel")
	EPBBallDeckSlotType SlotType = EPBBallDeckSlotType::Bench;
};
