#include "PBBettingWidget.h"
#include  "PinBallLike/Struct/Choice/PBBettingResult.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"

void UPBBettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

	Super::NativeDestruct();
}

void UPBBettingWidget::OnNationButton1Clicked()
{
	SetBetButtonsEnabled(false);

	OnBetSelected.Broadcast(0);
}

void UPBBettingWidget::OnNationButton2Clicked()
{
	SetBetButtonsEnabled(false);

	OnBetSelected.Broadcast(1);
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
		return;
	}

	PlayAnimation(
		IntroAnim,
		0.f,
		1,
		EUMGSequencePlayMode::Forward,
		1.f);
}

void UPBBettingWidget::PlayBetResultAnimations(
	const FPBBettingResult& Result)
{
	CachedResult = Result;
	bPlayerWon = Result.bWin;

	if (BetResultProgress)
	{
		PlayAnimation(BetResultProgress);
		return;
	}

	// Progress 애니메이션이 없는 경우에만
	// 다음 단계로 직접 넘어간다.
	OnProgressAnimationFinished();
}

void UPBBettingWidget::OnWinnerAnimationFinished()
{
	/*
	 * 왼쪽/오른쪽 승리 표시가 끝난 뒤
	 * 플레이어 개인의 승패 연출을 실행한다.
	 */
	UWidgetAnimation* FinalAnimation =
		bPlayerWon ? BetWin : BetLose;

	if (!FinalAnimation)
	{
		OnFinalResultAnimationFinished();
		return;
	}

	PlayAnimation(FinalAnimation);
}

void UPBBettingWidget::OnFinalResultAnimationFinished()
{
	/*
	 * 여기서는 직접 RemoveFromParent 하지 않는다.
	 * Actor에게 모든 연출이 끝났다고만 알린다.
	 */
	OnBetResultAnimationsFinished.Broadcast();
}

void UPBBettingWidget::SetBetButtonsEnabled(bool bEnabled)
{
	if (NationButton1)
	{
		NationButton1->SetIsEnabled(bEnabled);
	}

	if (NationButton2)
	{
		NationButton2->SetIsEnabled(bEnabled);
	}
}