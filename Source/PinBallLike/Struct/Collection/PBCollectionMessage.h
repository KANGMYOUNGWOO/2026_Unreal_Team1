#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionMessage.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBCollectionNotificationMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Notification")
	FName CollectionId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Notification")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Notification")
	EPBCollectionState PreviousState = EPBCollectionState::Locked;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Notification")
	EPBCollectionState NewState = EPBCollectionState::Locked;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Notification")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Notification")
	FText MessageText;
};
