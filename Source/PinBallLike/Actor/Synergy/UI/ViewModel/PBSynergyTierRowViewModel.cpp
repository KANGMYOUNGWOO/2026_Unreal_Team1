#include "PBSynergyTierRowViewModel.h"

void UPBSynergyTierRowViewModel::SetTierViewData(const FPBSynergyTierViewData& InViewData)
{
	UE_MVVM_SET_PROPERTY_VALUE(RequiredCount, InViewData.RequiredCount);
	UE_MVVM_SET_PROPERTY_VALUE(RequiredCountText, InViewData.RequiredCountText);
	UE_MVVM_SET_PROPERTY_VALUE(EffectText, InViewData.EffectText);
	UE_MVVM_SET_PROPERTY_VALUE(bIsActive, InViewData.bIsActive);
	UE_MVVM_SET_PROPERTY_VALUE(bIsCurrentTier, InViewData.bIsCurrentTier);
}

void UPBSynergyTierRowViewModel::ClearTier()
{
	UE_MVVM_SET_PROPERTY_VALUE(RequiredCount, 0);
	UE_MVVM_SET_PROPERTY_VALUE(RequiredCountText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(EffectText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(bIsActive, false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsCurrentTier, false);
}
