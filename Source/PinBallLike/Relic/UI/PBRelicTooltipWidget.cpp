#include "PBRelicTooltipWidget.h"

#include "PBRelicViewModel.h"
#include "View/MVVMView.h"

void UPBRelicTooltipWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureRelicViewModel();
}

void UPBRelicTooltipWidget::SetRelicData(const FText& InDisplayName, const FText& InDescription)
{
	FPBRelicViewData NewViewData;
	NewViewData.DisplayName = InDisplayName;
	NewViewData.Description = InDescription;
	SetRelicViewData(NewViewData);
}

void UPBRelicTooltipWidget::SetRelicViewData(const FPBRelicViewData& InViewData)
{
	ViewData = InViewData;
	EnsureRelicViewModel();
	if (RelicViewModel)
	{
		RelicViewModel->SetRelicViewData(ViewData);
	}
}

void UPBRelicTooltipWidget::EnsureRelicViewModel()
{
	if (!RelicViewModel)
	{
		RelicViewModel = NewObject<UPBRelicViewModel>(this);
	}

	if (RelicViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBRelicTooltipWidget::ApplyViewModelToWidget()
{
	if (!RelicViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelicTooltipWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"), *GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(RelicViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelicTooltipWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"), *GetNameSafe(this), *GetNameSafe(RelicViewModel));
	}
	return bResult;
}
