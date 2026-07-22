#include "PBRelicViewModel.h"

void UPBRelicViewModel::SetRelicViewData(const FPBRelicViewData& InViewData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasRelic, InViewData.IsValid());
	UE_MVVM_SET_PROPERTY_VALUE(RewardType, InViewData.RewardType);
	UE_MVVM_SET_PROPERTY_VALUE(RelicId, InViewData.RelicId);
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InViewData.DisplayName);
	UE_MVVM_SET_PROPERTY_VALUE(Description, InViewData.Description);
	UE_MVVM_SET_PROPERTY_VALUE(Rarity, InViewData.Rarity);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, InViewData.IconTexture);
	UE_MVVM_SET_PROPERTY_VALUE(GoldAmount, InViewData.GoldAmount);
}

void UPBRelicViewModel::ClearRelic()
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasRelic, false);
	UE_MVVM_SET_PROPERTY_VALUE(RewardType, EPBRelicChoiceRewardType::Relic);
	UE_MVVM_SET_PROPERTY_VALUE(RelicId, NAME_None);
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(Description, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(Rarity, EPBRelicRarity::Common);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, nullptr);
	UE_MVVM_SET_PROPERTY_VALUE(GoldAmount, 0);
}
