#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyDetailPanelWidget.generated.h"

class UPBSynergyDetailPanelViewModel;
class UPBSynergyBallIconWidget;
class UPBSynergyTierRowWidget;
class UPanelWidget;

UCLASS()
class PINBALLLIKE_API UPBSynergyDetailPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Synergy|Detail")
	void InitializeSynergyDetailPanel(const FPBSynergyViewData& InViewData);

	UFUNCTION(BlueprintPure, Category = "Synergy|Detail")
	const FPBSynergyViewData& GetSynergyViewData() const { return ViewData; }

	UFUNCTION(BlueprintPure, Category = "Synergy|Detail")
	UPBSynergyDetailPanelViewModel* GetViewModel() const { return ViewModel; }

protected:
	virtual void NativeOnInitialized() override;

private:
	void EnsureViewModel();
	bool ApplyViewModelToWidget();
	void RebuildTierList();
	void RebuildBallIconList();

	UPROPERTY(Transient)
	FPBSynergyViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBSynergyDetailPanelViewModel> ViewModel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> VerticalBox_TierList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy|Detail", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBSynergyTierRowWidget> TierRowWidgetClass;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> HorizontalBox_Ballicons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy|Detail", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPBSynergyBallIconWidget> BallIconWidgetClass;
};
