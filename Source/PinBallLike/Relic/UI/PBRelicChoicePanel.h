#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Relic/PBRelicViewData.h"
#include "PBRelicChoicePanel.generated.h"

class UPBRelicChoicePanel;
class UPBRelicViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBRelicChoicePanelClickedSignature,
	UPBRelicChoicePanel*,
	ClickedPanel);

UCLASS(BlueprintType, Blueprintable)
class PINBALLLIKE_API UPBRelicChoicePanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Relic|Choice")
	void SetRelicId(FName InRelicId);

	UFUNCTION(BlueprintCallable, Category = "Relic|Choice")
	void SetRelicViewData(const FPBRelicViewData& InViewData);

	UFUNCTION(BlueprintPure, Category = "Relic|Choice")
	FName GetRelicId() const;

	UPROPERTY(BlueprintAssignable, Category = "Relic|Choice")
	FPBRelicChoicePanelClickedSignature OnClicked;

protected:
	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

private:
	void EnsureRelicViewModel();
	bool ApplyViewModelToWidget();

private:
	UPROPERTY(Transient)
	TObjectPtr<UPBRelicViewModel> RelicViewModel;

	UPROPERTY()
	FPBRelicViewData ViewData;
};
