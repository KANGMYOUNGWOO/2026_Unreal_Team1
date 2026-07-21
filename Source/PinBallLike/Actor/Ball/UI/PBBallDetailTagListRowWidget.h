#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
#include "PBBallDetailTagListRowWidget.generated.h"

class UPBBallDetailIconTextTagWidget;
class UPanelWidget;
class UTextBlock;

UCLASS()
class PINBALLLIKE_API UPBBallDetailTagListRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void SetTagList(const FText& InLabelText, const TArray<FPBBallDetailIconTextViewData>& InTagDataList);

	UFUNCTION(BlueprintCallable, Category = "Ball|Detail")
	void ClearTagList();

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Detail")
	TSubclassOf<UPBBallDetailIconTextTagWidget> TagWidgetClass;

private:
	UPBBallDetailIconTextTagWidget* GetOrCreateTagWidget(int32 TagIndex);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Label;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> HorizontalBox_TagList;
};
