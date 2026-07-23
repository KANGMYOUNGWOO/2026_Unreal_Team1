#include "PBRelicChoiceWidget.h"
#include "PBRelicChoicePanel.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"

void UPBRelicChoiceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Panel_Relic0)
	{
		Panel_Relic0->OnClicked.AddUniqueDynamic(
			this,
			&UPBRelicChoiceWidget::HandleRelicPanelClicked);
	}

	if (Panel_Relic1)
	{
		Panel_Relic1->OnClicked.AddUniqueDynamic(
			this,
			&UPBRelicChoiceWidget::HandleRelicPanelClicked);
	}

	if (Panel_Relic2)
	{
		Panel_Relic2->OnClicked.AddUniqueDynamic(
			this,
			&UPBRelicChoiceWidget::HandleRelicPanelClicked);
	}
}

void UPBRelicChoiceWidget::SetRelicChoices(const TArray<FName>& InRelicIds, const int32 InFallbackGoldAmount)
{
	RelicIds = InRelicIds;
	FallbackGoldAmount = FMath::Max(InFallbackGoldAmount, 0);
	RelicViewDataList.Reset();

	for (const FName RelicId : RelicIds)
	{
		RelicViewDataList.Add(BuildRelicViewData(RelicId));
	}

	while (RelicViewDataList.Num() < 3)
	{
		RelicViewDataList.Add(BuildGoldViewData(FallbackGoldAmount));
	}

	if (Panel_Relic0)
	{
		Panel_Relic0->SetRelicViewData(RelicViewDataList.IsValidIndex(0) ? RelicViewDataList[0] : FPBRelicViewData());
	}

	if (Panel_Relic1)
	{
		Panel_Relic1->SetRelicViewData(RelicViewDataList.IsValidIndex(1) ? RelicViewDataList[1] : FPBRelicViewData());
	}

	if (Panel_Relic2)
	{
		Panel_Relic2->SetRelicViewData(RelicViewDataList.IsValidIndex(2) ? RelicViewDataList[2] : FPBRelicViewData());
	}
}

void UPBRelicChoiceWidget::SelectRelic(const int32 SlotIndex)
{
	if (!RelicViewDataList.IsValidIndex(SlotIndex))
	{
		return;
	}

	OnRelicSelected.ExecuteIfBound(RelicViewDataList[SlotIndex]);
}

FPBRelicViewData UPBRelicChoiceWidget::BuildRelicViewData(const FName RelicId) const
{
	FPBRelicViewData ViewData;
	ViewData.RelicId = RelicId;

	UGameInstance* GameInstance = GetGameInstance();
	UPBTableDataSubsystem* TableSubsystem = GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	if (!TableSubsystem)
	{
		return ViewData;
	}

	FPBRelicTableRow RelicRow;
	if (!TableSubsystem->FindRelicRow(RelicId, RelicRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RelicChoice] Relic row not found. RelicId=%s"), *RelicId.ToString());
		return ViewData;
	}

	if (!RelicRow.RelicDataAsset.IsNull())
	{
		if (UPBRelicDataAsset* DataAsset = RelicRow.RelicDataAsset.LoadSynchronous())
		{
			ViewData.IconTexture = DataAsset->RelicIcon.LoadSynchronous();
		}
	}
	
	ViewData.DisplayName = RelicRow.DisplayName.IsEmpty() ? FText::FromName(RelicId) : RelicRow.DisplayName;
	ViewData.Description = RelicRow.Description;
	ViewData.Rarity = RelicRow.Rarity;
	return ViewData;
}

FPBRelicViewData UPBRelicChoiceWidget::BuildGoldViewData(const int32 GoldAmount) const
{
	FPBRelicViewData ViewData;
	ViewData.RewardType = EPBRelicChoiceRewardType::Gold;
	ViewData.DisplayName = NSLOCTEXT("RelicChoice", "GoldRewardName", "골드");
	ViewData.Description = FText::Format(NSLOCTEXT("RelicChoice", "GoldRewardDescription", "{0} Gold 획득"), FText::AsNumber(GoldAmount));
	ViewData.GoldAmount = GoldAmount;
	return ViewData;
}

void UPBRelicChoiceWidget::HandleRelicPanelClicked(UPBRelicChoicePanel* ClickedPanel)
{
	if (ClickedPanel == Panel_Relic0)
	{
		SelectRelic(0);
		return;
	}

	if (ClickedPanel == Panel_Relic1)
	{
		SelectRelic(1);
		return;
	}

	if (ClickedPanel == Panel_Relic2)
	{
		SelectRelic(2);
	}
}
