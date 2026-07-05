// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDragDropOperation.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Deck/PBDeckDragMessage.h"

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

void UPBBallDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	Super::Drop_Implementation(PointerEvent);
	BroadcastDragEnded(false);
}

void UPBBallDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	Super::DragCancelled_Implementation(PointerEvent);
	BroadcastDragEnded(true);
}

void UPBBallDragDropOperation::BroadcastDragEnded(bool bCancelled)
{
	if (!IsValidBallDrag())
	{
		return;
	}

	FPBDeckDragEndedMessage Message;
	Message.ItemId = BallInstanceId;
	Message.SourceObject = Payload;
	Message.bCancelled = bCancelled;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Deck_Drag_Ended,
		Message);
}
