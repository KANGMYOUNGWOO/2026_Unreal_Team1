#include "PBSynergyDetailPanelViewModel.h"

void UPBSynergyDetailPanelViewModel::SetSynergyViewData(const FPBSynergyViewData& InViewData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasSynergy, InViewData.IsValid());
	UE_MVVM_SET_PROPERTY_VALUE(SynergyId, InViewData.SynergyId);
	UE_MVVM_SET_PROPERTY_VALUE(SynergyNameText, InViewData.SynergyName);
	UE_MVVM_SET_PROPERTY_VALUE(DescriptionText, InViewData.Description);
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCount, InViewData.CurrentCount);
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCountText, InViewData.CurrentCountText);
	UE_MVVM_SET_PROPERTY_VALUE(CountListText, InViewData.CountListText);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, InViewData.Icon);
	UE_MVVM_SET_PROPERTY_VALUE(TierViewDataList, InViewData.TierViewDataList);
	UE_MVVM_SET_PROPERTY_VALUE(BallIconViewDataList, InViewData.BallIconViewDataList);
}

void UPBSynergyDetailPanelViewModel::ClearSynergy()
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasSynergy, false);
	UE_MVVM_SET_PROPERTY_VALUE(SynergyId, NAME_None);
	UE_MVVM_SET_PROPERTY_VALUE(SynergyNameText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(DescriptionText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCount, 0);
	UE_MVVM_SET_PROPERTY_VALUE(CurrentCountText, FText::AsNumber(0));
	UE_MVVM_SET_PROPERTY_VALUE(CountListText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, nullptr);

	TArray<FPBSynergyTierViewData> EmptyTierViewDataList;
	UE_MVVM_SET_PROPERTY_VALUE(TierViewDataList, EmptyTierViewDataList);

	TArray<FPBSynergyBallIconViewData> EmptyBallIconViewDataList;
	UE_MVVM_SET_PROPERTY_VALUE(BallIconViewDataList, EmptyBallIconViewDataList);
}
