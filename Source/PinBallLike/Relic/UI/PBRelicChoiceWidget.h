#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Relic/PBRelicViewData.h"
#include "PBRelicChoiceWidget.generated.h"

class UPBRelicChoicePanel;

DECLARE_DELEGATE_OneParam(
	FPBOnRelicSelected,
	FPBRelicViewData);

UCLASS()
class PINBALLLIKE_API UPBRelicChoiceWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetRelicChoices(
		const TArray<FName>& InRelicIds,
		int32 InFallbackGoldAmount);

	FPBOnRelicSelected OnRelicSelected;

private:
	void SelectRelic(int32 SlotIndex);
	FPBRelicViewData BuildRelicViewData(FName RelicId) const;
	FPBRelicViewData BuildGoldViewData(int32 GoldAmount) const;

	UFUNCTION()
	void HandleRelicPanelClicked(UPBRelicChoicePanel* ClickedPanel);

private:

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBRelicChoicePanel> Panel_Relic0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBRelicChoicePanel> Panel_Relic1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBRelicChoicePanel> Panel_Relic2;

	UPROPERTY()
	TArray<FName> RelicIds;

	UPROPERTY()
	TArray<FPBRelicViewData> RelicViewDataList;

	UPROPERTY()
	int32 FallbackGoldAmount = 0;
};
