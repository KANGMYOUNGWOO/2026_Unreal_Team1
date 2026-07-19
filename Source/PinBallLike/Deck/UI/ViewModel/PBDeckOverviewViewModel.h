#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Synergy/PBSynergyState.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBDeckOverviewViewModel.generated.h"

class UPBTableDataSubsystem;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBDeckOverviewViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(UObject* InWorldContextObject);
	void SetSynergyStates(const TArray<FPBSynergyState>& InSynergyStates);
	void ClearSynergyViewData();

	const TArray<FPBSynergyViewData>& GetActiveSynergyViewData() const { return ActiveSynergyViewData; }

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "DeckOverview|ViewModel")
	TArray<FPBSynergyViewData> ActiveSynergyViewData;

private:
	const UPBTableDataSubsystem* GetTableDataSubsystem() const;
	FText BuildSynergyCountListText(FName SynergyId, int32 CurrentCount) const;

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;
};
