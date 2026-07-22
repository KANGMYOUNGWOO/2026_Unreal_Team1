#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Relic/PBRelicViewData.h"
#include "PBRelicViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBRelicViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Relic|ViewModel")
	void SetRelicViewData(const FPBRelicViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Relic|ViewModel")
	void ClearRelic();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	bool bHasRelic = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	EPBRelicChoiceRewardType RewardType = EPBRelicChoiceRewardType::Relic;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	FName RelicId = NAME_None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	FText Description;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	EPBRelicRarity Rarity = EPBRelicRarity::Common;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Relic|ViewModel")
	int32 GoldAmount = 0;
};
