#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBUIManagerSubsystem.generated.h"

class UPBUserWidget;

UCLASS()
class PINBALLLIKE_API UPBUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Widget을 생성하고 Viewport와 Stack에 추가한다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	UPBUserWidget* PushWidget(TSubclassOf<UPBUserWidget> WidgetClass, int32 ZOrder = 0);

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

private:
	// bForceRemove가 true면 닫기 요청 여부와 Top Widget 여부를 무시하고 제거한다.
	bool RemoveWidgetFromStack(UPBUserWidget* Widget, bool bForceRemove);
	void CleanInvalidWidgetsFromStack();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBUserWidget>> WidgetStack;
};
