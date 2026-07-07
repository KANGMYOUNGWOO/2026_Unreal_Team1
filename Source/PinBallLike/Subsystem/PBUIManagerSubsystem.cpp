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

bool UPBUIManagerSubsystem::RequestPopWidget()
{
	CleanInvalidWidgetsFromStack();

	UPBUserWidget* Widget = GetTopWidget();
	if (!IsValid(Widget) || Widget->IsPopRequested())
	{
		return false;
	}

	// 닫기 애니메이션 중 중복 요청이 들어오지 않도록 먼저 상태를 고정한다.
	Widget->SetPopRequested(true);
	Widget->OnPopRequested();

	return true;
}

bool UPBUIManagerSubsystem::CompletePopWidget(UPBUserWidget* Widget)
{
	return RemoveWidgetFromStack(Widget, false);
}

void UPBUIManagerSubsystem::PopAllWidgets()
{
	// 전체 정리는 레벨 전환/강제 종료 용도라 닫기 애니메이션을 기다리지 않는다.
	for (int32 WidgetIndex = WidgetStack.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
	{
		RemoveWidgetFromStack(WidgetStack[WidgetIndex].Get(), true);
	}

	WidgetStack.Reset();
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

bool UPBUIManagerSubsystem::RemoveWidgetFromStack(UPBUserWidget* Widget, const bool bForceRemove)
{
	if (!IsValid(Widget))
	{
		return false;
	}

	const int32 WidgetIndex = WidgetStack.IndexOfByPredicate(
		[Widget](const TObjectPtr<UPBUserWidget>& StackWidget)
		{
			return StackWidget.Get() == Widget;
		});
	if (WidgetIndex == INDEX_NONE)
	{
		return false;
	}

	const bool bIsTopWidget = WidgetIndex == WidgetStack.Num() - 1;
	if (!bForceRemove && !bIsTopWidget && !Widget->IsPopRequested())
	{
		return false;
	}

	WidgetStack.RemoveAt(WidgetIndex);
	Widget->SetPopRequested(false);
	Widget->OnPopped();
	Widget->RemoveFromParent();

	return true;
}

void UPBUIManagerSubsystem::CleanInvalidWidgetsFromStack()
{
	for (int32 WidgetIndex = WidgetStack.Num() - 1; WidgetIndex >= 0; --WidgetIndex)
	{
		if (!IsValid(WidgetStack[WidgetIndex].Get()))
		{
			WidgetStack.RemoveAt(WidgetIndex);
		}
	}
}
