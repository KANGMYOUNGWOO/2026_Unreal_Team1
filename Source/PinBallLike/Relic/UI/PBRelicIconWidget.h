#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Relic/PBRelicViewData.h"
#include "PBRelicIconWidget.generated.h"

class UPBRelicTooltipWidget;
class UPBRelicViewModel;

UCLASS()
class PINBALLLIKE_API UPBRelicIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void SetRelicId(FName InRelicId);
	void SetRelicViewData(const FPBRelicViewData& InViewData);

private:
	FPBRelicViewData BuildRelicViewData(FName InRelicId) const;
	void RebuildRelicTooltip();
	void EnsureRelicViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(EditAnywhere, Category = "Relic|UI")
	TSubclassOf<UPBRelicTooltipWidget> RelicTooltipWidgetClass;
	
	UPROPERTY()
	FPBRelicViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBRelicViewModel> RelicViewModel;
	
protected:
	virtual void NativeOnMouseEnter(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(
		const FPointerEvent& InMouseEvent) override;
};
