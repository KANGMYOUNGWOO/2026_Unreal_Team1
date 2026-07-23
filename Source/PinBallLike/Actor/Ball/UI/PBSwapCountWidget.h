#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "PBSwapCountWidget.generated.h"

class APBBattleGameState;
class UTextBlock;
class UWidgetAnimation;

UCLASS()
class PINBALLLIKE_API UPBSwapCountWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindBattleGameState();
	void UnbindBattleGameState();
	void ScheduleBindBattleGameState();
	void RefreshSwapCountText();
	void ApplySwapCountText(int32 RemainingCount);
	int32 GetBattleShiftLimit() const;

	UFUNCTION()
	void HandleBattleShiftCountChanged(int32 PreviousCount, int32 NewCount);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SwapCount;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Rotate;

	UPROPERTY(Transient)
	TObjectPtr<APBBattleGameState> BattleGameState;

	FTimerHandle BindRetryTimerHandle;
	bool bBattleGameStateBound = false;
};
