#include "PBOctopusTentacleStatusWidget.h"

#include "PBOctopusTentacleStatusViewModel.h"
#include "View/MVVMView.h"

void UPBOctopusTentacleStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureViewModel();
}

void UPBOctopusTentacleStatusWidget::SetOctopusBoss(APBOctopusBoss* NewOctopusBoss)
{
	EnsureViewModel();
	if (TentacleStatusViewModel)
	{
		TentacleStatusViewModel->SetOctopusBoss(NewOctopusBoss);
	}

	SetVisibility(NewOctopusBoss ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UPBOctopusTentacleStatusWidget::ClearOctopusBoss()
{
	if (TentacleStatusViewModel)
	{
		TentacleStatusViewModel->ClearOctopusBoss();
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UPBOctopusTentacleStatusWidget::NativeDestruct()
{
	ClearOctopusBoss();
	Super::NativeDestruct();
}

void UPBOctopusTentacleStatusWidget::EnsureViewModel()
{
	if (!TentacleStatusViewModel)
	{
		TentacleStatusViewModel = NewObject<UPBOctopusTentacleStatusViewModel>(this);
	}

	if (TentacleStatusViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBOctopusTentacleStatusWidget::ApplyViewModelToWidget()
{
	if (!TentacleStatusViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("OctopusTentacleStatusWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(TentacleStatusViewModel);
	return View->SetViewModelByClass(ViewModelInterface);
}
