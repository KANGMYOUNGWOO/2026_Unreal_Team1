#include "PinBallLike/Actor/StatusEffect/Factory/PBStatusEffectFactory.h"

#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Actor/StatusEffect/Effects/PBBounceStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Effects/PBStrengthStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Effects/PBBurnStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Effects/PBShieldStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Effects/PBZeroGravityStatusEffect.h"
#include "PinBallLike/Table/StatusEffect/PBStatusEffectAssetIds.h"
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
	const FName StatusEffectId,
	const FPBStatusEffectRow& StatusEffectRow,
	const TArray<FPBStatusEffectModifierRow>& ModifierRows,
	const TArray<FPBStatusEffectTriggerRow>& TriggerRows)
{
	const TSubclassOf<UPBBaseStatusEffect> StatusEffectClass =
		ResolveStatusEffectClass(StatusEffectId);
	UClass* StatusEffectClassPtr = StatusEffectClass.Get();
	const FString StatusEffectIdName = StatusEffectId.ToString();
	const FString OuterName = Outer ? Outer->GetName() : TEXT("None");
	const FString OwnerComponentName = OwnerComponent ? OwnerComponent->GetName() : TEXT("None");
	const FString StatusEffectClassName = StatusEffectClassPtr ? StatusEffectClassPtr->GetName() : TEXT("None");
	if (!StatusEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatusEffectFactory] Create failed. StatusEffect class not registered. StatusEffectId=%s Outer=%s OwnerComponent=%s"),
			*StatusEffectIdName,
			*OuterName,
			*OwnerComponentName);
		return nullptr;
	}

	UPBBaseStatusEffect* StatusEffect = NewObject<UPBBaseStatusEffect>(Outer, StatusEffectClassPtr);
	if (!IsValid(StatusEffect))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatusEffectFactory] Create failed. NewObject returned invalid. StatusEffectId=%s Class=%s Outer=%s OwnerComponent=%s"),
			*StatusEffectIdName,
			*StatusEffectClassName,
			*OuterName,
			*OwnerComponentName);
		return nullptr;
	}

	StatusEffect->InitializeStatusEffect(OwnerComponent, StatusEffectId, StatusEffectRow, ModifierRows, TriggerRows);
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
		StatusEffectClassMap.Add(PBStatusEffectAssetIds::StatusEffect::BounceUp, UPBBounceStatusEffect::StaticClass());
		StatusEffectClassMap.Add(PBStatusEffectAssetIds::StatusEffect::Strength, UPBStrengthStatusEffect::StaticClass());
		StatusEffectClassMap.Add(PBStatusEffectAssetIds::StatusEffect::Burn, UPBBurnStatusEffect::StaticClass());
		StatusEffectClassMap.Add(PBStatusEffectAssetIds::StatusEffect::Shield, UPBShieldStatusEffect::StaticClass());
		StatusEffectClassMap.Add(PBStatusEffectAssetIds::StatusEffect::InvincibleSkill, UPBBaseStatusEffect::StaticClass());
		StatusEffectClassMap.Add(PBStatusEffectAssetIds::StatusEffect::ZeroGravity, UPBZeroGravityStatusEffect::StaticClass());
	}

	return StatusEffectClassMap;
}
