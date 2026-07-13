#include "PBStatusEffectItemWidget.h"

#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/UI/ViewModel/PBStatusEffectItemViewModel.h"
#include "View/MVVMView.h"

void UPBStatusEffectItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureItemViewModel();
}

void UPBStatusEffectItemWidget::InitializeStatusEffectItem(const FPBStatusEffectViewData& InViewData)
{
	ViewData = InViewData;
	EnsureItemViewModel();
	if (ItemViewModel)
	{
		ItemViewModel->SetStatusEffectViewData(ViewData);
	}
}

void UPBStatusEffectItemWidget::InitializeStatusEffectItemFromEffect(
	UPBBaseStatusEffect* InStatusEffect,
	UTexture2D* InIcon)
{
	FPBStatusEffectViewData NewViewData;
	if (IsValid(InStatusEffect))
	{
		NewViewData.StatusEffectId = InStatusEffect->GetStatusEffectId();
		NewViewData.StackCount = InStatusEffect->GetStackCount();
		NewViewData.Icon = InIcon;
	}

	InitializeStatusEffectItem(NewViewData);
}

void UPBStatusEffectItemWidget::RefreshStackCount(const int32 InStackCount)
{
	ViewData.StackCount = FMath::Max(InStackCount, 0);
	if (ItemViewModel)
	{
		ItemViewModel->SetStackCount(ViewData.StackCount);
	}
}

void UPBStatusEffectItemWidget::ClearStatusEffectItem()
{
	ViewData = FPBStatusEffectViewData();
	EnsureItemViewModel();
	if (ItemViewModel)
	{
		ItemViewModel->ClearStatusEffect();
	}
}

void UPBStatusEffectItemWidget::EnsureItemViewModel()
{
	if (!ItemViewModel)
	{
		ItemViewModel = NewObject<UPBStatusEffectItemViewModel>(this);
	}

	if (ItemViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBStatusEffectItemWidget::ApplyViewModelToWidget()
{
	if (!ItemViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("StatusEffectItemWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ItemViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("StatusEffectItemWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(ItemViewModel));
	}
	return bResult;
}
