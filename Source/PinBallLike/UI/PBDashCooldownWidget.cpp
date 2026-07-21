#include "PBDashCooldownWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PinBallLike/GameState/PBBattleGameState.h"

void UPBDashCooldownWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CooldownCircleMaterial = Image_CooldownCircle
		? Image_CooldownCircle->GetDynamicMaterial()
		: nullptr;

	RefreshCooldown();
}

void UPBDashCooldownWidget::NativeTick(
	const FGeometry& MyGeometry,
	const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshCooldown();
}

void UPBDashCooldownWidget::RefreshCooldown()
{
	const UWorld* World = GetWorld();
	const APBBattleGameState* BattleGameState = World
		? World->GetGameState<APBBattleGameState>()
		: nullptr;
	if (!BattleGameState)
	{
		SetCooldownRatio(ReadyRatioValue);
		SetRemainingTimeText(0.0f);
		return;
	}

	const float RemainingSeconds = BattleGameState->GetRemainingBattleDashCooldown();
	const float CooldownRatio = RemainingSeconds > 0.0f
		? BattleGameState->GetBattleDashCooldownRatio()
		: ReadyRatioValue;

	SetCooldownRatio(CooldownRatio);
	SetRemainingTimeText(RemainingSeconds);
}

void UPBDashCooldownWidget::SetCooldownRatio(const float CooldownRatio)
{
	if (CooldownCircleMaterial && !CooldownRatioParameterName.IsNone())
	{
		CooldownCircleMaterial->SetScalarParameterValue(
			CooldownRatioParameterName,
			FMath::Clamp(CooldownRatio, 0.0f, 1.0f));
	}
}

void UPBDashCooldownWidget::SetRemainingTimeText(const float RemainingSeconds)
{
	if (!Text_RemainingTime)
	{
		return;
	}

	if (RemainingSeconds <= 0.0f && bHideRemainingTimeWhenReady)
	{
		Text_RemainingTime->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Text_RemainingTime->SetVisibility(ESlateVisibility::HitTestInvisible);

	FNumberFormattingOptions FormatOptions;
	FormatOptions.MinimumFractionalDigits = 1;
	FormatOptions.MaximumFractionalDigits = 1;
	Text_RemainingTime->SetText(FText::AsNumber(
		FMath::Max(RemainingSeconds, 0.0f),
		&FormatOptions));
}
