#include "PBBumperInfoPanelViewModel.h"

#include "Engine/Texture2D.h"

void UPBBumperInfoPanelViewModel::SetBumperInfoPanelData(
	const FText InTitleName,
	UTexture2D* InIconImage,
	const FText InDescription,
	const bool bInEquip)
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleName, InTitleName);
	UE_MVVM_SET_PROPERTY_VALUE(IconImage, InIconImage);
	UE_MVVM_SET_PROPERTY_VALUE(Description, InDescription);
	SetEquip(bInEquip);
}

void UPBBumperInfoPanelViewModel::ClearBumperInfoPanelData()
{
	UE_MVVM_SET_PROPERTY_VALUE(TitleName, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(IconImage, nullptr);
	UE_MVVM_SET_PROPERTY_VALUE(Description, FText::GetEmpty());
	SetEquip(false);
}

void UPBBumperInfoPanelViewModel::SetEquip(const bool bInEquip)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsEquip, bInEquip);
	UE_MVVM_SET_PROPERTY_VALUE(bIsNotEquip, !bInEquip);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetIsEquipVisibility);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetIsNotEquipVisibility);
}

ESlateVisibility UPBBumperInfoPanelViewModel::GetIsEquipVisibility() const
{
	return bIsEquip
		? ESlateVisibility::Visible
		: ESlateVisibility::Collapsed;
}

ESlateVisibility UPBBumperInfoPanelViewModel::GetIsNotEquipVisibility() const
{
	return bIsNotEquip
		? ESlateVisibility::Visible
		: ESlateVisibility::Collapsed;
}
