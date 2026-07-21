#include "PBCollectionCatalogEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "PBCollectionCatalogItemObject.h"

namespace
{
const FLinearColor CardColor(0.102f, 0.127f, 0.143f, 1.0f);
const FLinearColor SelectedCardColor(0.16f, 0.20f, 0.23f, 1.0f);
constexpr float IconFrameThickness = 2.0f;

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
		DefaultGlyphPadding = GlyphBorder->GetPadding();
		bHasDefaultGlyphStyle = true;

		if (WidgetTree)
		{
			GlyphIconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
			GlyphIconImage->SetClipping(EWidgetClipping::ClipToBounds);
			GlyphIconImage->SetColorAndOpacity(FLinearColor::White);
			GlyphIconImage->SetVisibility(ESlateVisibility::Collapsed);
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
		IconLetterText->SetColorAndOpacity(FSlateColor(Item->Summary.AccentColor));
	}
	if (IsValid(Item->Summary.IconTexture))
	{
		ApplyIconTexture(Item->Summary.IconTexture);
	}
	else
	{
		ApplyFallbackGlyph(Item->Category);
	}
	if (CardBorder)
	{
		CardBorder->SetBrushColor(CardColor);
		CardBorder->SetClipping(EWidgetClipping::ClipToBounds);
	}
}

void UPBCollectionCatalogEntryWidget::ApplyIconTexture(UTexture2D* IconTexture)
{
	if (!IsValid(GlyphBorder) || !IsValid(GlyphIconImage) || !IsValid(IconTexture))
	{
		return;
	}

	FSlateBrush FrameBrush;
	FrameBrush.TintColor = FSlateColor(FLinearColor::White);
	FrameBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
	GlyphBorder->SetBrush(FrameBrush);
	GlyphBorder->SetBrushColor(FLinearColor::White);
	GlyphBorder->SetPadding(FMargin(IconFrameThickness));
	GlyphBorder->SetClipping(EWidgetClipping::ClipToBounds);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(IconTexture);
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.TintColor = FSlateColor(FLinearColor::White);
	IconBrush.SetUVRegion(MakeCenteredSquareUV(IconTexture));
	GlyphIconImage->SetBrush(IconBrush);
	GlyphIconImage->SetColorAndOpacity(FLinearColor::White);
	GlyphIconImage->SetRenderOpacity(1.0f);
	GlyphIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetGlyphContent(GlyphIconImage);

	if (IconLetterText)
	{
		IconLetterText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPBCollectionCatalogEntryWidget::ApplyFallbackGlyph(const EPBCollectionCategory Category)
{
	if (GlyphIconImage)
	{
		GlyphIconImage->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (GlyphBorder && bHasDefaultGlyphStyle)
	{
		GlyphBorder->SetBrush(DefaultGlyphBrush);
		GlyphBorder->SetBrushColor(DefaultGlyphBrushColor);
		GlyphBorder->SetPadding(DefaultGlyphPadding);
		GlyphBorder->SetClipping(EWidgetClipping::ClipToBounds);
	}
	if (IconLetterText)
	{
		IconLetterText->SetText(GetCategoryGlyph(Category));
		IconLetterText->SetVisibility(ESlateVisibility::HitTestInvisible);
		SetGlyphContent(IconLetterText);
	}
}

void UPBCollectionCatalogEntryWidget::SetGlyphContent(UWidget* Content)
{
	if (!GlyphBorder || !Content || GlyphBorder->GetContent() == Content)
	{
		return;
	}

	GlyphBorder->ClearChildren();
	GlyphBorder->AddChild(Content);
	if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Content->Slot))
	{
		const bool bIsIcon = Content == GlyphIconImage;
		BorderSlot->SetHorizontalAlignment(bIsIcon ? HAlign_Fill : HAlign_Center);
		BorderSlot->SetVerticalAlignment(bIsIcon ? VAlign_Fill : VAlign_Center);
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
