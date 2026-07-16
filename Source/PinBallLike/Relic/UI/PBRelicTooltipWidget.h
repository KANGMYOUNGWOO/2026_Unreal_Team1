#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBRelicTooltipWidget.generated.h"

class UTextBlock;

UCLASS()
class PINBALLLIKE_API UPBRelicTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetRelicData(
		const FText& InDisplayName,
		const FText& InDescription);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RelicNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RelicDescriptionText;
};