#include "PBCollectionEntryWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

namespace
{
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

	if (EntryButton)
	{
		EntryButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionEntryWidget::HandleClicked);
	}

	ValidateRequiredWidgetBindings();
	Refresh();
}

bool UPBCollectionEntryWidget::ValidateRequiredWidgetBindings() const
{
	bool bAllWidgetsBound = true;
	const auto CheckBinding = [this, &bAllWidgetsBound](const UObject* Widget, const TCHAR* WidgetName)
	{
		if (!IsValid(Widget))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("%s: Widget Blueprint에 필수 위젯 '%s'가 없거나 이름/타입이 일치하지 않습니다."),
				*GetName(),
				WidgetName);
			bAllWidgetsBound = false;
		}
	};

	CheckBinding(EntryButton, TEXT("EntryButton"));
	CheckBinding(CardBorder, TEXT("CardBorder"));
	CheckBinding(AccentBorder, TEXT("AccentBorder"));
	CheckBinding(CategoryColorBorder, TEXT("CategoryColorBorder"));
	CheckBinding(NameText, TEXT("NameText"));
	CheckBinding(MetaText, TEXT("MetaText"));

	return bAllWidgetsBound;
}

void UPBCollectionEntryWidget::SetDisplayData(const FPBCollectionDisplayData& InDisplayData)
{
	DisplayData = InDisplayData;
	Refresh();
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
		CategoryColorBorder->SetBrushColor(GetIconColor(DisplayData.Category, DisplayData.State));
	}

	if (AccentBorder)
	{
		AccentBorder->SetBrushColor(GetAccentColor(DisplayData.Category, DisplayData.State));
	}

	if (CardBorder)
	{
		CardBorder->SetBrushColor(GetCardColor(DisplayData.State));
	}

	BP_OnCollectionDisplayDataChanged(DisplayData);
}

void UPBCollectionEntryWidget::HandleClicked()
{
	OnEntryClicked.Broadcast(DisplayData.CollectionId);
}
