#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBRelicChoiceWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DELEGATE_OneParam(
	FPBOnRelicSelected,
	FName);

UCLASS()
class PINBALLLIKE_API UPBRelicChoiceWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetRelicChoices(
		const TArray<FName>& InRelicIds);

	FPBOnRelicSelected OnRelicSelected;

private:
	void SelectRelic(int32 SlotIndex);

	UFUNCTION()
	void HandleRelic0Clicked();

	UFUNCTION()
	void HandleRelic1Clicked();

	UFUNCTION()
	void HandleRelic2Clicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Relic0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Relic1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Relic2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Relic0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Relic1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Relic2;

	UPROPERTY()
	TArray<FName> RelicIds;
};