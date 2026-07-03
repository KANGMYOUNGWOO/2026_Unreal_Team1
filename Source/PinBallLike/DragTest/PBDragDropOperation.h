#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PBDragDropOperation.generated.h"

UCLASS()
class PINBALLLIKE_API UPBDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> SourceObject = nullptr;
};