#include "PBUIManagerSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/DeveloperSettings/PBUISettings.h"
#include "PinBallLike/UI/Global/PBGlobalToolbarWidget.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PinBallLike/UI/Popup/PBBallRewardPopupWidget.h"
#include "PinBallLike/UI/Popup/PBSimplePopupWidget.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// AddToViewport adds an internal Z-order offset, so values near MAX_int32 can overflow.
	constexpr int32 GlobalToolbarZOrder = 10000;
}

UPBUIManagerSubsystem::UPBUIManagerSubsystem()
{
	static ConstructorHelpers::FClassFinder<UPBSimplePopupWidget> PopupClassFinder(
		TEXT("/Game/Blueprints/UI/Popup/WBP_SimplePopup"));
	if (PopupClassFinder.Succeeded())
	{
		DefaultSimplePopupClass = PopupClassFinder.Class;
	}

}

UPBBallRewardPopupWidget* UPBUIManagerSubsystem::ShowBallRewardPopup(
	const TSubclassOf<UPBBallRewardPopupWidget> PopupClass,
	const FText& Message,
	const FName BallId,
	const int32 StarLevel,
	FPBSimplePopupClosedDelegate ClosedCallback,
	const int32 ZOrder)
{
	if (!PopupClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Ball reward popup class is not assigned."));
		return nullptr;
	}

	UPBBallRewardPopupWidget* Popup = Cast<UPBBallRewardPopupWidget>(
		PushWidget(PopupClass, ZOrder));
	if (!IsValid(Popup))
	{
		return nullptr;
	}

	if (!Popup->InitializeBallRewardPopup(Message, BallId, StarLevel))
	{
		CompletePopWidget(Popup);
		return nullptr;
	}

	Popup->SetClosedCallback(MoveTemp(ClosedCallback));
	return Popup;
}

void UPBUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&UPBUIManagerSubsystem::HandlePostLoadMap);
	RefreshGlobalToolbarVisibility();
}

void UPBUIManagerSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	RemoveGlobalToolbar();
	Super::Deinitialize();
}

UPBUserWidget* UPBUIManagerSubsystem::PushWidget(
	const TSubclassOf<UPBUserWidget> WidgetClass,
	const int32 ZOrder)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	CleanInvalidWidgetsFromStack();
	if (UPBUserWidget* TopWidget = GetTopWidget();
		IsValid(TopWidget) && TopWidget->IsA(WidgetClass))
	{
		return TopWidget;
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

UPBSimplePopupWidget* UPBUIManagerSubsystem::ShowSimplePopup(
	const FText& Message,
	FPBSimplePopupClosedDelegate ClosedCallback,
	const int32 ZOrder)
{
	UPBSimplePopupWidget* Popup = PushSimplePopup(DefaultSimplePopupClass, Message, ZOrder);
	if (IsValid(Popup))
	{
		Popup->SetClosedCallback(MoveTemp(ClosedCallback));
	}

	return Popup;
}

UPBSimplePopupWidget* UPBUIManagerSubsystem::ShowSimplePopupBP(
	const FText& Message,
	const int32 ZOrder)
{
	return ShowSimplePopup(Message, {}, ZOrder);
}

UPBSimplePopupWidget* UPBUIManagerSubsystem::PushSimplePopup(
	const TSubclassOf<UPBSimplePopupWidget> PopupClass,
	const FText& Message,
	const int32 ZOrder)
{
	UPBSimplePopupWidget* Popup = Cast<UPBSimplePopupWidget>(PushWidget(PopupClass, ZOrder));
	if (IsValid(Popup))
	{
		Popup->InitializePopup(Message);
	}

	return Popup;
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

void UPBUIManagerSubsystem::SetGlobalToolbarSuppressedForLoading(const bool bSuppressed)
{
	if (bGlobalToolbarSuppressedForLoading == bSuppressed)
	{
		return;
	}

	bGlobalToolbarSuppressedForLoading = bSuppressed;
	RefreshGlobalToolbarVisibility();
}

void UPBUIManagerSubsystem::RequestGlobalDeckToggle()
{
	OnGlobalDeckToggleRequested.Broadcast();
}

void UPBUIManagerSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	const UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(LoadedWorld) || !IsValid(GameInstance) || LoadedWorld->GetGameInstance() != GameInstance)
	{
		return;
	}

	RefreshGlobalToolbarVisibility();
}

void UPBUIManagerSubsystem::RefreshGlobalToolbarVisibility()
{
	const UGameInstance* GameInstance = GetGameInstance();
	UWorld* World = IsValid(GameInstance) ? GameInstance->GetWorld() : nullptr;
	const bool bShouldShow = IsValid(World)
		&& !bGlobalToolbarSuppressedForLoading
		&& !ShouldHideGlobalToolbar(World);

	if (!bShouldShow)
	{
		if (IsValid(GlobalToolbarWidget))
		{
			GlobalToolbarWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	EnsureGlobalToolbar();
	if (IsValid(GlobalToolbarWidget))
	{
		GlobalToolbarWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UPBUIManagerSubsystem::EnsureGlobalToolbar()
{
	if (IsValid(GlobalToolbarWidget))
	{
		if (!GlobalToolbarWidget->IsInViewport())
		{
			GlobalToolbarWidget->AddToViewport(GlobalToolbarZOrder);
		}
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return;
	}

	TSubclassOf<UPBGlobalToolbarWidget> ToolbarWidgetClass = UPBGlobalToolbarWidget::StaticClass();
	const UPBUISettings* UISettings = GetDefault<UPBUISettings>();
	if (IsValid(UISettings) && !UISettings->GlobalToolbarWidgetClass.IsNull())
	{
		if (UClass* ConfiguredClass = UISettings->GlobalToolbarWidgetClass.LoadSynchronous())
		{
			ToolbarWidgetClass = ConfiguredClass;
		}
	}

	GlobalToolbarWidget = CreateWidget<UPBGlobalToolbarWidget>(
		GameInstance,
		ToolbarWidgetClass);
	if (IsValid(GlobalToolbarWidget))
	{
		GlobalToolbarWidget->AddToViewport(GlobalToolbarZOrder);
	}
}

void UPBUIManagerSubsystem::RemoveGlobalToolbar()
{
	if (IsValid(GlobalToolbarWidget))
	{
		GlobalToolbarWidget->RemoveFromParent();
	}
	GlobalToolbarWidget = nullptr;
}

bool UPBUIManagerSubsystem::ShouldHideGlobalToolbar(const UWorld* World) const
{
	if (!IsValid(World))
	{
		return true;
	}

	const UPBUISettings* UISettings = GetDefault<UPBUISettings>();
	if (!IsValid(UISettings) || UISettings->GlobalToolbarHiddenMapName.IsNone())
	{
		return false;
	}

	FString MapName = World->GetMapName();
	MapName.RemoveFromStart(World->StreamingLevelsPrefix);
	return FName(MapName).IsEqual(UISettings->GlobalToolbarHiddenMapName);
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
