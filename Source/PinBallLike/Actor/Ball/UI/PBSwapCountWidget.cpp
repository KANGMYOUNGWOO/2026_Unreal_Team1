#include "PBSwapCountWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/GameState/PBBattleGameState.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"

void UPBSwapCountWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindBattleGameState();
	RefreshSwapCountText();
}

void UPBSwapCountWidget::NativeDestruct()
{
	UnbindBattleGameState();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
	}

	Super::NativeDestruct();
}

void UPBSwapCountWidget::BindBattleGameState()
{
	if (bBattleGameStateBound)
	{
		return;
	}

	const UWorld* World = GetWorld();
	BattleGameState = World ? World->GetGameState<APBBattleGameState>() : nullptr;
	if (!BattleGameState)
	{
		ScheduleBindBattleGameState();
		return;
	}

	BattleGameState->OnBattleShiftCountChanged.AddUniqueDynamic(this, &UPBSwapCountWidget::HandleBattleShiftCountChanged);
	bBattleGameStateBound = true;
	RefreshSwapCountText();
}

void UPBSwapCountWidget::UnbindBattleGameState()
{
	if (!bBattleGameStateBound || !BattleGameState)
	{
		return;
	}

	BattleGameState->OnBattleShiftCountChanged.RemoveDynamic(this, &UPBSwapCountWidget::HandleBattleShiftCountChanged);
	bBattleGameStateBound = false;
	BattleGameState = nullptr;
}

void UPBSwapCountWidget::ScheduleBindBattleGameState()
{
	if (UWorld* World = GetWorld())
	{
		BindRetryTimerHandle = World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UPBSwapCountWidget::BindBattleGameState));
	}
}

void UPBSwapCountWidget::RefreshSwapCountText()
{
	const int32 RemainingCount = BattleGameState ? BattleGameState->GetRemainingBattleShiftCount() : 0;
	ApplySwapCountText(RemainingCount);
}

void UPBSwapCountWidget::ApplySwapCountText(const int32 RemainingCount)
{
	if (!Text_SwapCount)
	{
		return;
	}

	const int32 BattleShiftLimit = FMath::Max(GetBattleShiftLimit(), RemainingCount);
	Text_SwapCount->SetText(FText::Format(
		NSLOCTEXT("SwapCountWidget", "SwapCountFormat", "교체\n{0}/{1}"),
		RemainingCount,
		BattleShiftLimit));
}

int32 UPBSwapCountWidget::GetBattleShiftLimit() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBPlayerDataSubsystem* PlayerDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBPlayerDataSubsystem>() : nullptr;
	return PlayerDataSubsystem ? PlayerDataSubsystem->GetInitialBattleShiftCount() : 0;
}

void UPBSwapCountWidget::HandleBattleShiftCountChanged(const int32 PreviousCount, const int32 NewCount)
{
	ApplySwapCountText(NewCount);

	if (PreviousCount != NewCount && Rotate)
	{
		PlayAnimation(Rotate);
	}
}
