#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectViewData.h"
#include "PBStatusEffectItemWidget.generated.h"

class UPBBaseStatusEffect;
class UPBStatusEffectItemViewModel;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBStatusEffectItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "StatusEffect|Item")
	void InitializeStatusEffectItem(const FPBStatusEffectViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "StatusEffect|Item")
	void InitializeStatusEffectItemFromEffect(UPBBaseStatusEffect* InStatusEffect, UTexture2D* InIcon);

	UFUNCTION(BlueprintCallable, Category = "StatusEffect|Item")
	void RefreshStackCount(int32 InStackCount);

	UFUNCTION(BlueprintCallable, Category = "StatusEffect|Item")
	void ClearStatusEffectItem();

protected:
	virtual void NativeOnInitialized() override;

private:
	void EnsureItemViewModel();
	bool ApplyViewModelToWidget();

	UPROPERTY(Transient)
	FPBStatusEffectViewData ViewData;

	UPROPERTY(Transient)
	TObjectPtr<UPBStatusEffectItemViewModel> ItemViewModel;
};
