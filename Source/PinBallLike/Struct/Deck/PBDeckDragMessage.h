#pragma once

#include "CoreMinimal.h"
#include "PBDeckDragMessage.generated.h"

USTRUCT(BlueprintType)
struct FPBDeckDragStartedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	UObject* SourceObject = nullptr;
};

USTRUCT(BlueprintType)
struct FPBDeckDragEndedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	UObject* SourceObject = nullptr;

	UPROPERTY(BlueprintReadOnly)
	bool bCancelled = false;
};
