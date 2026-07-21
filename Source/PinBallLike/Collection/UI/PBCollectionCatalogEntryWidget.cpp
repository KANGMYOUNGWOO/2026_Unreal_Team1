#include "PBCollectionCatalogEntryWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
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

FBox2f MakeCenteredSquareUV(const UTexture2D* Texture)
{
	const float Width = Texture ? static_cast<float>(Texture->GetSizeX()) : 1.0f;
	const float Height = Texture ? static_cast<float>(Texture->GetSizeY()) : 1.0f;
	if (Width > Height)
	{
		const float HorizontalInset = (1.0f - Height / Width) * 0.5f;
		return FBox2f(FVector2f(HorizontalInset, 0.0f), FVector2f(1.0f - HorizontalInset, 1.0f));
	}
	if (Height > Width)
	{
		const float VerticalInset = (1.0f - Width / Height) * 0.5f;
		return FBox2f(FVector2f(0.0f, VerticalInset), FVector2f(1.0f, 1.0f - VerticalInset));
	}

	return FBox2f(FVector2f::ZeroVector, FVector2f(1.0f, 1.0f));
}
}

void UPBCollectionCatalogEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (GlyphBorder)
	{
		DefaultGlyphBrush = GlyphBorder->Background;
		DefaultGlyphBrushColor = GlyphBorder->GetBrushColor();
		bHasDefaultGlyphStyle = true;
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
	const bool bHasIcon = IsValid(Item->Summary.IconTexture);
	if (GlyphBorder)
	{
		if (bHasIcon)
		{
			FSlateBrush IconBrush = DefaultGlyphBrush;
			IconBrush.SetResourceObject(Item->Summary.IconTexture);
			IconBrush.DrawAs = ESlateBrushDrawType::Image;
			IconBrush.Margin = FMargin(0.0f);
			IconBrush.SetUVRegion(MakeCenteredSquareUV(Item->Summary.IconTexture));
			GlyphBorder->SetBrush(IconBrush);
			GlyphBorder->SetBrushColor(FLinearColor::White);
		}
		else if (bHasDefaultGlyphStyle)
		{
			GlyphBorder->SetBrush(DefaultGlyphBrush);
			GlyphBorder->SetBrushColor(DefaultGlyphBrushColor);
		}
		GlyphBorder->SetClipping(EWidgetClipping::ClipToBounds);
	}
	if (IconLetterText)
	{
		IconLetterText->SetText(GetCategoryGlyph(Item->Category));
		IconLetterText->SetColorAndOpacity(FSlateColor(Item->Summary.AccentColor));
		IconLetterText->SetVisibility(bHasIcon ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
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
