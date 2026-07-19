#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionMessage.h"

class PINBALLLIKE_API FPBCollectionNotificationRouter
{
public:
	static FText BuildNotificationText(const FPBCollectionNotificationMessage& Message);
	static bool Broadcast(UObject* WorldContextObject, const FPBCollectionNotificationMessage& Message);
};
