#include "PBCollectionEntryViewModel.h"

void UPBCollectionEntryViewModel::SetDisplayData(const FPBCollectionDisplayData& InDisplayData)
{
	UE_MVVM_SET_PROPERTY_VALUE(CollectionId, InDisplayData.CollectionId);
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, InDisplayData.DisplayName);
	UE_MVVM_SET_PROPERTY_VALUE(MetaText, FText::Format(
		NSLOCTEXT("PBCollection", "EntryMetaFormat", "{0} · {1} · {2}성"),
		InDisplayData.CategoryText,
		InDisplayData.StateText,
		FText::AsNumber(InDisplayData.StarGrade)));
	UE_MVVM_SET_PROPERTY_VALUE(ShortDescription, InDisplayData.ShortDescription);
	UE_MVVM_SET_PROPERTY_VALUE(CardColor, ResolveCardColor(InDisplayData.State));
	UE_MVVM_SET_PROPERTY_VALUE(AccentColor, ResolveAccentColor(InDisplayData.Category, InDisplayData.State));
	UE_MVVM_SET_PROPERTY_VALUE(CategoryColor, ResolveCategoryColor(InDisplayData.Category, InDisplayData.State));
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

FLinearColor UPBCollectionEntryViewModel::ResolveCardColor(const EPBCollectionState State)
{
	return State == EPBCollectionState::Locked
		? FLinearColor(0.05f, 0.055f, 0.07f, 0.92f)
		: FLinearColor(0.07f, 0.105f, 0.13f, 0.96f);
}

FLinearColor UPBCollectionEntryViewModel::ResolveAccentColor(
	const EPBCollectionCategory Category,
	const EPBCollectionState State)
{
	if (State == EPBCollectionState::Locked)
	{
		return FLinearColor(0.28f, 0.28f, 0.32f, 1.0f);
	}

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
	const EPBCollectionCategory Category,
	const EPBCollectionState State)
{
	FLinearColor Color = ResolveAccentColor(Category, State);
	Color.A = State == EPBCollectionState::Locked ? 0.48f : 0.94f;
	return Color;
}
