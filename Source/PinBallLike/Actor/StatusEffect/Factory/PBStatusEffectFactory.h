#pragma once

#include "CoreMinimal.h"

class UPBBaseStatusEffect;
class UPBStatusEffectComponent;
struct FPBStatusEffectModifierRow;
struct FPBStatusEffectRow;
struct FPBStatusEffectTriggerRow;

class PINBALLLIKE_API PBStatusEffectFactory
{
public:
	static void RegisterStatusEffectClass(
		FName StatusEffectId,
		TSubclassOf<UPBBaseStatusEffect> StatusEffectClass);

	static UPBBaseStatusEffect* CreateStatusEffect(
		UObject* Outer,
		UPBStatusEffectComponent* OwnerComponent,
		FName StatusEffectId,
		const FPBStatusEffectRow& StatusEffectRow,
		const TArray<FPBStatusEffectModifierRow>& ModifierRows,
		const TArray<FPBStatusEffectTriggerRow>& TriggerRows);

	static TSubclassOf<UPBBaseStatusEffect> ResolveStatusEffectClass(FName StatusEffectId);

private:
	static TMap<FName, TSubclassOf<UPBBaseStatusEffect>>& GetStatusEffectClassMap();
};
