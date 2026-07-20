#include "PBSynergyPanelViewModel.h"

void UPBSynergyPanelViewModel::SetSynergyViewData(const FPBSynergyViewData& InViewData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasSynergy, InViewData.IsValid());
	UE_MVVM_SET_PROPERTY_VALUE(SynergyId, InViewData.SynergyId);
	UE_MVVM_SET_PROPERTY_VALUE(SynergyNameText, InViewData.SynergyName);
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCount, InViewData.CurrentCount);
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCountText, InViewData.CurrentCountText);
	UE_MVVM_SET_PROPERTY_VALUE(CountListText, InViewData.CountListText);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, InViewData.Icon);
}

void UPBSynergyPanelViewModel::ClearSynergy()
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasSynergy, false);
	UE_MVVM_SET_PROPERTY_VALUE(SynergyId, NAME_None);
	UE_MVVM_SET_PROPERTY_VALUE(SynergyNameText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCount, 0);
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCountText, FText::AsNumber(0));
	UE_MVVM_SET_PROPERTY_VALUE(CountListText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, nullptr);
}
