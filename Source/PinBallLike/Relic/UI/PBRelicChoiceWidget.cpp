#include "PBRelicChoiceWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPBRelicChoiceWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Relic0)
    {
        Button_Relic0->OnClicked.AddUniqueDynamic(
            this,
            &UPBRelicChoiceWidget::HandleRelic0Clicked);
    }

    if (Button_Relic1)
    {
        Button_Relic1->OnClicked.AddUniqueDynamic(
            this,
            &UPBRelicChoiceWidget::HandleRelic1Clicked);
    }

    if (Button_Relic2)
    {
        Button_Relic2->OnClicked.AddUniqueDynamic(
            this,
            &UPBRelicChoiceWidget::HandleRelic2Clicked);
    }
}

void UPBRelicChoiceWidget::SetRelicChoices(
    const TArray<FName>& InRelicIds)
{
    RelicIds = InRelicIds;

    if (Text_Relic0)
    {
        Text_Relic0->SetText(
            RelicIds.IsValidIndex(0)
                ? FText::FromName(RelicIds[0])
                : FText::GetEmpty());
    }

    if (Text_Relic1)
    {
        Text_Relic1->SetText(
            RelicIds.IsValidIndex(1)
                ? FText::FromName(RelicIds[1])
                : FText::GetEmpty());
    }

    if (Text_Relic2)
    {
        Text_Relic2->SetText(
            RelicIds.IsValidIndex(2)
                ? FText::FromName(RelicIds[2])
                : FText::GetEmpty());
    }
}

void UPBRelicChoiceWidget::SelectRelic(
    const int32 SlotIndex)
{
    if (!RelicIds.IsValidIndex(SlotIndex))
    {
        return;
    }

    OnRelicSelected.ExecuteIfBound(
        RelicIds[SlotIndex]);
}

void UPBRelicChoiceWidget::HandleRelic0Clicked()
{
    SelectRelic(0);
}

void UPBRelicChoiceWidget::HandleRelic1Clicked()
{
    SelectRelic(1);
}

void UPBRelicChoiceWidget::HandleRelic2Clicked()
{
    SelectRelic(2);
}