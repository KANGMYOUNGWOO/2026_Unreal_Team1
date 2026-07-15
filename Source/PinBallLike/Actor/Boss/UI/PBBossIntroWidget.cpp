#include "PBBossIntroWidget.h"

#include "PBBossIntroViewModel.h"
#include "View/MVVMView.h"

void UPBBossIntroWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureIntroViewModel();
}

UPBBossIntroViewModel* UPBBossIntroWidget::CreateIntroViewModel()
{
	if (!IntroViewModel)
	{
		IntroViewModel = NewObject<UPBBossIntroViewModel>(this);
	}

	ApplyViewModelToWidget();
	return IntroViewModel;
}

void UPBBossIntroWidget::SetIntroViewModel(UPBBossIntroViewModel* NewIntroViewModel)
{
	IntroViewModel = NewIntroViewModel;
	ApplyViewModelToWidget();
}

UPBBossIntroViewModel* UPBBossIntroWidget::GetIntroViewModel() const
{
	return IntroViewModel;
}

void UPBBossIntroWidget::SetBoss(APBBossBase* NewBoss)
{
	EnsureIntroViewModel();
	if (IntroViewModel)
	{
		IntroViewModel->SetBoss(NewBoss);
	}
}

void UPBBossIntroWidget::ClearBoss()
{
	if (IntroViewModel)
	{
		IntroViewModel->ClearBoss();
	}
}

void UPBBossIntroWidget::FinishBossIntro()
{
	IsBossIntroAnimationPlaying = false;
	OnBossIntroFinished.Broadcast();
}

void UPBBossIntroWidget::PlayBossIntroAnimation()
{
	AnimationElapsedSeconds = 0.0f;
	IsBossIntroAnimationPlaying = true;
	SetRenderTranslation(FVector2D(SlideStartPositionX, 0.0f));
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPBBossIntroWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsBossIntroAnimationPlaying)
	{
		return;
	}

	AnimationElapsedSeconds += InDeltaTime;

	const float TotalDurationSeconds = (SlideDurationSeconds * 2.0f) + IntroHoldDurationSeconds;
	if (AnimationElapsedSeconds >= TotalDurationSeconds)
	{
		SetRenderTranslation(FVector2D(SlideEndPositionX, 0.0f));
		FinishBossIntro();
		return;
	}

	if (AnimationElapsedSeconds <= SlideDurationSeconds)
	{
		const float SlideAlpha = CalculateSlideAlpha(AnimationElapsedSeconds / SlideDurationSeconds);
		SetRenderTranslation(FVector2D(FMath::Lerp(SlideStartPositionX, 0.0f, SlideAlpha), 0.0f));
		return;
	}

	const float SlideOutStartSeconds = SlideDurationSeconds + IntroHoldDurationSeconds;
	if (AnimationElapsedSeconds >= SlideOutStartSeconds)
	{
		const float SlideOutElapsedSeconds = AnimationElapsedSeconds - SlideOutStartSeconds;
		const float SlideAlpha = CalculateSlideAlpha(SlideOutElapsedSeconds / SlideDurationSeconds);
		SetRenderTranslation(FVector2D(FMath::Lerp(0.0f, SlideEndPositionX, SlideAlpha), 0.0f));
		return;
	}

	SetRenderTranslation(FVector2D::ZeroVector);
}

void UPBBossIntroWidget::NativeDestruct()
{
	IsBossIntroAnimationPlaying = false;
	ClearBoss();

	Super::NativeDestruct();
}

void UPBBossIntroWidget::EnsureIntroViewModel()
{
	if (!IntroViewModel)
	{
		IntroViewModel = NewObject<UPBBossIntroViewModel>(this);
	}

	if (IntroViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBBossIntroWidget::ApplyViewModelToWidget()
{
	if (!IntroViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossIntroWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(IntroViewModel);
	const bool IsResult = View->SetViewModelByClass(ViewModelInterface);
	if (!IsResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossIntroWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(IntroViewModel));
	}

	return IsResult;
}

float UPBBossIntroWidget::CalculateSlideAlpha(float CurrentTime) const
{
	return FMath::InterpEaseInOut(0.0f, 1.0f, FMath::Clamp(CurrentTime, 0.0f, 1.0f), 2.0f);
}
