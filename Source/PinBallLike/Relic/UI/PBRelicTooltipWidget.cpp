#include "PBRelicTooltipWidget.h"

#include "Components/TextBlock.h"

void UPBRelicTooltipWidget::SetRelicData(
	const FText& InDisplayName,
	const FText& InDescription)
{
	if (RelicNameText)
	{
		RelicNameText->SetText(
			InDisplayName);
	}

	if (RelicDescriptionText)
	{
		RelicDescriptionText->SetText(
			InDescription);
	}
}