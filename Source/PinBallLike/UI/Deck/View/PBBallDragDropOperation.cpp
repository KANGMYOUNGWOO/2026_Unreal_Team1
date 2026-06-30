// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDragDropOperation.h"

void UPBBallDragDropOperation::InitializeBallDrag(int32 InBallInstanceId, EPBBallDeckSlotType InSourceSlotType, int32 InSourceSlotIndex)
{
	BallInstanceId = InBallInstanceId;
	SourceSlotType = InSourceSlotType;
	SourceSlotIndex = InSourceSlotIndex;
}

bool UPBBallDragDropOperation::IsValidBallDrag() const
{
	return BallInstanceId != INDEX_NONE && SourceSlotIndex != INDEX_NONE;
}
