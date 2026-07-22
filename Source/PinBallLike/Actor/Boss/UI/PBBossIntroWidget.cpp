#include "PBBossIntroWidget.h"

#include "PBBossIntroViewModel.h"
#include "PBBossIntroBallWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
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

	SetBossOnChildWidgets(NewBoss);
}

void UPBBossIntroWidget::ClearBoss()
{
	if (IntroViewModel)
	{
		IntroViewModel->ClearBoss();
	}

	ClearBossOnChildWidgets();
}

void UPBBossIntroWidget::FinishBossIntro()
{
	IsBossIntroAnimationPlaying = false;
	OnBossIntroFinished.Broadcast();
}

void UPBBossIntroWidget::PlayBossIntroAnimation()
{
	ResolveAnimationWidgets();
	ResetAnimationWidgets();
	AnimationElapsedSeconds = 0.0f;
	IsBossIntroAnimationPlaying = true;
	SetRenderTranslation(FVector2D::ZeroVector);
	SetRenderOpacity(1.0f);
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

	if (AnimationElapsedSeconds < ApproachDurationSeconds)
	{
		UpdateApproachAnimation(AnimationElapsedSeconds / ApproachDurationSeconds);
		return;
	}

	const float ImpactReturnEndSeconds = ApproachDurationSeconds + ImpactReturnDurationSeconds;
	if (AnimationElapsedSeconds < ImpactReturnEndSeconds)
	{
		const float PhaseElapsedSeconds = AnimationElapsedSeconds - ApproachDurationSeconds;
		UpdateImpactReturnAnimation(PhaseElapsedSeconds / ImpactReturnDurationSeconds);
		return;
	}

	const float HoldEndSeconds = ImpactReturnEndSeconds + IntroHoldDurationSeconds;
	if (AnimationElapsedSeconds < HoldEndSeconds)
	{
		return;
	}

	const float TotalDurationSeconds = HoldEndSeconds + FadeOutDurationSeconds;
	if (AnimationElapsedSeconds < TotalDurationSeconds)
	{
		const float PhaseElapsedSeconds = AnimationElapsedSeconds - HoldEndSeconds;
		UpdateFadeOutAnimation(PhaseElapsedSeconds / FadeOutDurationSeconds);
		return;
	}

	if (AnimationElapsedSeconds >= TotalDurationSeconds)
	{
		FinishBossIntro();
	}
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

void UPBBossIntroWidget::SetBossOnChildWidgets(APBBossBase* NewBoss)
{
	if (!WidgetTree)
	{
		return;
	}

	TArray<UWidget*> ChildWidgets;
	WidgetTree->GetAllWidgets(ChildWidgets);
	for (UWidget* ChildWidget : ChildWidgets)
	{
		UPBBossIntroWidget* BossIntroChildWidget = Cast<UPBBossIntroWidget>(ChildWidget);
		if (BossIntroChildWidget && BossIntroChildWidget != this)
		{
			BossIntroChildWidget->SetBoss(NewBoss);
		}
	}
}

void UPBBossIntroWidget::ClearBossOnChildWidgets()
{
	if (!WidgetTree)
	{
		return;
	}

	TArray<UWidget*> ChildWidgets;
	WidgetTree->GetAllWidgets(ChildWidgets);
	for (UWidget* ChildWidget : ChildWidgets)
	{
		UPBBossIntroWidget* BossIntroChildWidget = Cast<UPBBossIntroWidget>(ChildWidget);
		if (BossIntroChildWidget && BossIntroChildWidget != this)
		{
			BossIntroChildWidget->ClearBoss();
		}
	}
}

void UPBBossIntroWidget::ResolveAnimationWidgets()
{
	BossPanelWidget = nullptr;
	BallPanelWidget = nullptr;
	VersusTextWidget = nullptr;

	if (!WidgetTree)
	{
		return;
	}

	TArray<UWidget*> ChildWidgets;
	WidgetTree->GetAllWidgets(ChildWidgets);
	for (UWidget* ChildWidget : ChildWidgets)
	{
		if (!BossPanelWidget)
		{
			UPBBossIntroWidget* BossIntroChildWidget = Cast<UPBBossIntroWidget>(ChildWidget);
			if (BossIntroChildWidget && BossIntroChildWidget != this)
			{
				BossPanelWidget = BossIntroChildWidget;
				continue;
			}
		}

		if (!BallPanelWidget && Cast<UPBBossIntroBallWidget>(ChildWidget))
		{
			BallPanelWidget = ChildWidget;
			continue;
		}

		UTextBlock* TextBlock = Cast<UTextBlock>(ChildWidget);
		if (!VersusTextWidget && TextBlock && TextBlock->GetText().ToString().Equals(TEXT("VS"), ESearchCase::IgnoreCase))
		{
			VersusTextWidget = TextBlock;
		}
	}
}

void UPBBossIntroWidget::ResetAnimationWidgets()
{
	if (BossPanelWidget)
	{
		BossPanelWidget->SetRenderTranslation(FVector2D::ZeroVector);
	}

	if (BallPanelWidget)
	{
		BallPanelWidget->SetRenderTranslation(FVector2D::ZeroVector);
	}

	if (VersusTextWidget)
	{
		VersusTextWidget->SetRenderOpacity(0.0f);
		VersusTextWidget->SetRenderScale(FVector2D(1.4f, 1.4f));
	}
}

void UPBBossIntroWidget::UpdateApproachAnimation(const float PhaseAlpha)
{
	const float EasedAlpha = FMath::InterpEaseIn(0.0f, 1.0f, FMath::Clamp(PhaseAlpha, 0.0f, 1.0f), 2.0f);
	if (BossPanelWidget)
	{
		BossPanelWidget->SetRenderTranslation(FVector2D(PanelApproachDistance * EasedAlpha, 0.0f));
	}

	if (BallPanelWidget)
	{
		BallPanelWidget->SetRenderTranslation(FVector2D(-PanelApproachDistance * EasedAlpha, 0.0f));
	}
}

void UPBBossIntroWidget::UpdateImpactReturnAnimation(const float PhaseAlpha)
{
	const float ClampedAlpha = FMath::Clamp(PhaseAlpha, 0.0f, 1.0f);
	const float ImpactDistance = FMath::Lerp(PanelApproachDistance - PanelImpactPushDistance, 0.0f,
		FMath::InterpEaseOut(0.0f, 1.0f, ClampedAlpha, 3.0f));

	if (BossPanelWidget)
	{
		BossPanelWidget->SetRenderTranslation(FVector2D(ImpactDistance, 0.0f));
	}

	if (BallPanelWidget)
	{
		BallPanelWidget->SetRenderTranslation(FVector2D(-ImpactDistance, 0.0f));
	}

	if (VersusTextWidget)
	{
		VersusTextWidget->SetRenderOpacity(FMath::Clamp(ClampedAlpha * 4.0f, 0.0f, 1.0f));
		const float VersusScale = FMath::Lerp(1.4f, 1.0f,
			FMath::InterpEaseOut(0.0f, 1.0f, ClampedAlpha, 3.0f));
		VersusTextWidget->SetRenderScale(FVector2D(VersusScale, VersusScale));
	}
}

void UPBBossIntroWidget::UpdateFadeOutAnimation(const float PhaseAlpha)
{
	SetRenderOpacity(1.0f - FMath::Clamp(PhaseAlpha, 0.0f, 1.0f));
}
