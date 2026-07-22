#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBossIntroWidget.generated.h"

class APBBossBase;
class UPBBossIntroViewModel;
class UTextBlock;
class UWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBBossIntroFinishedSignature);

UCLASS()
class PINBALLLIKE_API UPBBossIntroWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Intro UI")
	UPBBossIntroViewModel* CreateIntroViewModel();

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro UI")
	void SetIntroViewModel(UPBBossIntroViewModel* NewIntroViewModel);

	UFUNCTION(BlueprintPure, Category = "Boss|Intro UI")
	UPBBossIntroViewModel* GetIntroViewModel() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro UI")
	void SetBoss(APBBossBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro UI")
	void ClearBoss();

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro UI")
	void FinishBossIntro();

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro UI")
	void PlayBossIntroAnimation();

	UPROPERTY(BlueprintAssignable, Category = "Boss|Intro UI")
	FPBBossIntroFinishedSignature OnBossIntroFinished;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0"))
	float IntroHoldDurationSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0.01"))
	float ApproachDurationSeconds = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0.01"))
	float ImpactReturnDurationSeconds = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0.01"))
	float FadeOutDurationSeconds = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0"))
	float PanelApproachDistance = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0"))
	float PanelImpactPushDistance = 40.0f;

private:
	void EnsureIntroViewModel();
	bool ApplyViewModelToWidget();
	void SetBossOnChildWidgets(APBBossBase* NewBoss);
	void ClearBossOnChildWidgets();
	void ResolveAnimationWidgets();
	void ResetAnimationWidgets();
	void UpdateApproachAnimation(float PhaseAlpha);
	void UpdateImpactReturnAnimation(float PhaseAlpha);
	void UpdateFadeOutAnimation(float PhaseAlpha);

	UPROPERTY(Transient)
	TObjectPtr<UPBBossIntroViewModel> IntroViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> BossPanelWidget;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> BallPanelWidget;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> VersusTextWidget;

	float AnimationElapsedSeconds = 0.0f;
	bool IsBossIntroAnimationPlaying = false;
};
