#include "PBBallDetailTooltipWidget.h"

#include "PBBallDetailInfoRowWidget.h"
#include "PBBallDetailTagListRowWidget.h"
#include "PBBallDetailTooltipViewModel.h"
#include "View/MVVMView.h"

void UPBBallDetailTooltipWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureViewModel();
}

void UPBBallDetailTooltipWidget::SetTooltipViewData(const FPBBallDetailTooltipViewData& InViewData)
{
	ViewData = InViewData;
	EnsureViewModel();

	if (TooltipViewModel)
	{
		TooltipViewModel->SetTooltipViewData(ViewData);
	}

	ApplyViewDataToChildWidgets();
	BP_OnTooltipViewDataChanged(ViewData);
}

void UPBBallDetailTooltipWidget::ClearTooltip()
{
	ViewData = FPBBallDetailTooltipViewData();
	EnsureViewModel();

	if (TooltipViewModel)
	{
		TooltipViewModel->ClearTooltip();
	}

	ApplyViewDataToChildWidgets();
	BP_OnTooltipCleared();
}

void UPBBallDetailTooltipWidget::EnsureViewModel()
{
	if (!TooltipViewModel)
	{
		TooltipViewModel = NewObject<UPBBallDetailTooltipViewModel>(this);
	}

	if (TooltipViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBBallDetailTooltipWidget::ApplyViewModelToWidget()
{
	if (!TooltipViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDetailTooltipWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(TooltipViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDetailTooltipWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(TooltipViewModel));
	}
	return bResult;
}

void UPBBallDetailTooltipWidget::ApplyViewDataToChildWidgets()
{
	if (InfoRow_HP)
	{
		InfoRow_HP->SetInfoRow(ViewData.HpRow);
	}

	if (InfoRow_MP)
	{
		InfoRow_MP->SetInfoRow(ViewData.MpRow);
	}

	if (InfoRow_Attack)
	{
		InfoRow_Attack->SetInfoRow(ViewData.AttackRow);
	}

	if (InfoRow_ManaRegen)
	{
		InfoRow_ManaRegen->SetInfoRow(ViewData.ManaRegenRow);
	}

	if (Tag_Type)
	{
		Tag_Type->SetIconTextRow(NSLOCTEXT("BallDetailTooltip", "TypeLabel", "타입"), ViewData.PowerFlipData);
	}

	if (Tag_Job)
	{
		Tag_Job->SetIconTextRow(NSLOCTEXT("BallDetailTooltip", "JobLabel", "직업"), ViewData.ClassData);
	}

	if (TagListRow_Race)
	{
		TagListRow_Race->SetTagList(NSLOCTEXT("BallDetailTooltip", "RaceLabel", "종족"), ViewData.RaceDataList);
	}
}
