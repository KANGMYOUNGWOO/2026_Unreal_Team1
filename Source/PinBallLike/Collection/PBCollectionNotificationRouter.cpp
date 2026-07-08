#include "PBCollectionNotificationRouter.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"

FText FPBCollectionNotificationRouter::BuildNotificationText(const FPBCollectionNotificationMessage& Message)
{
	switch (Message.NewState)
	{
	case EPBCollectionState::Discovered:
		return FText::Format(NSLOCTEXT("PBCollection", "NotificationDiscovered", "도감 발견: {0}"), Message.DisplayName);
	case EPBCollectionState::Unlocked:
		return FText::Format(NSLOCTEXT("PBCollection", "NotificationUnlocked", "도감 해금: {0}"), Message.DisplayName);
	case EPBCollectionState::Completed:
		return Message.Category == EPBCollectionCategory::Achievement
			? FText::Format(NSLOCTEXT("PBCollection", "NotificationAchievementCompleted", "업적 달성: {0}"), Message.DisplayName)
			: FText::Format(NSLOCTEXT("PBCollection", "NotificationCompleted", "도감 완료: {0}"), Message.DisplayName);
	case EPBCollectionState::Locked:
	default:
		return FText::GetEmpty();
	}
}

bool FPBCollectionNotificationRouter::Broadcast(
	UObject* WorldContextObject,
	const FPBCollectionNotificationMessage& Message)
{
	if (!IsValid(WorldContextObject) || Message.MessageText.IsEmpty())
	{
		return false;
	}

	if (!UGameplayMessageSubsystem::HasInstance(WorldContextObject))
	{
		return false;
	}

	UGameplayMessageSubsystem::Get(WorldContextObject).BroadcastMessage(
		GameplayTags::Event_UI_Collection_Notification,
		Message);
	return true;
}
