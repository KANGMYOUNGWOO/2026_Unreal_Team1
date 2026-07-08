#include "PBBossStatusWidget.h"

#include "PBBossStatusViewModel.h"
#include "View/MVVMView.h"

void UPBBossStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureStatusViewModel();
}

void UPBBossStatusWidget::SetBoss(APBBossBase* NewBoss)
{
	EnsureStatusViewModel();
	if (StatusViewModel)
	{
		StatusViewModel->SetBoss(NewBoss);
	}
}

void UPBBossStatusWidget::ClearBoss()
{
	ClearEnrageWarningTimer();

	if (StatusViewModel)
	{
		StatusViewModel->HideEnrageWarning();
		StatusViewModel->ClearBoss();
	}
}

void UPBBossStatusWidget::ShowEnrageWarning()
{
	EnsureStatusViewModel();
	if (StatusViewModel)
	{
		StatusViewModel->ShowEnrageWarning();
	}

	ClearEnrageWarningTimer();

	UWorld* World = GetWorld();
	if (World && EnrageWarningVisibleSeconds > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			EnrageWarningTimerHandle,
			this,
			&UPBBossStatusWidget::HideEnrageWarning,
			EnrageWarningVisibleSeconds,
			false);
	}
}

void UPBBossStatusWidget::HideEnrageWarning()
{
	ClearEnrageWarningTimer();

	if (StatusViewModel)
	{
		StatusViewModel->HideEnrageWarning();
	}
}

void UPBBossStatusWidget::NativeDestruct()
{
	ClearBoss();
	Super::NativeDestruct();
}

void UPBBossStatusWidget::EnsureStatusViewModel()
{
	if (!StatusViewModel)
	{
		StatusViewModel = NewObject<UPBBossStatusViewModel>(this);
	}

	if (StatusViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBBossStatusWidget::ApplyViewModelToWidget()
{
	if (!StatusViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossStatusWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(StatusViewModel);
	const bool IsResult = View->SetViewModelByClass(ViewModelInterface);
	if (!IsResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossStatusWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(StatusViewModel));
	}

	return IsResult;
}

void UPBBossStatusWidget::ClearEnrageWarningTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(EnrageWarningTimerHandle);
	}
}
