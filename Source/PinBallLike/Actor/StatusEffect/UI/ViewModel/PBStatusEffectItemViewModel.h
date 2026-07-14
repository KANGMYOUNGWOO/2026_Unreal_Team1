#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectViewData.h"
#include "PBStatusEffectItemViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBStatusEffectItemViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "StatusEffect|ItemViewModel")
	void SetStatusEffectViewData(const FPBStatusEffectViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "StatusEffect|ItemViewModel")
	void SetStackCount(int32 InStackCount);

	UFUNCTION(BlueprintCallable, Category = "StatusEffect|ItemViewModel")
	void ClearStatusEffect();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "StatusEffect|ItemViewModel")
	bool bHasStatusEffect = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "StatusEffect|ItemViewModel")
	FName StatusEffectId = NAME_None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "StatusEffect|ItemViewModel")
	int32 StackCount = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "StatusEffect|ItemViewModel")
	FText StackCountText = FText::AsNumber(0);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "StatusEffect|ItemViewModel")
	TObjectPtr<UTexture2D> IconTexture = nullptr;
};
