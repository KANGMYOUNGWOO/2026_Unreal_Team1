#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PBBumperDragDropOperation.generated.h"

UENUM(BlueprintType)
enum class EPBBumperDragResult : uint8
{
	Pending,
	Equipped,
	Moved,
	Swapped,
	Unequipped,
	NoChange,
	Cancelled,
	Failed
};

UCLASS()
class PINBALLLIKE_API UPBBumperDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	void InitializeBumperDrag(FName InBumperRowName);
	void InitializeEquippedBumperDrag(
		FName InBumperRowName,
		EPBBumperEquipSlot InSourceEquipSlot);

	UFUNCTION(BlueprintPure, Category = "Bumper|DragDrop")
	bool IsValidBumperDrag() const;

	UFUNCTION(BlueprintPure, Category = "Bumper|DragDrop")
	bool HasSourceEquipSlot() const { return bHasSourceEquipSlot; }

	UFUNCTION(BlueprintPure, Category = "Bumper|DragDrop")
	EPBBumperDragResult GetResult() const { return Result; }

	UFUNCTION(BlueprintPure, Category = "Bumper|DragDrop")
	bool WasDropHandled() const;

	void SetResult(EPBBumperDragResult InResult);

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|DragDrop")
	FName BumperRowName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|DragDrop")
	EPBBumperEquipSlot SourceEquipSlot = EPBBumperEquipSlot::TopLeft;

private:
	UPROPERTY()
	bool bHasSourceEquipSlot = false;

	UPROPERTY()
	EPBBumperDragResult Result = EPBBumperDragResult::Pending;
};
