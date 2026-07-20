#include "PBSynergyPanelWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "PBSynergyDetailPanelWidget.h"
#include "PinBallLike/Actor/Synergy/UI/ViewModel/PBSynergyPanelViewModel.h"
#include "View/MVVMView.h"

void UPBSynergyPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsurePanelViewModel();
}

void UPBSynergyPanelWidget::NativeDestruct()
{
	HideDetailPanel();

	Super::NativeDestruct();
}

void UPBSynergyPanelWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	ShowDetailPanel();
	UpdateDetailPanelPosition();
}

void UPBSynergyPanelWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	HideDetailPanel();

	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply UPBSynergyPanelWidget::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	UpdateDetailPanelPosition();

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
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

void UPBSynergyPanelWidget::ShowDetailPanel()
{
	if (!ViewData.IsValid() || !SynergyDetailPanelWidgetClass)
	{
		return;
	}

	if (!DetailPanelWidget)
	{
		DetailPanelWidget = CreateWidget<UPBSynergyDetailPanelWidget>(
			GetOwningPlayer(),
			SynergyDetailPanelWidgetClass);
	}

	if (!DetailPanelWidget)
	{
		return;
	}

	DetailPanelWidget->InitializeSynergyDetailPanel(ViewData);

	if (!DetailPanelWidget->IsInViewport())
	{
		DetailPanelWidget->AddToViewport(100);
	}
}

void UPBSynergyPanelWidget::HideDetailPanel()
{
	if (DetailPanelWidget)
	{
		DetailPanelWidget->RemoveFromParent();
		DetailPanelWidget = nullptr;
	}
}

void UPBSynergyPanelWidget::UpdateDetailPanelPosition()
{
	if (!DetailPanelWidget || !DetailPanelWidget->IsInViewport())
	{
		return;
	}

	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	DetailPanelWidget->SetPositionInViewport(MousePosition + DetailPanelViewportOffset, false);
}
