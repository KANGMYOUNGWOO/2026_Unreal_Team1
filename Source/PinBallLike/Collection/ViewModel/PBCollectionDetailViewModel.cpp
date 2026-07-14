#include "PBCollectionDetailViewModel.h"

void UPBCollectionDetailViewModel::SetDisplayData(const FPBCollectionDisplayData& InDisplayData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasSelection, true);
	UE_MVVM_SET_PROPERTY_VALUE(bIsLoading, false);
	UE_MVVM_SET_PROPERTY_VALUE(NameText, InDisplayData.DisplayName);
	UE_MVVM_SET_PROPERTY_VALUE(MetaText, FText::Format(
		NSLOCTEXT("PBCollection", "DetailMetaFormat", "{0} · {1} · {2} · {3} · {4} · {5}성"),
		InDisplayData.CategoryText,
		InDisplayData.StateText,
		InDisplayData.AttackTypeText,
		InDisplayData.RoleText,
		InDisplayData.AttributeText,
		FText::AsNumber(InDisplayData.StarGrade)));
	UE_MVVM_SET_PROPERTY_VALUE(DescriptionText, InDisplayData.DetailDescription);
	UE_MVVM_SET_PROPERTY_VALUE(UnlockText, FText::Format(
		NSLOCTEXT("PBCollection", "UnlockFormat", "해금 조건: {0}"),
		InDisplayData.UnlockConditionText));
	UE_MVVM_SET_PROPERTY_VALUE(RecordText, InDisplayData.bCanShowFullData
		? InDisplayData.RecordText
		: NSLOCTEXT("PBCollection", "LockedRecordText", "기록은 해금 이후 표시됩니다."));
	UE_MVVM_SET_PROPERTY_VALUE(AccentColor, InDisplayData.State == EPBCollectionState::Locked
		? FLinearColor(0.22f, 0.22f, 0.22f, 1.0f)
		: InDisplayData.AccentColor);
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
