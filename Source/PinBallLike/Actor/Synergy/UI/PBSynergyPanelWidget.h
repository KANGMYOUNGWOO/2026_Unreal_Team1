#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyPanelWidget.generated.h"

class UPBSynergyPanelViewModel;

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

private:
	void EnsurePanelViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	FPBSynergyViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBSynergyPanelViewModel> PanelViewModel;
};
