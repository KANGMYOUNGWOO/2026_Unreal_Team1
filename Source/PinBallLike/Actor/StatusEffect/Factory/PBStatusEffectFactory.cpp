#include "PinBallLike/Actor/StatusEffect/Factory/PBStatusEffectFactory.h"

#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Effects/PBBurnStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Effects/PBShieldStatusEffect.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectModifierRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectRow.h"
#include "PinBallLike/Table/StatusEffect/Struct/PBStatusEffectTriggerRow.h"

void PBStatusEffectFactory::RegisterStatusEffectClass(
	const FName StatusEffectId,
	const TSubclassOf<UPBBaseStatusEffect> StatusEffectClass)
{
	if (StatusEffectId.IsNone() || !StatusEffectClass)
	{
		return;
	}

	GetStatusEffectClassMap().Add(StatusEffectId, StatusEffectClass);
}

UPBBaseStatusEffect* PBStatusEffectFactory::CreateStatusEffect(
	UObject* Outer,
	UPBStatusEffectComponent* OwnerComponent,
	const FPBStatusEffectRow& StatusEffectRow,
	const TArray<FPBStatusEffectModifierRow>& ModifierRows,
	const TArray<FPBStatusEffectTriggerRow>& TriggerRows)
{
	const TSubclassOf<UPBBaseStatusEffect> StatusEffectClass =
		ResolveStatusEffectClass(StatusEffectRow.StatusEffectId);
	if (!StatusEffectClass)
	{
		return nullptr;
	}

	UPBBaseStatusEffect* StatusEffect = NewObject<UPBBaseStatusEffect>(Outer, StatusEffectClass);
	if (!IsValid(StatusEffect))
	{
		return nullptr;
	}

	StatusEffect->InitializeStatusEffect(OwnerComponent, StatusEffectRow, ModifierRows, TriggerRows);
	return StatusEffect;
}

TSubclassOf<UPBBaseStatusEffect> PBStatusEffectFactory::ResolveStatusEffectClass(const FName StatusEffectId)
{
	const TSubclassOf<UPBBaseStatusEffect>* StatusEffectClass = GetStatusEffectClassMap().Find(StatusEffectId);
	return StatusEffectClass ? *StatusEffectClass : nullptr;
}

TMap<FName, TSubclassOf<UPBBaseStatusEffect>>& PBStatusEffectFactory::GetStatusEffectClassMap()
{
	static TMap<FName, TSubclassOf<UPBBaseStatusEffect>> StatusEffectClassMap;
	if (StatusEffectClassMap.Num() == 0)
	{
		StatusEffectClassMap.Add(TEXT("Burn"), UPBBurnStatusEffect::StaticClass());
		StatusEffectClassMap.Add(TEXT("Shield"), UPBShieldStatusEffect::StaticClass());
	}

	return StatusEffectClassMap;
}
