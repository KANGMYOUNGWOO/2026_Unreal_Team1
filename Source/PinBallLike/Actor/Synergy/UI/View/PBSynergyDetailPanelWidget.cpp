#include "PBSynergyDetailPanelWidget.h"

#include "Components/PanelWidget.h"
#include "PBSynergyBallIconWidget.h"
#include "PBSynergyTierRowWidget.h"
#include "PinBallLike/Actor/Synergy/UI/ViewModel/PBSynergyDetailPanelViewModel.h"
#include "View/MVVMView.h"

void UPBSynergyDetailPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureViewModel();
}

void UPBSynergyDetailPanelWidget::InitializeSynergyDetailPanel(const FPBSynergyViewData& InViewData)
{
	ViewData = InViewData;

	EnsureViewModel();
	if (ViewModel)
	{
		ViewModel->SetSynergyViewData(ViewData);
	}

	RebuildTierList();
	RebuildBallIconList();
}

void UPBSynergyDetailPanelWidget::EnsureViewModel()
{
	if (!ViewModel)
	{
		ViewModel = NewObject<UPBSynergyDetailPanelViewModel>(this);
	}

	if (ViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBSynergyDetailPanelWidget::ApplyViewModelToWidget()
{
	if (!ViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("SynergyDetailPanelWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("SynergyDetailPanelWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(ViewModel));
	}

	return bResult;
}

void UPBSynergyDetailPanelWidget::RebuildTierList()
{
	if (!VerticalBox_TierList)
	{
		return;
	}

	VerticalBox_TierList->ClearChildren();

	if (!TierRowWidgetClass)
	{
		return;
	}

	for (const FPBSynergyTierViewData& TierViewData : ViewData.TierViewDataList)
	{
		UPBSynergyTierRowWidget* TierRowWidget = CreateWidget<UPBSynergyTierRowWidget>(this, TierRowWidgetClass);
		if (!TierRowWidget)
		{
			continue;
		}

		TierRowWidget->InitializeTierRow(TierViewData);
		VerticalBox_TierList->AddChild(TierRowWidget);
	}
}

void UPBSynergyDetailPanelWidget::RebuildBallIconList()
{
	if (!HorizontalBox_Ballicons)
	{
		return;
	}

	HorizontalBox_Ballicons->ClearChildren();

	if (!BallIconWidgetClass)
	{
		return;
	}

	for (const FPBSynergyBallIconViewData& BallIconViewData : ViewData.BallIconViewDataList)
	{
		UPBSynergyBallIconWidget* BallIconWidget = CreateWidget<UPBSynergyBallIconWidget>(this, BallIconWidgetClass);
		if (!BallIconWidget)
		{
			continue;
		}

		BallIconWidget->InitializeBallIcon(BallIconViewData);
		HorizontalBox_Ballicons->AddChild(BallIconWidget);
	}
}
