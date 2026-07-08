#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionMessage.generated.h"

/**
 * 도감 상태 변경을 UI 알림으로 전달할 때 사용하는 메시지입니다.
 * 실제 알림 위젯은 이 데이터를 받아 화면 표시 방식만 결정합니다.
 */
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
