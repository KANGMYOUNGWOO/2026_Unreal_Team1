#include "PBBettingWidget.h"
#include  "PinBallLike/Struct/Choice/PBBettingResult.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPBBettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TotalBetGold = 0;
	RefreshTotalBetGoldText();
	SetBetGoldControlsOpacity(0.0f);

	if (OverBetGoldText)
	{
		OverBetGoldText->SetText(
			NSLOCTEXT(
				"PBBetting",
				"OverBetGold",
				"보유 골드를 초과하여 배팅할 수 없습니다."));
		OverBetGoldText->SetRenderOpacity(0.0f);
		OverBetGoldText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (WinnerImage)
	{
		WinnerImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (EarnedGoldText)
	{
		EarnedGoldText->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetNationButtonsEnabled(false);
	SetBetGoldButtonsEnabled(false);
	if (ExitButton)
	{
		ExitButton->SetIsEnabled(false);
	}

	if (NationButton1)
	{
		NationButton1->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnNationButton1Clicked);
	}

	if (NationButton2)
	{
		NationButton2->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnNationButton2Clicked);
	}

	if (BetGoldButton10)
	{
		BetGoldButton10->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnBetGoldButton10Clicked);
	}

	if (BetGoldButton50)
	{
		BetGoldButton50->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnBetGoldButton50Clicked);
	}

	if (BetGoldButton100)
	{
		BetGoldButton100->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnBetGoldButton100Clicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnExitButtonClicked);
	}

	IntroAnimationFinishedEvent.BindDynamic(
		this,
		&UPBBettingWidget::OnIntroAnimationFinished);

	if (IntroAnim)
	{
		BindToAnimationFinished(
			IntroAnim,
			IntroAnimationFinishedEvent);
	}

	/*
	 * 1단계 애니메이션:
	 * 어느 국가가 이겼는지 보여주는 애니메이션
	 */
	WinnerAnimationFinishedEvent.BindDynamic(
		this,
		&UPBBettingWidget::OnWinnerAnimationFinished);

	if (BetResultLeftWin)
	{
		BindToAnimationFinished(
			BetResultLeftWin,
			WinnerAnimationFinishedEvent);
	}

	if (BetResultRightWin)
	{
		BindToAnimationFinished(
			BetResultRightWin,
			WinnerAnimationFinishedEvent);
	}

	/*
	 * 2단계 애니메이션:
	 * 플레이어가 이겼는지 졌는지 보여주는 애니메이션
	 */
	FinalAnimationFinishedEvent.BindDynamic(
		this,
		&UPBBettingWidget::OnFinalResultAnimationFinished);

	if (BetWin)
	{
		BindToAnimationFinished(
			BetWin,
			FinalAnimationFinishedEvent);
	}

	if (BetLose)
	{
		BindToAnimationFinished(
			BetLose,
			FinalAnimationFinishedEvent);
	}
	
	
	BetProgressAnimationFinishedEvent.BindDynamic(
		this,
		&UPBBettingWidget::OnProgressAnimationFinished);
	
	if(BetResultProgress)
	{
		BindToAnimationFinished(
			BetResultProgress,
			BetProgressAnimationFinishedEvent);
	}
	
}

void UPBBettingWidget::NativeDestruct()
{
	IsOverBetGoldMessagePlaying = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			FinalResultDisplayTimerHandle);
	}

	if (BetResultLeftWin)
	{
		UnbindFromAnimationFinished(
			BetResultLeftWin,
			WinnerAnimationFinishedEvent);
	}

	if (BetResultRightWin)
	{
		UnbindFromAnimationFinished(
			BetResultRightWin,
			WinnerAnimationFinishedEvent);
	}

	if (BetWin)
	{
		UnbindFromAnimationFinished(
			BetWin,
			FinalAnimationFinishedEvent);
	}

	if (BetLose)
	{
		UnbindFromAnimationFinished(
			BetLose,
			FinalAnimationFinishedEvent);
	}

	if (IntroAnim)
	{
		UnbindFromAnimationFinished(
			IntroAnim,
			IntroAnimationFinishedEvent);
	}

	Super::NativeDestruct();
}

void UPBBettingWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateOverBetGoldMessage(InDeltaTime);
	UpdateBetGoldControlsFadeIn(InDeltaTime);
	SyncEarnedGoldTextAppearance();
}

void UPBBettingWidget::OnNationButton1Clicked()
{
	if (TotalBetGold <= 0)
	{
		return;
	}

	SetNationButtonsEnabled(false);
	SetBetGoldButtonsEnabled(false);

	OnBetSelected.Broadcast(0, TotalBetGold);
}

void UPBBettingWidget::OnNationButton2Clicked()
{
	if (TotalBetGold <= 0)
	{
		return;
	}

	SetNationButtonsEnabled(false);
	SetBetGoldButtonsEnabled(false);

	OnBetSelected.Broadcast(1, TotalBetGold);
}

void UPBBettingWidget::OnBetGoldButton10Clicked()
{
	AddBetGold(10);
}

void UPBBettingWidget::OnBetGoldButton50Clicked()
{
	AddBetGold(50);
}

void UPBBettingWidget::OnBetGoldButton100Clicked()
{
	AddBetGold(100);
}

void UPBBettingWidget::OnExitButtonClicked()
{
	SetNationButtonsEnabled(false);
	SetBetGoldButtonsEnabled(false);
	if (ExitButton)
	{
		ExitButton->SetIsEnabled(false);
	}

	OnBetExitRequested.Broadcast();
}

void UPBBettingWidget::OnProgressAnimationFinished()
{
	if (CachedResult.WinnerIndex == 0)
	{
		if (BetResultLeftWin)
		{
			PlayAnimation(BetResultLeftWin);
			return;
		}
	}
	else
	{
		if (BetResultRightWin)
		{
			PlayAnimation(BetResultRightWin);
			return;
		}
	}

	OnWinnerAnimationFinished();
}

void UPBBettingWidget::PlayIntroAnimation()
{
	if (!IntroAnim)
	{
		OnIntroAnimationFinished();
		return;
	}

	PlayAnimation(
		IntroAnim,
		0.f,
		1,
		EUMGSequencePlayMode::Forward,
		1.f);
}

void UPBBettingWidget::OnIntroAnimationFinished()
{
	SetBetGoldButtonsEnabled(true);
	if (ExitButton)
	{
		ExitButton->SetIsEnabled(true);
	}
	StartBetGoldControlsFadeIn();
}

void UPBBettingWidget::PlayBetResultAnimations(
	const FPBBettingResult& Result)
{
	CachedResult = Result;
	IsPlayerWon = Result.bWin;
	SetBetGoldControlsVisibility(ESlateVisibility::Collapsed);

	if (BetResultProgress)
	{
		PlayAnimation(BetResultProgress);
		return;
	}

	// Progress 애니메이션이 없는 경우에만
	// 다음 단계로 직접 넘어간다.
	OnProgressAnimationFinished();
}

void UPBBettingWidget::SetAvailableGold(int32 NewAvailableGold)
{
	AvailableGold = FMath::Max(NewAvailableGold, 0);
	RefreshCurrentGoldText();
}

void UPBBettingWidget::OnWinnerAnimationFinished()
{
	ShowWinnerImage();
	ShowEarnedGoldText();

	/*
	 * 왼쪽/오른쪽 승리 표시가 끝난 뒤
	 * 플레이어 개인의 승패 연출을 실행한다.
	 */
	UWidgetAnimation* FinalAnimation =
		IsPlayerWon ? BetWin : BetLose;

	if (!FinalAnimation)
	{
		OnFinalResultAnimationFinished();
		return;
	}

	PlayAnimation(FinalAnimation);
}

void UPBBettingWidget::ShowWinnerImage()
{
	if (!WinnerImage)
	{
		return;
	}

	const UImage* WinningFlag =
		CachedResult.WinnerIndex == 0 ? Flag1.Get() : Flag2.Get();
	if (!WinningFlag)
	{
		return;
	}

	WinnerImage->SetBrush(WinningFlag->GetBrush());
	WinnerImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPBBettingWidget::ShowEarnedGoldText()
{
	if (!EarnedGoldText)
	{
		return;
	}

	EarnedGoldText->SetText(
		FText::Format(
			IsPlayerWon
				? NSLOCTEXT("PBBetting", "EarnedGoldWin", "+{0} Gold")
				: NSLOCTEXT("PBBetting", "EarnedGoldLose", "-{0} Gold"),
			FText::AsNumber(TotalBetGold)));

	IsEarnedGoldTextActive = true;
	EarnedGoldText->SetVisibility(ESlateVisibility::HitTestInvisible);
	SyncEarnedGoldTextAppearance();
}

void UPBBettingWidget::SyncEarnedGoldTextAppearance()
{
	if (!IsEarnedGoldTextActive || !EarnedGoldText)
	{
		return;
	}

	if (!ResultText)
	{
		EarnedGoldText->SetRenderOpacity(1.0f);
		return;
	}

	EarnedGoldText->SetRenderOpacity(
		ResultText->GetRenderOpacity());
	EarnedGoldText->SetVisibility(
		ResultText->GetVisibility());
}

void UPBBettingWidget::OnFinalResultAnimationFinished()
{
	UWorld* World = GetWorld();
	if (!World || FinalResultDisplayDuration <= 0.0f)
	{
		FinishResultDisplay();
		return;
	}

	World->GetTimerManager().SetTimer(
		FinalResultDisplayTimerHandle,
		this,
		&UPBBettingWidget::FinishResultDisplay,
		FinalResultDisplayDuration,
		false);
}

void UPBBettingWidget::FinishResultDisplay()
{
	IsEarnedGoldTextActive = false;
	if (EarnedGoldText)
	{
		EarnedGoldText->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnBetResultAnimationsFinished.Broadcast();
}

void UPBBettingWidget::AddBetGold(int32 GoldAmount)
{
	const int32 RequestedBetGold = TotalBetGold + GoldAmount;
	if (RequestedBetGold > AvailableGold)
	{
		PlayOverBetGoldMessage();
		return;
	}

	TotalBetGold = RequestedBetGold;
	RefreshTotalBetGoldText();
	RefreshCurrentGoldText();
	SetNationButtonsEnabled(TotalBetGold > 0);
}

void UPBBettingWidget::RefreshTotalBetGoldText()
{
	if (!TotalBetGoldText)
	{
		return;
	}

	TotalBetGoldText->SetText(
		FText::Format(
			NSLOCTEXT("PBBetting", "TotalBetGold", "총 배팅 금액 : {0}"),
			FText::AsNumber(TotalBetGold)));
}

void UPBBettingWidget::RefreshCurrentGoldText()
{
	if (!CurrentGoldText)
	{
		return;
	}

	CurrentGoldText->SetText(
		FText::Format(
			NSLOCTEXT("PBBetting", "CurrentGold", "현재 골드 : {0}"),
			FText::AsNumber(
				FMath::Max(AvailableGold - TotalBetGold, 0))));
}

void UPBBettingWidget::PlayOverBetGoldMessage()
{
	if (!OverBetGoldText)
	{
		return;
	}

	OverBetGoldMessageElapsedTime = 0.0f;
	IsOverBetGoldMessagePlaying = true;
	OverBetGoldText->SetVisibility(ESlateVisibility::HitTestInvisible);
	OverBetGoldText->SetRenderOpacity(0.0f);
}

void UPBBettingWidget::UpdateOverBetGoldMessage(float DeltaTime)
{
	if (!IsOverBetGoldMessagePlaying || !OverBetGoldText)
	{
		return;
	}

	OverBetGoldMessageElapsedTime += DeltaTime;

	const float FadeInEndTime = OverBetGoldFadeInDuration;
	const float DisplayEndTime = FadeInEndTime + OverBetGoldDisplayDuration;
	const float FadeOutEndTime = DisplayEndTime + OverBetGoldFadeOutDuration;
	float Opacity = 1.0f;

	if (OverBetGoldMessageElapsedTime < FadeInEndTime)
	{
		Opacity = OverBetGoldFadeInDuration > 0.0f
			? OverBetGoldMessageElapsedTime / OverBetGoldFadeInDuration
			: 1.0f;
	}
	else if (OverBetGoldMessageElapsedTime > DisplayEndTime)
	{
		Opacity = OverBetGoldFadeOutDuration > 0.0f
			? 1.0f - ((OverBetGoldMessageElapsedTime - DisplayEndTime) / OverBetGoldFadeOutDuration)
			: 0.0f;
	}

	OverBetGoldText->SetRenderOpacity(FMath::Clamp(Opacity, 0.0f, 1.0f));

	if (OverBetGoldMessageElapsedTime < FadeOutEndTime)
	{
		return;
	}

	IsOverBetGoldMessagePlaying = false;
	OverBetGoldText->SetRenderOpacity(0.0f);
	OverBetGoldText->SetVisibility(ESlateVisibility::Collapsed);
}

void UPBBettingWidget::SetBetGoldControlsVisibility(
	ESlateVisibility NewVisibility)
{
	if (BetGoldButton10)
	{
		BetGoldButton10->SetVisibility(NewVisibility);
	}

	if (BetGoldButton50)
	{
		BetGoldButton50->SetVisibility(NewVisibility);
	}

	if (BetGoldButton100)
	{
		BetGoldButton100->SetVisibility(NewVisibility);
	}

	if (ExitButton)
	{
		ExitButton->SetVisibility(NewVisibility);
	}

	if (TotalBetGoldText)
	{
		TotalBetGoldText->SetVisibility(NewVisibility);
	}

	if (CurrentGoldText)
	{
		CurrentGoldText->SetVisibility(NewVisibility);
	}

	if (BetText1)
	{
		BetText1->SetVisibility(NewVisibility);
	}

	if (BetText2)
	{
		BetText2->SetVisibility(NewVisibility);
	}
}

void UPBBettingWidget::StartBetGoldControlsFadeIn()
{
	BetGoldControlsFadeInElapsedTime = 0.0f;
	IsBetGoldControlsFadingIn = true;
	SetBetGoldControlsOpacity(0.0f);
}

void UPBBettingWidget::UpdateBetGoldControlsFadeIn(float DeltaTime)
{
	if (!IsBetGoldControlsFadingIn)
	{
		return;
	}

	BetGoldControlsFadeInElapsedTime += DeltaTime;
	const float Opacity = BetGoldControlsFadeInDuration > 0.0f
		? BetGoldControlsFadeInElapsedTime / BetGoldControlsFadeInDuration
		: 1.0f;

	SetBetGoldControlsOpacity(FMath::Clamp(Opacity, 0.0f, 1.0f));

	if (Opacity >= 1.0f)
	{
		IsBetGoldControlsFadingIn = false;
	}
}

void UPBBettingWidget::SetBetGoldControlsOpacity(float Opacity)
{
	if (BetGoldButton10)
	{
		BetGoldButton10->SetRenderOpacity(Opacity);
	}

	if (BetGoldButton50)
	{
		BetGoldButton50->SetRenderOpacity(Opacity);
	}

	if (BetGoldButton100)
	{
		BetGoldButton100->SetRenderOpacity(Opacity);
	}

	if (ExitButton)
	{
		ExitButton->SetRenderOpacity(Opacity);
	}

	if (TotalBetGoldText)
	{
		TotalBetGoldText->SetRenderOpacity(Opacity);
	}

	if (CurrentGoldText)
	{
		CurrentGoldText->SetRenderOpacity(Opacity);
	}

	if (BetText1)
	{
		BetText1->SetRenderOpacity(Opacity);
	}

	if (BetText2)
	{
		BetText2->SetRenderOpacity(Opacity);
	}
}

void UPBBettingWidget::SetNationButtonsEnabled(bool IsEnabled)
{
	if (NationButton1)
	{
		NationButton1->SetIsEnabled(IsEnabled);
	}

	if (NationButton2)
	{
		NationButton2->SetIsEnabled(IsEnabled);
	}
}

void UPBBettingWidget::SetBetGoldButtonsEnabled(bool IsEnabled)
{
	if (BetGoldButton10)
	{
		BetGoldButton10->SetIsEnabled(IsEnabled);
	}

	if (BetGoldButton50)
	{
		BetGoldButton50->SetIsEnabled(IsEnabled);
	}

	if (BetGoldButton100)
	{
		BetGoldButton100->SetIsEnabled(IsEnabled);
	}
}
