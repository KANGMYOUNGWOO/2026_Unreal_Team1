#include "PBCollectionDetailViewModel.h"

void UPBCollectionDetailViewModel::SetDisplayData(const FPBCollectionDisplayData& InDisplayData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasSelection, true);
	UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
	UE_MVVM_SET_PROPERTY_VALUE(NameText, InDisplayData.DisplayName);

	TArray<FString> MetadataParts;
	MetadataParts.Add(InDisplayData.CategoryText.ToString());
	if (!InDisplayData.AttackTypeId.IsNone())
	{
		MetadataParts.Add(InDisplayData.AttackTypeText.ToString());
	}
	if (!InDisplayData.RoleId.IsNone())
	{
		MetadataParts.Add(InDisplayData.RoleText.ToString());
	}
	if (!InDisplayData.AttributeId.IsNone())
	{
		MetadataParts.Add(InDisplayData.AttributeText.ToString());
	}
	if (InDisplayData.StarGrade > 0)
	{
		MetadataParts.Add(FString::Printf(TEXT("%d성"), InDisplayData.StarGrade));
	}
	UE_MVVM_SET_PROPERTY_VALUE(MetaText, FText::FromString(FString::Join(MetadataParts, TEXT(" · "))));
	UE_MVVM_SET_PROPERTY_VALUE(DescriptionText, InDisplayData.DetailDescription);
	UE_MVVM_SET_PROPERTY_VALUE(UnlockText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(RecordText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(AccentColor, InDisplayData.AccentColor);
}

void UPBCollectionDetailViewModel::SetEmpty(const bool bLoading)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasSelection, false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, bLoading);
	UE_MVVM_SET_PROPERTY_VALUE(NameText, bLoading
		? NSLOCTEXT("PBCollection", "LoadingName", "도감 불러오는 중")
		: NSLOCTEXT("PBCollection", "NoSelectionName", "도감"));
	UE_MVVM_SET_PROPERTY_VALUE(MetaText, bLoading
		? NSLOCTEXT("PBCollection", "LoadingMeta", "잠시만 기다려 주세요.")
		: NSLOCTEXT("PBCollection", "NoSelectionMeta", "항목 없음"));
	UE_MVVM_SET_PROPERTY_VALUE(DescriptionText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(UnlockText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(RecordText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(AccentColor, FLinearColor(0.22f, 0.22f, 0.22f, 1.0f));
}
