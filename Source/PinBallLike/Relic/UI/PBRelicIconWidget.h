#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBRelicIconWidget.generated.h"

class UImage;
class UPBRelicTooltipWidget;

UCLASS()
class PINBALLLIKE_API UPBRelicIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetRelicId(FName InRelicId);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RelicIcon;

	UPROPERTY(EditAnywhere, Category = "Relic|UI")
	TSubclassOf<UPBRelicTooltipWidget> RelicTooltipWidgetClass;
	
	UPROPERTY()
	FName RelicId = NAME_None;
	
protected:
	virtual void NativeOnMouseEnter(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseLeave(
		const FPointerEvent& InMouseEvent) override;
};