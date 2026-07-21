#include "PBBallDetailIconTextTagWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UPBBallDetailIconTextTagWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPBBallDetailIconTextTagWidget::SetIconText(const FPBBallDetailIconTextViewData& InViewData)
{
	SetIconTextData(InViewData.IconTexture, InViewData.Text);
}

void UPBBallDetailIconTextTagWidget::SetIconTextData(UTexture2D* InIconTexture, const FText& InText)
{
	if (Image_Icon)
	{
		Image_Icon->SetBrushFromTexture(InIconTexture);
		Image_Icon->SetVisibility(InIconTexture ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_Name)
	{
		Text_Name->SetText(InText);
	}
}

void UPBBallDetailIconTextTagWidget::ClearIconText()
{
	SetIconTextData(nullptr, FText::GetEmpty());
}
