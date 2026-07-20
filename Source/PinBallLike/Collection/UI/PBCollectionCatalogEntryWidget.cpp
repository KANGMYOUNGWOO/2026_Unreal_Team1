#include "PBCollectionCatalogEntryWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "PBCollectionCatalogItemObject.h"

namespace
{
const FLinearColor CardColor(0.102f, 0.127f, 0.143f, 1.0f);
const FLinearColor SelectedCardColor(0.16f, 0.20f, 0.23f, 1.0f);

FText GetCategoryGlyph(const EPBCollectionCategory Category)
{
	switch (Category)
	{
	case EPBCollectionCategory::Ball: return FText::FromString(TEXT("B"));
	case EPBCollectionCategory::Synergy: return FText::FromString(TEXT("S"));
	case EPBCollectionCategory::Relic: return FText::FromString(TEXT("R"));
	case EPBCollectionCategory::Bumper: return FText::FromString(TEXT("P"));
	case EPBCollectionCategory::Boss: return FText::FromString(TEXT("!"));
	default: return FText::FromString(TEXT("C"));
	}
}
}

void UPBCollectionCatalogEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UPBCollectionCatalogItemObject* Item = Cast<UPBCollectionCatalogItemObject>(ListItemObject);
	if (!Item)
	{
		return;
	}

	if (NameText)
	{
		NameText->SetText(Item->Summary.DisplayName);
		NameText->SetAutoWrapText(false);
		NameText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		NameText->SetClipping(EWidgetClipping::ClipToBounds);
		NameText->SetToolTipText(Item->Summary.DisplayName);
	}
	if (SubtitleText)
	{
		SubtitleText->SetText(Item->Summary.Subtitle);
		SubtitleText->SetAutoWrapText(false);
		SubtitleText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		SubtitleText->SetClipping(EWidgetClipping::ClipToBounds);
		SubtitleText->SetToolTipText(Item->Summary.Subtitle);
		SubtitleText->SetVisibility(Item->Summary.Subtitle.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (AccentBorder)
	{
		AccentBorder->SetBrushColor(Item->Summary.AccentColor);
	}
	if (IconLetterText)
	{
		IconLetterText->SetText(GetCategoryGlyph(Item->Category));
		IconLetterText->SetColorAndOpacity(FSlateColor(Item->Summary.AccentColor));
	}
	if (CardBorder)
	{
		CardBorder->SetBrushColor(CardColor);
		CardBorder->SetClipping(EWidgetClipping::ClipToBounds);
	}
}

void UPBCollectionCatalogEntryWidget::NativeOnItemSelectionChanged(const bool bIsSelected)
{
	IUserObjectListEntry::NativeOnItemSelectionChanged(bIsSelected);
	if (CardBorder)
	{
		CardBorder->SetBrushColor(bIsSelected ? SelectedCardColor : CardColor);
	}
}
