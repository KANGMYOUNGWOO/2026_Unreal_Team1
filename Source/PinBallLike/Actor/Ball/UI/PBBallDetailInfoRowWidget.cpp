#include "PBBallDetailInfoRowWidget.h"

#include "Components/TextBlock.h"
#include "PBBallDetailIconTextTagWidget.h"

void UPBBallDetailInfoRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPBBallDetailInfoRowWidget::SetInfoRow(const FPBBallDetailInfoRowViewData& InViewData)
{
	SetInfoText(InViewData.LabelText, InViewData.ValueText);
}

void UPBBallDetailInfoRowWidget::SetInfoText(const FText& InLabelText, const FText& InValueText)
{
	if (Text_Label)
	{
		Text_Label->SetText(InLabelText);
	}

	if (Custom_Value)
	{
		Custom_Value->SetIconTextData(nullptr, InValueText);
	}
}

void UPBBallDetailInfoRowWidget::SetIconTextRow(
	const FText& InLabelText,
	const FPBBallDetailIconTextViewData& InViewData)
{
	if (Text_Label)
	{
		Text_Label->SetText(InLabelText);
	}

	if (Custom_Value)
	{
		Custom_Value->SetIconText(InViewData);
	}
}

void UPBBallDetailInfoRowWidget::ClearInfoRow()
{
	SetInfoText(FText::GetEmpty(), FText::GetEmpty());
}
