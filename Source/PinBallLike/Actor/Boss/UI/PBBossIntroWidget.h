#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBossIntroWidget.generated.h"

class APBBossBase;
class UPBBossIntroViewModel;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0.01"))
	float SlideDurationSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation", meta = (ClampMin = "0"))
	float IntroHoldDurationSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation")
	float SlideStartPositionX = -1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Intro Animation")
	float SlideEndPositionX = 1200.0f;

private:
	void EnsureIntroViewModel();
	bool ApplyViewModelToWidget();
	float CalculateSlideAlpha(float CurrentTime) const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossIntroViewModel> IntroViewModel;

	float AnimationElapsedSeconds = 0.0f;
	bool IsBossIntroAnimationPlaying = false;
};
