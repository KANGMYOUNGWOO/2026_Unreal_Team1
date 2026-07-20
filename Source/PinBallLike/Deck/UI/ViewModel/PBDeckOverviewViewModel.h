#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Synergy/PBSynergyState.h"
#include "PinBallLike/Struct/Synergy/PBSynergyViewData.h"
#include "PBDeckOverviewViewModel.generated.h"

class UPBTableDataSubsystem;
class UTexture2D;
struct FPBEffectParamRow;
struct FPBEffectTableRow;
struct FPBBallTableRow;
struct FPBSynergyTableRow;
struct FPBSynergyTierRow;

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
	FPBSynergyViewData BuildSynergyViewData(
		const FPBSynergyState& SynergyState,
		const FPBSynergyTableRow& SynergyRow) const;
	FText BuildSynergyCountListText(FName SynergyId, int32 CurrentCount) const;
	TArray<FPBSynergyTierViewData> BuildTierViewDataList(FName SynergyId, int32 CurrentCount) const;
	TArray<FPBSynergyBallIconViewData> BuildBallIconViewDataList(FName SynergyId) const;
	bool DoesBallMatchSynergy(const FPBBallTableRow& BallRow, FName SynergyId) const;
	FText BuildTierEffectText(const FPBSynergyTierRow& TierRow) const;
	FText BuildEffectSetText(FName EffectSetId) const;
	FText BuildEffectText(FName EffectId, const FPBEffectTableRow& EffectRow) const;
	UTexture2D* ResolveSynergyIcon(FName SynergyId) const;
	UTexture2D* ResolveBallIcon(FName BallId) const;
	FString FindEffectParamValue(const TArray<FPBEffectParamRow>& ParamRows, FName ParamKey) const;
	FName FindEffectParamName(const TArray<FPBEffectParamRow>& ParamRows, FName ParamKey) const;
	float FindEffectParamFloat(const TArray<FPBEffectParamRow>& ParamRows, FName ParamKey, float DefaultValue = 0.0f) const;
	FText MakeDisplayTextFromKey(FName TextKey) const;
	FText MakeModifyValueText(FName ModifyType, float Value) const;
	FText MakeEffectTargetText(FName TargetType, FName TargetFilter) const;

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;
};
