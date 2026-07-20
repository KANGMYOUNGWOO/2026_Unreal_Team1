#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyTierRowWidget.generated.h"

class UPBSynergyTierRowViewModel;

UCLASS()
class PINBALLLIKE_API UPBSynergyTierRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Synergy|TierRow")
	void InitializeTierRow(const FPBSynergyTierViewData& InViewData);

	UFUNCTION(BlueprintPure, Category = "Synergy|TierRow")
	UPBSynergyTierRowViewModel* GetViewModel() const { return ViewModel; }

protected:
	virtual void NativeOnInitialized() override;

private:
	void EnsureViewModel();
	bool ApplyViewModelToWidget();
	void ApplyTierStyle();
	static FSlateColor GetActiveTierTextColor();
	static FSlateColor GetInactiveTierTextColor();

	UPROPERTY(Transient)
	FPBSynergyTierViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBSynergyTierRowViewModel> ViewModel;
};
