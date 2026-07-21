#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
#include "PBBallDetailInfoRowWidget.generated.h"

class UTextBlock;
class UPBBallDetailIconTextTagWidget;

UCLASS()
class PINBALLLIKE_API UPBBallDetailInfoRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void SetInfoRow(const FPBBallDetailInfoRowViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void SetInfoText(const FText& InLabelText, const FText& InValueText);

	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void SetIconTextRow(const FText& InLabelText, const FPBBallDetailIconTextViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void ClearInfoRow();

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Label;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailIconTextTagWidget> Custom_Value;
};
