#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyTierRowViewModel.generated.h"

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBSynergyTierRowViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Synergy|TierRowViewModel")
	void SetTierViewData(const FPBSynergyTierViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Synergy|TierRowViewModel")
	void ClearTier();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|TierRowViewModel")
	int32 RequiredCount = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|TierRowViewModel")
	FText RequiredCountText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|TierRowViewModel")
	FText EffectText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|TierRowViewModel")
	bool bIsActive = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|TierRowViewModel")
	bool bIsCurrentTier = false;
};
