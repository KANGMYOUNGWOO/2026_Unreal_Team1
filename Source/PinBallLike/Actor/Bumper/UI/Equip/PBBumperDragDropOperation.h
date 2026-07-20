#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PBBumperDragDropOperation.generated.h"

UCLASS()
class PINBALLLIKE_API UPBBumperDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	void InitializeBumperDrag(FName InBumperRowName);

	UFUNCTION(BlueprintPure, Category = "Bumper|DragDrop")
	bool IsValidBumperDrag() const;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|DragDrop")
	FName BumperRowName = NAME_None;

};
