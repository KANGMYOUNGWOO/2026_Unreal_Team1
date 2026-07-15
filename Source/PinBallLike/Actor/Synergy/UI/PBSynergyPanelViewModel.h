#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBSynergyPanelViewModel.generated.h"

class UTexture2D;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBSynergyPanelViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Synergy|PanelViewModel")
	void SetSynergyViewData(const FPBSynergyViewData& InViewData);

	UFUNCTION(BlueprintCallable, Category = "Synergy|PanelViewModel")
	void ClearSynergy();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|PanelViewModel")
	bool bHasSynergy = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|PanelViewModel")
	FName SynergyId = NAME_None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|PanelViewModel")
	FText SynergyNameText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|PanelViewModel")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|PanelViewModel")
	FText CurrentCountText = FText::AsNumber(0);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|PanelViewModel")
	FText CountListText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Synergy|PanelViewModel")
	TObjectPtr<UTexture2D> IconTexture = nullptr;
};
