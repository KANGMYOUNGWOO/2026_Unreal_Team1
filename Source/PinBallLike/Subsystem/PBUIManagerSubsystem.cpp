#include "PBUIManagerSubsystem.h"

#include "PinBallLike/UI/PBUserWidget.h"

UPBUserWidget* UPBUIManagerSubsystem::PushWidget(
	const TSubclassOf<UPBUserWidget> WidgetClass,
	const int32 ZOrder)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	UPBUserWidget* Widget = CreateWidget<UPBUserWidget>(GameInstance, WidgetClass);
	if (!IsValid(Widget))
	{
		return nullptr;
	}

	Widget->AddToViewport(ZOrder);
	WidgetStack.Add(Widget);
	Widget->OnPushed();

	return Widget;
}

bool UPBUIManagerSubsystem::PopWidget()
{
	while (!WidgetStack.IsEmpty())
	{
		UPBUserWidget* Widget = WidgetStack.Pop();
		if (!IsValid(Widget))
		{
			continue;
		}

		Widget->OnPopped();
		Widget->RemoveFromParent();
		return true;
	}

	return false;
}

void UPBUIManagerSubsystem::PopAllWidgets()
{
	while (PopWidget())
	{
	}
}

UPBUserWidget* UPBUIManagerSubsystem::GetTopWidget() const
{
	for (int32 WidgetIndex = WidgetStack.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
	{
		UPBUserWidget* Widget = WidgetStack[WidgetIndex].Get();
		if (IsValid(Widget))
		{
			return Widget;
		}
	}

	return nullptr;
}
