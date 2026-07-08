#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionMessage.h"

/**
 * 도감 알림 메시지를 팀 공용 GameplayMessage 채널로 전달하는 작은 라우터입니다.
 * 도감 Subsystem이 알림 위젯을 직접 만들지 않도록 책임을 분리합니다.
 */
class PINBALLLIKE_API FPBCollectionNotificationRouter
{
public:
	static FText BuildNotificationText(const FPBCollectionNotificationMessage& Message);
	static bool Broadcast(UObject* WorldContextObject, const FPBCollectionNotificationMessage& Message);
};
