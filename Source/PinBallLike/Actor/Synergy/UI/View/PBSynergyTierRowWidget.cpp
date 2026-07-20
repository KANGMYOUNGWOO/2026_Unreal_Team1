#include "PBSynergyTierRowWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "PinBallLike/Actor/Synergy/UI/ViewModel/PBSynergyTierRowViewModel.h"
#include "View/MVVMView.h"

void UPBSynergyTierRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureViewModel();
}

void UPBSynergyTierRowWidget::InitializeTierRow(const FPBSynergyTierViewData& InViewData)
{
	ViewData = InViewData;

	EnsureViewModel();
	if (ViewModel)
	{
		ViewModel->SetTierViewData(ViewData);
	}

	ApplyTierStyle();
}

void UPBSynergyTierRowWidget::EnsureViewModel()
{
	if (!ViewModel)
	{
		ViewModel = NewObject<UPBSynergyTierRowViewModel>(this);
	}

	if (ViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBSynergyTierRowWidget::ApplyViewModelToWidget()
{
	if (!ViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("SynergyTierRowWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("SynergyTierRowWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(ViewModel));
	}

	return bResult;
}

void UPBSynergyTierRowWidget::ApplyTierStyle()
{
	const FSlateColor TierTextColor = ViewData.bIsCurrentTier
		? GetActiveTierTextColor()
		: GetInactiveTierTextColor();

	UTextBlock* RequiredCountTextBlock = WidgetTree
		? Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RequiredCount")))
		: nullptr;
	if (RequiredCountTextBlock)
	{
		RequiredCountTextBlock->SetColorAndOpacity(TierTextColor);
	}

	UTextBlock* TierEffectTextBlock = WidgetTree
		? Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_TierEffect")))
		: nullptr;
	if (TierEffectTextBlock)
	{
		TierEffectTextBlock->SetColorAndOpacity(TierTextColor);
	}
}

FSlateColor UPBSynergyTierRowWidget::GetActiveTierTextColor()
{
	return FSlateColor(FLinearColor(0.0f, 0.85f, 0.45f, 1.0f));
}

FSlateColor UPBSynergyTierRowWidget::GetInactiveTierTextColor()
{
	return FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
}
