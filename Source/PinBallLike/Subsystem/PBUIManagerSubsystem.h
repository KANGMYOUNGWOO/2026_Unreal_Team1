#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/UI/Popup/PBSimplePopupWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBUIManagerSubsystem.generated.h"

class UPBUserWidget;
class UPBGlobalToolbarWidget;
class UWorld;
class UPBBallRewardPopupWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBGlobalDeckToggleRequested);

UCLASS()
class PINBALLLIKE_API UPBUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPBUIManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Widget을 생성하고 Viewport와 Stack에 추가한다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	UPBUserWidget* PushWidget(TSubclassOf<UPBUserWidget> WidgetClass, int32 ZOrder = 0);

	UPBSimplePopupWidget* ShowSimplePopup(
		const FText& Message,
		FPBSimplePopupClosedDelegate ClosedCallback = {},
		int32 ZOrder = 100);

	UPBBallRewardPopupWidget* ShowBallRewardPopup(
		const FText& Message,
		FName BallId,
		int32 StarLevel,
		FPBSimplePopupClosedDelegate ClosedCallback = {},
		int32 ZOrder = 100);

	UFUNCTION(BlueprintCallable, Category = "UI", meta = (DisplayName = "Show Simple Popup"))
	UPBSimplePopupWidget* ShowSimplePopupBP(
		const FText& Message,
		int32 ZOrder = 100);

	// Top Widget에 닫기 요청을 보낸다. 실제 제거는 Widget의 CompletePop 호출 후 처리된다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool RequestPopWidget();

	// 닫기 연출이 끝난 Widget을 Viewport와 Stack에서 제거한다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool CompletePopWidget(UPBUserWidget* Widget);

	// Stack에 남아 있는 모든 Widget을 즉시 제거한다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void PopAllWidgets();

	// Stack의 최상단 Widget을 반환한다.
	UFUNCTION(BlueprintPure, Category = "UI")
	UPBUserWidget* GetTopWidget() const;

	void SetGlobalToolbarSuppressedForLoading(bool bSuppressed);

	UFUNCTION(BlueprintPure, Category = "UI|Global Toolbar")
	UPBGlobalToolbarWidget* GetGlobalToolbarWidget() const { return GlobalToolbarWidget; }

	UFUNCTION(BlueprintCallable, Category = "UI|Global Toolbar")
	void RequestGlobalDeckToggle();

	UPROPERTY(BlueprintAssignable, Category = "UI|Global Toolbar")
	FPBGlobalDeckToggleRequested OnGlobalDeckToggleRequested;

private:
	UPBSimplePopupWidget* PushSimplePopup(
		TSubclassOf<UPBSimplePopupWidget> PopupClass,
		const FText& Message,
		int32 ZOrder);

	// bForceRemove가 true면 닫기 요청 여부와 Top Widget 여부를 무시하고 제거한다.
	bool RemoveWidgetFromStack(UPBUserWidget* Widget, bool bForceRemove);
	void CleanInvalidWidgetsFromStack();
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void RefreshGlobalToolbarVisibility();
	void EnsureGlobalToolbar();
	void RemoveGlobalToolbar();
	bool ShouldHideGlobalToolbar(const UWorld* World) const;

	UPROPERTY()
	TSubclassOf<UPBSimplePopupWidget> DefaultSimplePopupClass;

	UPROPERTY()
	TSoftClassPtr<UPBBallRewardPopupWidget> DefaultBallRewardPopupClass;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBUserWidget>> WidgetStack;

	UPROPERTY(Transient)
	TObjectPtr<UPBGlobalToolbarWidget> GlobalToolbarWidget;

	FDelegateHandle PostLoadMapHandle;
	bool bGlobalToolbarSuppressedForLoading = false;
};
