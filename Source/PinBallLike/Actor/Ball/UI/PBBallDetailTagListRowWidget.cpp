#include "PBBallDetailTagListRowWidget.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "PBBallDetailIconTextTagWidget.h"

void UPBBallDetailTagListRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPBBallDetailTagListRowWidget::SetTagList(
	const FText& InLabelText,
	const TArray<FPBBallDetailIconTextViewData>& InTagDataList)
{
	if (Text_Label)
	{
		Text_Label->SetText(InLabelText);
	}

	if (!HorizontalBox_TagList)
	{
		return;
	}

	for (int32 TagIndex = 0; TagIndex < InTagDataList.Num(); ++TagIndex)
	{
		UPBBallDetailIconTextTagWidget* TagWidget = GetOrCreateTagWidget(TagIndex);
		if (!TagWidget)
		{
			continue;
		}

		TagWidget->SetIconText(InTagDataList[TagIndex]);
		TagWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	for (int32 ChildIndex = InTagDataList.Num(); ChildIndex < HorizontalBox_TagList->GetChildrenCount(); ++ChildIndex)
	{
		if (UWidget* ChildWidget = HorizontalBox_TagList->GetChildAt(ChildIndex))
		{
			ChildWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UPBBallDetailTagListRowWidget::ClearTagList()
{
	if (Text_Label)
	{
		Text_Label->SetText(FText::GetEmpty());
	}

	if (!HorizontalBox_TagList)
	{
		return;
	}

	for (int32 ChildIndex = 0; ChildIndex < HorizontalBox_TagList->GetChildrenCount(); ++ChildIndex)
	{
		if (UWidget* ChildWidget = HorizontalBox_TagList->GetChildAt(ChildIndex))
		{
			ChildWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

UPBBallDetailIconTextTagWidget* UPBBallDetailTagListRowWidget::GetOrCreateTagWidget(const int32 TagIndex)
{
	if (!HorizontalBox_TagList || TagIndex < 0)
	{
		return nullptr;
	}

	if (TagIndex < HorizontalBox_TagList->GetChildrenCount())
	{
		return Cast<UPBBallDetailIconTextTagWidget>(HorizontalBox_TagList->GetChildAt(TagIndex));
	}

	if (!TagWidgetClass)
	{
		return nullptr;
	}

	UPBBallDetailIconTextTagWidget* NewTagWidget =
		CreateWidget<UPBBallDetailIconTextTagWidget>(this, TagWidgetClass);
	if (!NewTagWidget)
	{
		return nullptr;
	}

	HorizontalBox_TagList->AddChild(NewTagWidget);
	return NewTagWidget;
}
