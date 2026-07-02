#include "PBCollectionEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
constexpr float CardWidth = 248.0f;
constexpr float CardHeight = 248.0f;
constexpr float CardInnerPadding = 28.0f;

FSlateBrush MakeColorBrush(const FLinearColor& Color, const FVector2D& ImageSize)
{
	FSlateBrush Brush;
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Box;
	Brush.TintColor = FSlateColor(Color);
	return Brush;
}

FButtonStyle MakeTransparentButtonStyle()
{
	FSlateBrush NoDrawBrush;
	NoDrawBrush.DrawAs = ESlateBrushDrawType::NoDrawType;

	FButtonStyle Style;
	Style.SetNormal(NoDrawBrush);
	Style.SetHovered(NoDrawBrush);
	Style.SetPressed(NoDrawBrush);
	Style.SetDisabled(NoDrawBrush);
	Style.SetNormalPadding(FMargin(0.0f));
	Style.SetPressedPadding(FMargin(0.0f));
	return Style;
}

FLinearColor GetCardColor(EPBCollectionState State)
{
	return State == EPBCollectionState::Locked
		? FLinearColor(0.05f, 0.055f, 0.07f, 0.92f)
		: FLinearColor(0.07f, 0.105f, 0.13f, 0.96f);
}

FLinearColor GetAccentColor(EPBCollectionCategory Category, EPBCollectionState State)
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

FLinearColor GetIconColor(EPBCollectionCategory Category, EPBCollectionState State)
{
	FLinearColor Color = GetAccentColor(Category, State);
	Color.A = State == EPBCollectionState::Locked ? 0.48f : 0.94f;
	return Color;
}
}

void UPBCollectionEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildDefaultWidgetTree();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionEntryWidget::HandleClicked);
	}

	Refresh();
}

void UPBCollectionEntryWidget::SetDisplayData(const FPBCollectionDisplayData& InDisplayData)
{
	DisplayData = InDisplayData;
	Refresh();
}

void UPBCollectionEntryWidget::BuildDefaultWidgetTree()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RootSizeBox"));
	RootSizeBox->SetWidthOverride(CardWidth);
	RootSizeBox->SetHeightOverride(CardHeight);
	WidgetTree->RootWidget = RootSizeBox;

	EntryButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EntryButton"));
	EntryButton->SetStyle(MakeTransparentButtonStyle());
	EntryButton->SetBackgroundColor(FLinearColor::Transparent);
	EntryButton->SetColorAndOpacity(FLinearColor::White);
	RootSizeBox->AddChild(EntryButton);

	CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CardBorder"));
	CardBorder->SetBrush(MakeColorBrush(GetCardColor(EPBCollectionState::Unlocked), FVector2D(CardWidth, CardHeight)));
	CardBorder->SetPadding(FMargin(CardInnerPadding, 30.0f, CardInnerPadding, 24.0f));
	EntryButton->AddChild(CardBorder);
	if (UButtonSlot* EntryButtonSlot = Cast<UButtonSlot>(CardBorder->Slot))
	{
		EntryButtonSlot->SetPadding(FMargin(0.0f));
		EntryButtonSlot->SetHorizontalAlignment(HAlign_Fill);
		EntryButtonSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* CardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardBox"));
	CardBorder->SetContent(CardBox);

	AccentBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AccentBorder"));
	AccentBorder->SetBrush(MakeColorBrush(
		GetAccentColor(EPBCollectionCategory::Bumper, EPBCollectionState::Unlocked),
		FVector2D(CardWidth - (CardInnerPadding * 2.0f), 14.0f)));
	CardBox->AddChildToVerticalBox(AccentBorder);
	if (UVerticalBoxSlot* AccentSlot = Cast<UVerticalBoxSlot>(AccentBorder->Slot))
	{
		AccentSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		AccentSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		AccentSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IconSizeBox"));
	IconSizeBox->SetWidthOverride(82.0f);
	IconSizeBox->SetHeightOverride(82.0f);
	CardBox->AddChildToVerticalBox(IconSizeBox);
	if (UVerticalBoxSlot* IconSlot = Cast<UVerticalBoxSlot>(IconSizeBox->Slot))
	{
		IconSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		IconSlot->SetHorizontalAlignment(HAlign_Center);
		IconSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	}

	CategoryColorBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CategoryColorBorder"));
	CategoryColorBorder->SetBrush(MakeColorBrush(
		GetIconColor(EPBCollectionCategory::Bumper, EPBCollectionState::Unlocked),
		FVector2D(82.0f, 82.0f)));
	IconSizeBox->AddChild(CategoryColorBorder);

	NameText = CreateText(WidgetTree, TEXT("NameText"), 18);
	CardBox->AddChildToVerticalBox(NameText);
	if (UVerticalBoxSlot* NameSlot = Cast<UVerticalBoxSlot>(NameText->Slot))
	{
		NameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 3.0f));
		NameSlot->SetHorizontalAlignment(HAlign_Center);
	}

	MetaText = CreateText(WidgetTree, TEXT("MetaText"), 12);
	CardBox->AddChildToVerticalBox(MetaText);
	if (UVerticalBoxSlot* MetaSlot = Cast<UVerticalBoxSlot>(MetaText->Slot))
	{
		MetaSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
		MetaSlot->SetHorizontalAlignment(HAlign_Center);
	}

	DescriptionText = CreateText(WidgetTree, TEXT("DescriptionText"), 12);
	DescriptionText->SetAutoWrapText(true);
	DescriptionText->SetVisibility(ESlateVisibility::Collapsed);
	CardBox->AddChildToVerticalBox(DescriptionText);
	if (UVerticalBoxSlot* DescriptionSlot = Cast<UVerticalBoxSlot>(DescriptionText->Slot))
	{
		DescriptionSlot->SetPadding(FMargin(0.0f));
		DescriptionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
}

void UPBCollectionEntryWidget::Refresh()
{
	if (NameText)
	{
		NameText->SetText(DisplayData.DisplayName);
	}

	if (MetaText)
	{
		MetaText->SetText(FText::Format(
			NSLOCTEXT("PBCollection", "EntryMetaFormat", "{0} · {1} · {2}성"),
			DisplayData.CategoryText,
			DisplayData.StateText,
			FText::AsNumber(DisplayData.StarGrade)));
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(DisplayData.ShortDescription);
	}

	if (CategoryColorBorder)
	{
		CategoryColorBorder->SetBrush(MakeColorBrush(
			GetIconColor(DisplayData.Category, DisplayData.State),
			FVector2D(82.0f, 82.0f)));
	}

	if (AccentBorder)
	{
		AccentBorder->SetBrush(MakeColorBrush(
			GetAccentColor(DisplayData.Category, DisplayData.State),
			FVector2D(CardWidth - (CardInnerPadding * 2.0f), 14.0f)));
	}

	if (CardBorder)
	{
		CardBorder->SetBrush(MakeColorBrush(GetCardColor(DisplayData.State), FVector2D(CardWidth, CardHeight)));
	}
}

void UPBCollectionEntryWidget::HandleClicked()
{
	OnEntryClicked.Broadcast(DisplayData.CollectionId);
}

UTextBlock* UPBCollectionEntryWidget::CreateText(UWidgetTree* InWidgetTree, FName WidgetName, int32 FontSize)
{
	UTextBlock* TextBlock = InWidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.94f, 0.96f, 1.0f)));
	TextBlock->SetAutoWrapText(false);
	TextBlock->SetMinDesiredWidth(0.0f);
	TextBlock->SetJustification(ETextJustify::Center);
	TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));

	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = FontSize;
	TextBlock->SetFont(FontInfo);

	return TextBlock;
}
