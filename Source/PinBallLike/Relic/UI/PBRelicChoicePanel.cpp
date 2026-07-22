#include "PBRelicChoicePanel.h"

#include "InputCoreTypes.h"
#include "PBRelicViewModel.h"
#include "View/MVVMView.h"

void UPBRelicChoicePanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureRelicViewModel();
}

void UPBRelicChoicePanel::SetRelicId(const FName InRelicId)
{
	FPBRelicViewData NewViewData;
	NewViewData.RelicId = InRelicId;
	SetRelicViewData(NewViewData);
}

void UPBRelicChoicePanel::SetRelicViewData(const FPBRelicViewData& InViewData)
{
	ViewData = InViewData;
	EnsureRelicViewModel();
	if (RelicViewModel)
	{
		RelicViewModel->SetRelicViewData(ViewData);
	}
}

FName UPBRelicChoicePanel::GetRelicId() const
{
	return ViewData.RelicId;
}

FReply UPBRelicChoicePanel::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	OnClicked.Broadcast(this);
	return FReply::Handled();
}

void UPBRelicChoicePanel::EnsureRelicViewModel()
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

bool UPBRelicChoicePanel::ApplyViewModelToWidget()
{
	if (!RelicViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelicChoicePanel ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"), *GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(RelicViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("RelicChoicePanel ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"), *GetNameSafe(this), *GetNameSafe(RelicViewModel));
	}
	return bResult;
}
