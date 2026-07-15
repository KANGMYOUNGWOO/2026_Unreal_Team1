#include "PBSynergyPanelWidget.h"

#include "PBSynergyPanelViewModel.h"
#include "View/MVVMView.h"

void UPBSynergyPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsurePanelViewModel();
}

void UPBSynergyPanelWidget::InitializeSynergyPanel(const FPBSynergyViewData& InViewData)
{
	ViewData = InViewData;

	EnsurePanelViewModel();
	if (PanelViewModel)
	{
		PanelViewModel->SetSynergyViewData(ViewData);
	}
}

void UPBSynergyPanelWidget::ClearSynergyPanel()
{
	ViewData = FPBSynergyViewData();

	if (PanelViewModel)
	{
		PanelViewModel->ClearSynergy();
	}
}

void UPBSynergyPanelWidget::EnsurePanelViewModel()
{
	if (!PanelViewModel)
	{
		PanelViewModel = NewObject<UPBSynergyPanelViewModel>(this);
	}

	if (PanelViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBSynergyPanelWidget::ApplyViewModelToWidget()
{
	if (!PanelViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("SynergyPanelWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(PanelViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("SynergyPanelWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(PanelViewModel));
	}

	return bResult;
}
