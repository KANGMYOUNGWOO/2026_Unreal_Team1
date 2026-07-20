#include "PBEffectHandler.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Ball/Component/PBBallEffectRuntimeComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

UPBEffectHandler::UPBEffectHandler()
{
	struct FPBEffectApplyEntry
	{
		FName EffectType = NAME_None;
		FApplyEffectFunction Function = nullptr;
	};

	const FPBEffectApplyEntry Entries[] = {
		{ PBEffectTypes::EffectType::StatusEffect, &UPBEffectHandler::ApplyStatusEffect },
		{ PBEffectTypes::EffectType::StatBuff, &UPBEffectHandler::ApplyStatBuff },
		{ PBEffectTypes::EffectType::ResourceBuff, &UPBEffectHandler::ApplyResourceBuff },
		{ PBEffectTypes::EffectType::DamageTakenModifier, &UPBEffectHandler::ApplyDamageTakenModifier },
		{ PBEffectTypes::EffectType::DamageIgnore, &UPBEffectHandler::ApplyDamageIgnore },
		{ PBEffectTypes::EffectType::FirstAttack, &UPBEffectHandler::ApplyFirstAttack },
		{ PBEffectTypes::EffectType::ComboStatBuff, &UPBEffectHandler::ApplyComboStatBuff },
		{ PBEffectTypes::EffectType::SkillDamageModifier, &UPBEffectHandler::ApplySkillDamageModifier },
		{ PBEffectTypes::EffectType::OnHitStatStack, &UPBEffectHandler::ApplyOnHitStatStack },
		{ PBEffectTypes::EffectType::OnHitDamageIgnoreChance, &UPBEffectHandler::ApplyOnHitDamageIgnoreChance },
		{ PBEffectTypes::EffectType::ComboExtraDamage, &UPBEffectHandler::ApplyComboExtraDamage },
		{ PBEffectTypes::EffectType::MultiBall, &UPBEffectHandler::ApplyMultiBall },
		{ PBEffectTypes::EffectType::ReviveOnZero, &UPBEffectHandler::ApplyReviveOnZero },
		{ PBEffectTypes::EffectType::Pierce, &UPBEffectHandler::ApplyPierce },
		{ PBEffectTypes::EffectType::ResourceCostStatBuff, &UPBEffectHandler::ApplyResourceCostStatBuff },
		{ PBEffectTypes::EffectType::ShopPriceDiscount, &UPBEffectHandler::ApplyShopPriceDiscount },
		{ PBEffectTypes::EffectType::ShopRerollDiscount, &UPBEffectHandler::ApplyShopRerollDiscount },
		{ PBEffectTypes::EffectType::PostDamageHeal, &UPBEffectHandler::ApplyPostDamageHeal },
		{ PBEffectTypes::EffectType::SwitchCountBonus, &UPBEffectHandler::ApplySwitchCountBonus },
		{ PBEffectTypes::EffectType::SwitchNextHitDamage, &UPBEffectHandler::ApplySwitchNextHitDamage },
		{ PBEffectTypes::EffectType::SwitchElasticityBuff, &UPBEffectHandler::ApplySwitchElasticityBuff },
		{ PBEffectTypes::EffectType::ReviveStatBuff, &UPBEffectHandler::ApplyReviveStatBuff },
		{ PBEffectTypes::EffectType::FirstHitTimedStatBuff, &UPBEffectHandler::ApplyFirstHitTimedStatBuff }
	};

	for (const FPBEffectApplyEntry& Entry : Entries)
	{
		if (Entry.EffectType.IsNone() || !Entry.Function)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Effect] Invalid handler registration entry."));
			continue;
		}
		if (ApplyFunctionMap.Contains(Entry.EffectType))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Effect] Duplicate handler registration. EffectType=%s"),
				*Entry.EffectType.ToString());
			continue;
		}

		ApplyFunctionMap.Add(Entry.EffectType, Entry.Function);
	}
}

bool UPBEffectHandler::ExecuteEffect(
	const FName EffectId,
	const FPBEffectTableRow& EffectRow,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FApplyEffectFunction* ApplyFunction = ApplyFunctionMap.Find(EffectRow.EffectType);
	if (!ApplyFunction)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] Handler cannot apply EffectType. EffectId=%s EffectType=%s"),
			*EffectId.ToString(),
			*EffectRow.EffectType.ToString());
		return false;
	}

	return (this->*(*ApplyFunction))(EffectId, EffectRow, ParamRows, Context);
}

bool UPBEffectHandler::IsEffectTypeSupported(const FName EffectType) const
{
	return ApplyFunctionMap.Contains(EffectType);
}

FString UPBEffectHandler::FindParamValue(const TArray<FPBEffectParamRow>& ParamRows, const FName ParamKey)
{
	if (ParamKey.IsNone())
	{
		return FString();
	}

	for (const FPBEffectParamRow& ParamRow : ParamRows)
	{
		if (ParamRow.ParamKey == ParamKey)
		{
			return ParamRow.ParamValue;
		}
	}

	return FString();
}

FName UPBEffectHandler::FindParamName(const TArray<FPBEffectParamRow>& ParamRows, const FName ParamKey)
{
	const FString ParamValue = FindParamValue(ParamRows, ParamKey);
	return ParamValue.IsEmpty() ? NAME_None : FName(*ParamValue);
}

FName UPBEffectHandler::NormalizeStatName(const FName StatName)
{
	if (StatName == FName(TEXT("Elasticity")))
	{
		return PBStatNames::Bounciness;
	}

	if (StatName == FName(TEXT("ManaRecovery")))
	{
		return PBStatNames::ManaRegen;
	}

	return StatName;
}

FName UPBEffectHandler::NormalizeResourceName(const FName ResourceName)
{
	if (ResourceName == FName(TEXT("Health")))
	{
		return PBResourceNames::Health;
	}

	if (ResourceName == FName(TEXT("Mana")))
	{
		return PBResourceNames::Mana;
	}

	return ResourceName;
}

int32 UPBEffectHandler::CalculateModifiedStatValue(
	const int32 CurrentValue,
	const FName ModifyType,
	const float Value)
{
	if (ModifyType == PBEffectTypes::ModifyType::Set)
	{
		return FMath::RoundToInt(Value);
	}

	if (ModifyType == PBEffectTypes::ModifyType::PercentAdd)
	{
		return FMath::RoundToInt(static_cast<float>(CurrentValue) * (1.0f + Value * 0.01f));
	}

	return CurrentValue + FMath::RoundToInt(Value);
}

float UPBEffectHandler::CalculateModifiedResourceValue(
	const float CurrentValue,
	const FName ModifyType,
	const float Value)
{
	if (ModifyType == PBEffectTypes::ModifyType::Set)
	{
		return Value;
	}

	if (ModifyType == PBEffectTypes::ModifyType::PercentAdd)
	{
		return CurrentValue * (1.0f + Value * 0.01f);
	}

	return CurrentValue + Value;
}

bool UPBEffectHandler::ApplyResourceBuffToComponent(
	UPBBaseResourceComponent* ResourceComponent,
	const FName ResourceName,
	const FName ApplyTo,
	const FName ModifyType,
	const float Value)
{
	if (!ResourceComponent)
	{
		return false;
	}

	bool bApplied = false;
	if (ApplyTo == PBEffectTypes::ResourceApplyTo::Max
		|| ApplyTo == PBEffectTypes::ResourceApplyTo::CurrentAndMax)
	{
		const float CurrentMax = ResourceComponent->GetResourceMax(ResourceName);
		const float NewMax = CalculateModifiedResourceValue(CurrentMax, ModifyType, Value);
		ResourceComponent->SetResourceMax(ResourceName, NewMax, false);
		bApplied = true;
	}

	if (ApplyTo == PBEffectTypes::ResourceApplyTo::Current
		|| ApplyTo == PBEffectTypes::ResourceApplyTo::CurrentAndMax)
	{
		const float CurrentValue = ResourceComponent->GetResourceCurrent(ResourceName);
		const float NewValue = CalculateModifiedResourceValue(CurrentValue, ModifyType, Value);
		ResourceComponent->SetResourceCurrent(ResourceName, NewValue);
		bApplied = true;
	}

	return bApplied;
}

TArray<UPBBallEffectRuntimeComponent*> UPBEffectHandler::ResolveRuntimeComponents(
	const FPBEffectContext& Context) const
{
	TArray<UPBBallEffectRuntimeComponent*> RuntimeComponents;
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBBallEffectRuntimeComponent* RuntimeComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBallEffectRuntimeComponent>()
			: nullptr;
		if (RuntimeComponent)
		{
			RuntimeComponents.Add(RuntimeComponent);
		}
	}

	return RuntimeComponents;
}
