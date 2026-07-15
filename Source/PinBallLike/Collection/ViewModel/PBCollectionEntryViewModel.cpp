#include "PBCollectionEntryViewModel.h"

void UPBCollectionEntryViewModel::SetDisplayData(const FPBCollectionDisplayData& InDisplayData)
{
	UE_MVVM_SET_PROPERTY_VALUE(CollectionId, InDisplayData.CollectionId);
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InDisplayData.DisplayName);
	UE_MVVM_SET_PROPERTY_VALUE(MetaText, InDisplayData.StarGrade > 0
		? FText::Format(
			NSLOCTEXT("PBCollection", "EntryMetaFormat", "{0} · {1}성"),
			InDisplayData.CategoryText,
			FText::AsNumber(InDisplayData.StarGrade))
		: InDisplayData.CategoryText);
	UE_MVVM_SET_PROPERTY_VALUE(ShortDescription, InDisplayData.ShortDescription);
	UE_MVVM_SET_PROPERTY_VALUE(CardColor, ResolveCardColor());
	UE_MVVM_SET_PROPERTY_VALUE(AccentColor, ResolveAccentColor(InDisplayData.Category));
	UE_MVVM_SET_PROPERTY_VALUE(CategoryColor, ResolveCategoryColor(InDisplayData.Category));
}

void UPBCollectionEntryViewModel::Clear()
{
	UE_MVVM_SET_PROPERTY_VALUE(CollectionId, NAME_None);
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(MetaText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(ShortDescription, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(CardColor, FLinearColor::Transparent);
	UE_MVVM_SET_PROPERTY_VALUE(AccentColor, FLinearColor::Transparent);
	UE_MVVM_SET_PROPERTY_VALUE(CategoryColor, FLinearColor::Transparent);
}

FLinearColor UPBCollectionEntryViewModel::ResolveCardColor()
{
	return FLinearColor(0.07f, 0.105f, 0.13f, 0.96f);
}

FLinearColor UPBCollectionEntryViewModel::ResolveAccentColor(
	const EPBCollectionCategory Category)
{
	switch (Category)
	{
	case EPBCollectionCategory::Ball:
		return FLinearColor(0.88f, 0.22f, 0.18f, 1.0f);
	case EPBCollectionCategory::Bumper:
		return FLinearColor(0.12f, 0.72f, 0.86f, 1.0f);
	case EPBCollectionCategory::Boss:
		return FLinearColor(0.72f, 0.24f, 0.86f, 1.0f);
	case EPBCollectionCategory::Relic:
		return FLinearColor(0.92f, 0.66f, 0.22f, 1.0f);
	case EPBCollectionCategory::Achievement:
		return FLinearColor(0.34f, 0.72f, 0.42f, 1.0f);
	default:
		return FLinearColor(0.12f, 0.72f, 0.86f, 1.0f);
	}
}

FLinearColor UPBCollectionEntryViewModel::ResolveCategoryColor(
	const EPBCollectionCategory Category)
{
	FLinearColor Color = ResolveAccentColor(Category);
	Color.A = 0.94f;
	return Color;
}
