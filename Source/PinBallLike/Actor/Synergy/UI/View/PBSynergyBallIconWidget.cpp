#include "PBSynergyBallIconWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"

void UPBSynergyBallIconWidget::InitializeBallIcon(const FPBSynergyBallIconViewData& InViewData)
{
	ViewData = InViewData;
	ApplyIcon();
}

void UPBSynergyBallIconWidget::ApplyIcon()
{
	UImage* IconImage = WidgetTree
		? Cast<UImage>(WidgetTree->FindWidget(TEXT("Image_BallIcon")))
		: nullptr;

	if (!IconImage)
	{
		IconImage = WidgetTree
			? Cast<UImage>(WidgetTree->FindWidget(TEXT("Image_Icon")))
			: nullptr;
	}

	if (!IconImage)
	{
		return;
	}

	IconImage->SetBrushFromTexture(ViewData.Icon);
	IconImage->SetVisibility(ViewData.Icon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
