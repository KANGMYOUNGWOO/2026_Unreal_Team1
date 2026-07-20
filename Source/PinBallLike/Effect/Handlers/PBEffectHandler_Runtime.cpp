#include "PBEffectHandler.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Ball/Component/PBBallEffectRuntimeComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

bool UPBEffectHandler::ApplyFirstAttack(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const float AttackPercent = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::AttackPercent));
	const int32 ExtraDamage = FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::ExtraDamage));

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddFirstAttackRule(AttackPercent, ExtraDamage);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyComboStatBuff(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FName StatName = NormalizeStatName(FindParamName(ParamRows, PBEffectTypes::ParamKey::StatName));
	const FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	const int32 RequiredCombo = FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::RequiredCombo));

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddComboStatBuffRule(StatName, ModifyType, Value, RequiredCombo);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplySkillDamageModifier(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddSkillDamagePercent(Value);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyOnHitStatStack(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FName StatName = NormalizeStatName(FindParamName(ParamRows, PBEffectTypes::ParamKey::StatName));
	const FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddOnHitStatStackRule(StatName, ModifyType, Value);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyOnHitDamageIgnoreChance(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FName ResourceName = NormalizeResourceName(FindParamName(ParamRows, PBEffectTypes::ParamKey::ResourceName));
	const int32 IgnoreCount = FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::IgnoreCount));
	const float ChancePercent = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::ChancePercent));

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddOnHitDamageIgnoreChance(ResourceName, IgnoreCount, ChancePercent);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyComboExtraDamage(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const int32 ComboInterval = FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::ComboInterval));
	const int32 DamageAmount = FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::DamageAmount));

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddComboExtraDamageRule(ComboInterval, DamageAmount);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyMultiBall(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const int32 Count = FMath::Max(1, FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Count)));
	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddPendingMultiBallCount(Count);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyPierce(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const float DamageRetentionPercent = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::DamageRetentionPercent));
	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddPierceRule(DamageRetentionPercent);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyResourceCostStatBuff(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FName ResourceName = NormalizeResourceName(FindParamName(ParamRows, PBEffectTypes::ParamKey::ResourceName));
	if (ResourceName.IsNone())
	{
		return false;
	}

	const float ResourceCost = FMath::Max(0.0f, FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::ResourceCost)));
	const float AttackPercent = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::AttackPercent));

	bool bApplied = false;
	for (AActor* TargetActor : Context.TargetActors)
	{
		if (!IsValid(TargetActor))
		{
			continue;
		}

		if (UPBBaseResourceComponent* ResourceComponent = TargetActor->FindComponentByClass<UPBBaseResourceComponent>())
		{
			if (ResourceCost > 0.0f && ResourceComponent->HasResource(ResourceName))
			{
				ResourceComponent->ApplyResourceDelta(ResourceName, -ResourceCost);
			}
		}

		if (UPBBallEffectRuntimeComponent* RuntimeComponent = TargetActor->FindComponentByClass<UPBBallEffectRuntimeComponent>())
		{
			RuntimeComponent->AddAttackPercent(AttackPercent);
			bApplied = true;
		}
	}

	return bApplied;
}

bool UPBEffectHandler::ApplySwitchNextHitDamage(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddSwitchNextHitDamageRule(Value);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplySwitchElasticityBuff(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	FName StatName = NormalizeStatName(FindParamName(ParamRows, PBEffectTypes::ParamKey::StatName));
	if (StatName.IsNone())
	{
		StatName = PBStatNames::Bounciness;
	}

	FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	if (ModifyType.IsNone())
	{
		ModifyType = PBEffectTypes::ModifyType::PercentAdd;
	}

	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	float Duration = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Duration));
	if (Duration <= 0.0f)
	{
		Duration = 3.0f;
	}

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddSwitchTimedStatBuffRule(StatName, ModifyType, Value, Duration);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyReviveStatBuff(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FName StatName = NormalizeStatName(FindParamName(ParamRows, PBEffectTypes::ParamKey::StatName));
	const FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	if (StatName.IsNone() || ModifyType.IsNone())
	{
		return false;
	}

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddReviveStatBuffRule(StatName, ModifyType, Value);
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyFirstHitTimedStatBuff(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	FName StatName = NormalizeStatName(FindParamName(ParamRows, PBEffectTypes::ParamKey::StatName));
	if (StatName.IsNone())
	{
		StatName = PBStatNames::Attack;
	}

	FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	if (ModifyType.IsNone())
	{
		ModifyType = PBEffectTypes::ModifyType::PercentAdd;
	}

	FString ValueString = FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value);
	if (ValueString.IsEmpty())
	{
		ValueString = FindParamValue(ParamRows, PBEffectTypes::ParamKey::AttackPercent);
	}

	const float Value = FCString::Atof(*ValueString);
	float Duration = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Duration));
	if (Duration <= 0.0f)
	{
		Duration = 3.0f;
	}

	bool bApplied = false;
	for (UPBBallEffectRuntimeComponent* RuntimeComponent : ResolveRuntimeComponents(Context))
	{
		RuntimeComponent->AddFirstHitTimedStatBuffRule(StatName, ModifyType, Value, Duration);
		bApplied = true;
	}

	return bApplied;
}
