#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Choice/PBBettingResult.h"
#include "PBBettingWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;
class UWidgetAnimation;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBetSelected,
	int32,
	SelectedIndex,
	int32,
	BetGold);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnBetResultAnimationsFinished);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnBetExitRequested);

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

	UPROPERTY(BlueprintAssignable, Category = "Bet")
	FOnBetExitRequested OnBetExitRequested;

	void PlayIntroAnimation();

	void PlayBetResultAnimations(const FPBBettingResult& Result);
	void SetAvailableGold(int32 NewAvailableGold);
	void SetNationData(
		const FText& NationName1,
		UTexture2D* NationSprite1,
		const FText& NationName2,
		UTexture2D* NationSprite2);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NationButton1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NationButton2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BetGoldButton10;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BetGoldButton50;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BetGoldButton100;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TotalBetGoldText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentGoldText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EarnedGoldText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OverBetGoldText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BetText1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BetText2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NationText1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NationText2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Flag1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Flag2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> WinnerImage;

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
	void OnBetGoldButton10Clicked();

	UFUNCTION()
	void OnBetGoldButton50Clicked();

	UFUNCTION()
	void OnBetGoldButton100Clicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void OnProgressAnimationFinished();

	UFUNCTION()
	void OnIntroAnimationFinished();
	// 왼쪽/오른쪽 승리 애니메이션 종료
	UFUNCTION()
	void OnWinnerAnimationFinished();

	// 플레이어 승리/패배 애니메이션 종료
	UFUNCTION()
	void OnFinalResultAnimationFinished();
	void FinishResultDisplay();

	void AddBetGold(int32 GoldAmount);
	void RefreshTotalBetGoldText();
	void RefreshCurrentGoldText();
	void PlayOverBetGoldMessage();
	void UpdateOverBetGoldMessage(float DeltaTime);
	void ShowWinnerImage();
	void ShowEarnedGoldText();
	void SyncEarnedGoldTextAppearance();
	void StartBetGoldControlsFadeIn();
	void UpdateBetGoldControlsFadeIn(float DeltaTime);
	void SetBetGoldControlsOpacity(float Opacity);
	void SetBetGoldControlsVisibility(ESlateVisibility NewVisibility);
	void SetNationButtonsEnabled(bool IsEnabled);
	void SetBetGoldButtonsEnabled(bool IsEnabled);

	bool IsPlayerWon = false;
	bool IsOverBetGoldMessagePlaying = false;
	bool IsBetGoldControlsFadingIn = false;
	bool IsEarnedGoldTextActive = false;
	int32 TotalBetGold = 0;
	int32 AvailableGold = 0;
	float OverBetGoldMessageElapsedTime = 0.0f;
	float OverBetGoldFadeInDuration = 0.25f;
	float OverBetGoldDisplayDuration = 0.5f;
	float OverBetGoldFadeOutDuration = 0.25f;
	float BetGoldControlsFadeInElapsedTime = 0.0f;
	float BetGoldControlsFadeInDuration = 0.5f;
	float FinalResultDisplayDuration = 1.3f;
	FTimerHandle FinalResultDisplayTimerHandle;

	FPBBettingResult CachedResult;
	
	FWidgetAnimationDynamicEvent WinnerAnimationFinishedEvent;
	FWidgetAnimationDynamicEvent FinalAnimationFinishedEvent;
	FWidgetAnimationDynamicEvent BetProgressAnimationFinishedEvent;
	FWidgetAnimationDynamicEvent IntroAnimationFinishedEvent;
};
