#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Relic/PBRelicViewData.h"
#include "PBRelicTooltipWidget.generated.h"

class UPBRelicViewModel;

UCLASS()
class PINBALLLIKE_API UPBRelicTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void SetRelicData(const FText& InDisplayName, const FText& InDescription);
	void SetRelicViewData(const FPBRelicViewData& InViewData);

private:
	void EnsureRelicViewModel();
	bool ApplyViewModelToWidget();

private:
	UPROPERTY(Transient)
	TObjectPtr<UPBRelicViewModel> RelicViewModel;

	UPROPERTY()
	FPBRelicViewData ViewData;
};
