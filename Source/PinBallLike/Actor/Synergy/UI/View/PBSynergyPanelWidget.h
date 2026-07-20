#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyPanelWidget.generated.h"

class UPBSynergyPanelViewModel;
class UPBSynergyDetailPanelWidget;

UCLASS()
class PINBALLLIKE_API UPBSynergyPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Synergy|Panel")
	void InitializeSynergyPanel(const FPBSynergyViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Synergy|Panel")
	void ClearSynergyPanel();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy|Detail")
	TSubclassOf<UPBSynergyDetailPanelWidget> SynergyDetailPanelWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy|Detail")
	FVector2D DetailPanelViewportOffset = FVector2D(16.0f, 12.0f);

private:
	void EnsurePanelViewModel();
	bool ApplyViewModelToWidget();
	void ShowDetailPanel();
	void HideDetailPanel();
	void UpdateDetailPanelPosition();

	UPROPERTY(Transient)
	FPBSynergyViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBSynergyPanelViewModel> PanelViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UPBSynergyDetailPanelWidget> DetailPanelWidget;
};
