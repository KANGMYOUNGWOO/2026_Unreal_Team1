#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Choice/PBBettingResult.h"
#include "PBBettingWidget.generated.h"

class UButton;
class UWidgetAnimation;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnBetSelected,
	int32,
	SelectedIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnBetResultAnimationsFinished);

UCLASS()
class PINBALLLIKE_API UPBBettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Bet")
	FOnBetSelected OnBetSelected;

	// 결과 연출이 전부 끝났음을 Actor에 알림
	UPROPERTY(BlueprintAssignable, Category = "Bet")
	FOnBetResultAnimationsFinished OnBetResultAnimationsFinished;

	void PlayIntroAnimation();

	void PlayBetResultAnimations(const FPBBettingResult& Result);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NationButton1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NationButton2;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> IntroAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> BetResultLeftWin;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> BetResultRightWin;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> BetWin;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> BetLose;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> BetResultProgress;
	
	UFUNCTION()
	void OnNationButton1Clicked();

	UFUNCTION()
	void OnNationButton2Clicked();

	UFUNCTION()
	void OnProgressAnimationFinished();
	// 왼쪽/오른쪽 승리 애니메이션 종료
	UFUNCTION()
	void OnWinnerAnimationFinished();

	// 플레이어 승리/패배 애니메이션 종료
	UFUNCTION()
	void OnFinalResultAnimationFinished();

	void SetBetButtonsEnabled(bool bEnabled);

	bool bPlayerWon = false;

	FPBBettingResult CachedResult;
	
	FWidgetAnimationDynamicEvent WinnerAnimationFinishedEvent;
	FWidgetAnimationDynamicEvent FinalAnimationFinishedEvent;
	FWidgetAnimationDynamicEvent BetProgressAnimationFinishedEvent;
};