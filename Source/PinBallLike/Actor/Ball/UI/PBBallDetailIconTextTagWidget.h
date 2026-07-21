#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
#include "PBBallDetailIconTextTagWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBBallDetailIconTextTagWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void SetIconText(const FPBBallDetailIconTextViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void SetIconTextData(UTexture2D* InIconTexture, const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void ClearIconText();

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Name;
};
