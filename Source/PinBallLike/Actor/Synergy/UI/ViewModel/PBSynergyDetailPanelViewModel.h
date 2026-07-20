#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyDetailPanelViewModel.generated.h"

class UTexture2D;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBSynergyDetailPanelViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Synergy|DetailViewModel")
	void SetSynergyViewData(const FPBSynergyViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Synergy|DetailViewModel")
	void ClearSynergy();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	bool bHasSynergy = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	FName SynergyId = NAME_None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	FText SynergyNameText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	FText DescriptionText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	FText CurrentCountText = FText::AsNumber(0);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	FText CountListText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	TArray<FPBSynergyTierViewData> TierViewDataList;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|DetailViewModel")
	TArray<FPBSynergyBallIconViewData> BallIconViewDataList;
};
