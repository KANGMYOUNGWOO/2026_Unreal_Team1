#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PBEffectHandler.generated.h"

struct FPBEffectContext;
struct FPBEffectParamRow;
struct FPBEffectTableRow;

class UPBBallEffectRuntimeComponent;
class UPBBaseResourceComponent;

UCLASS()
class PINBALLLIKE_API UPBEffectHandler : public UObject
{
	GENERATED_BODY()

public:
	using FApplyEffectFunction = bool (UPBEffectHandler::*)(
		const FName EffectId,
		const FPBEffectTableRow& EffectRow,
		const TArray<FPBEffectParamRow>& ParamRows,
		const FPBEffectContext& Context);

	UPBEffectHandler();

	bool ExecuteEffect(
		const FName EffectId,
		const FPBEffectTableRow& EffectRow,
		const TArray<FPBEffectParamRow>& ParamRows,
		const FPBEffectContext& Context);

	bool IsEffectTypeSupported(FName EffectType) const;

private:
	bool ApplyStatusEffect(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyStatBuff(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyResourceBuff(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyDamageTakenModifier(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyDamageIgnore(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyFirstAttack(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyComboStatBuff(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplySkillDamageModifier(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyOnHitStatStack(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyOnHitDamageIgnoreChance(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyComboExtraDamage(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyMultiBall(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyReviveOnZero(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyPierce(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyResourceCostStatBuff(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyShopPriceDiscount(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyShopRerollDiscount(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyPostDamageHeal(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplySwitchCountBonus(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplySwitchNextHitDamage(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplySwitchElasticityBuff(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyReviveStatBuff(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);
	bool ApplyFirstHitTimedStatBuff(FName EffectId, const FPBEffectTableRow& EffectRow, const TArray<FPBEffectParamRow>& ParamRows, const FPBEffectContext& Context);

	static FString FindParamValue(const TArray<FPBEffectParamRow>& ParamRows, FName ParamKey);
	static FName FindParamName(const TArray<FPBEffectParamRow>& ParamRows, FName ParamKey);
	static FName NormalizeStatName(FName StatName);
	static FName NormalizeResourceName(FName ResourceName);
	static int32 CalculateModifiedStatValue(int32 CurrentValue, FName ModifyType, float Value);
	static float CalculateModifiedResourceValue(float CurrentValue, FName ModifyType, float Value);
	static bool ApplyResourceBuffToComponent(
		UPBBaseResourceComponent* ResourceComponent,
		FName ResourceName,
		FName ApplyTo,
		FName ModifyType,
		float Value);

	TArray<UPBBallEffectRuntimeComponent*> ResolveRuntimeComponents(const FPBEffectContext& Context) const;

	TMap<FName, FApplyEffectFunction> ApplyFunctionMap;
};
