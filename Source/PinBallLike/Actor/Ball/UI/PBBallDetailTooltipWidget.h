#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Ball/PBBallDetailTooltipViewData.h"
#include "PBBallDetailTooltipWidget.generated.h"

class UPBBallDetailTooltipViewModel;
class UPBBallDetailInfoRowWidget;
class UPBBallDetailTagListRowWidget;

UCLASS()
class PINBALLLIKE_API UPBBallDetailTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|DetailTooltip")
	void SetTooltipViewData(const FPBBallDetailTooltipViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Ball|DetailTooltip")
	void ClearTooltip();

	UFUNCTION(BlueprintPure, Category = "Ball|DetailTooltip")
	UPBBallDetailTooltipViewModel* GetTooltipViewModel() const { return TooltipViewModel; }

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|DetailTooltip")
	void BP_OnTooltipViewDataChanged(const FPBBallDetailTooltipViewData& InViewData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ball|DetailTooltip")
	void BP_OnTooltipCleared();

private:
	void EnsureViewModel();
	bool ApplyViewModelToWidget();
	void ApplyViewDataToChildWidgets();

	UPROPERTY(Transient)
	FPBBallDetailTooltipViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDetailTooltipViewModel> TooltipViewModel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailInfoRowWidget> InfoRow_HP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailInfoRowWidget> InfoRow_MP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailInfoRowWidget> InfoRow_Attack;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailInfoRowWidget> InfoRow_ManaRegen;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailInfoRowWidget> Tag_Type;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailInfoRowWidget> Tag_Job;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBBallDetailTagListRowWidget> TagListRow_Race;
};
