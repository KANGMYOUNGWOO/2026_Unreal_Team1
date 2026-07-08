// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperListItemObject.h"

#include "Engine/Texture2D.h"

void UPBBumperListItemObject::SetBumperListItemData(
	const FName InRowName,
	const FText InDisplayName,
	const EPBBumperType InBumperType,
	const FPrimaryAssetId InPrimaryAssetId,
	UTexture2D* InIconTexture,
	const bool bInEquip)
{
	RowName = InRowName;
	BumperType = InBumperType;
	PrimaryAssetId = InPrimaryAssetId;

	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InDisplayName);
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, InIconTexture);
	SetEquip(bInEquip);
}

void UPBBumperListItemObject::SetEquip(const bool bInEquip)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsEquip, bInEquip);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetEquipFrameVisibility);
}

ESlateVisibility UPBBumperListItemObject::GetEquipFrameVisibility() const
{
	return bIsEquip
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;
}

void UPBBumperListItemObject::BroadcastClicked()
{
	OnClicked.Broadcast(RowName);
}
