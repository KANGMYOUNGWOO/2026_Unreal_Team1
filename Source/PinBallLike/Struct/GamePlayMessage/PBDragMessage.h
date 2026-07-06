#pragma once

#include "CoreMinimal.h"
#include "PBDragMessage.generated.h"

USTRUCT(BlueprintType)
struct FPBDragStartedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	UObject* SourceObject = nullptr;
};

USTRUCT(BlueprintType)
struct FPBDragMovedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	FVector2D ScreenPosition = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct FPBDragDroppedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	UObject* SourceObject = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UObject* TargetObject = nullptr;
};