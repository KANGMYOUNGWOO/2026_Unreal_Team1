// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PBBallDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UPBBallDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "BallDeck|DragDrop")
	void InitializeBallDrag(int32 InBallInstanceId, EPBBallDeckSlotType InSourceSlotType, int32 InSourceSlotIndex);

	UFUNCTION(BlueprintPure, Category = "BallDeck|DragDrop")
	bool IsValidBallDrag() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|DragDrop", meta = (ExposeOnSpawn = true))
	int32 BallInstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|DragDrop", meta = (ExposeOnSpawn = true))
	EPBBallDeckSlotType SourceSlotType = EPBBallDeckSlotType::Bench;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BallDeck|DragDrop", meta = (ExposeOnSpawn = true))
	int32 SourceSlotIndex = INDEX_NONE;
};
