#include "PBStatusEffectItemViewModel.h"

void UPBStatusEffectItemViewModel::SetStatusEffectViewData(const FPBStatusEffectViewData& InViewData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasStatusEffect, InViewData.IsValid());
	UE_MVVM_SET_PROPERTY_VALUE(StatusEffectId, InViewData.StatusEffectId);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, InViewData.Icon);
	SetStackCount(InViewData.StackCount);
}

void UPBStatusEffectItemViewModel::SetStackCount(const int32 InStackCount)
{
	const int32 ClampedStackCount = FMath::Max(InStackCount, 0);
	UE_MVVM_SET_PROPERTY_VALUE(StackCount, ClampedStackCount);
	UE_MVVM_SET_PROPERTY_VALUE(StackCountText, FText::AsNumber(ClampedStackCount));
}

void UPBStatusEffectItemViewModel::ClearStatusEffect()
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasStatusEffect, false);
	UE_MVVM_SET_PROPERTY_VALUE(StatusEffectId, NAME_None);
	UE_MVVM_SET_PROPERTY_VALUE(StackCount, 0);
	UE_MVVM_SET_PROPERTY_VALUE(StackCountText, FText::AsNumber(0));
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, nullptr);
}
