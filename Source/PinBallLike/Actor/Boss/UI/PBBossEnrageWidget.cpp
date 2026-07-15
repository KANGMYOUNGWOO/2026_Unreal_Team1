#include "PBBossEnrageWidget.h"

#include "PBBossEnrageViewModel.h"
#include "View/MVVMView.h"

void UPBBossEnrageWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureEnrageViewModel();
}

void UPBBossEnrageWidget::ShowEnrageWarning()
{
	EnsureEnrageViewModel();
	if (EnrageViewModel)
	{
		EnrageViewModel->ShowEnrageWarning();
	}

	ClearEnrageWarningTimer();

	UWorld* World = GetWorld();
	if (World && EnrageWarningVisibleSeconds > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			EnrageWarningTimerHandle,
			this,
			&UPBBossEnrageWidget::HideEnrageWarning,
			EnrageWarningVisibleSeconds,
			false);
	}
}

void UPBBossEnrageWidget::HideEnrageWarning()
{
	ClearEnrageWarningTimer();

	if (EnrageViewModel)
	{
		EnrageViewModel->HideEnrageWarning();
	}
}

void UPBBossEnrageWidget::NativeDestruct()
{
	HideEnrageWarning();
	Super::NativeDestruct();
}

void UPBBossEnrageWidget::EnsureEnrageViewModel()
{
	if (!EnrageViewModel)
	{
		EnrageViewModel = NewObject<UPBBossEnrageViewModel>(this);
	}

	if (EnrageViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBBossEnrageWidget::ApplyViewModelToWidget()
{
	if (!EnrageViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossEnrageWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(EnrageViewModel);
	const bool IsResult = View->SetViewModelByClass(ViewModelInterface);
	if (!IsResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossEnrageWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(EnrageViewModel));
	}

	return IsResult;
}

void UPBBossEnrageWidget::ClearEnrageWarningTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(EnrageWarningTimerHandle);
	}
}
