#include "PBEffectHandler.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Ball/Component/PBBallResourceComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

bool UPBEffectHandler::ApplyStatusEffect(
	const FName,
	const FPBEffectTableRow& EffectRow,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	FName StatusEffectId = EffectRow.LinkedStatusEffectId;
	if (StatusEffectId.IsNone())
	{
		StatusEffectId = FindParamName(ParamRows, PBEffectTypes::ParamKey::StatusEffectId);
	}
	if (StatusEffectId.IsNone())
	{
		return false;
	}

	bool bApplied = false;
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBStatusEffectComponent* StatusEffectComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBStatusEffectComponent>()
			: nullptr;
		if (IsValid(StatusEffectComponent))
		{
			bApplied |= StatusEffectComponent->ApplyStatusEffect(StatusEffectId);
		}
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyStatBuff(
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
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBBaseStatComponent* StatComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBaseStatComponent>()
			: nullptr;
		if (!StatComponent)
		{
			continue;
		}

		const int32 CurrentValue = StatComponent->GetStat(StatName);
		StatComponent->SetStat(StatName, CalculateModifiedStatValue(CurrentValue, ModifyType, Value));
		bApplied = true;
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyResourceBuff(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FName ResourceName = NormalizeResourceName(FindParamName(ParamRows, PBEffectTypes::ParamKey::ResourceName));
	const FName ApplyTo = FindParamName(ParamRows, PBEffectTypes::ParamKey::ApplyTo);
	const FName ModifyType = FindParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
	const float Value = FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	if (ResourceName.IsNone() || ApplyTo.IsNone() || ModifyType.IsNone())
	{
		return false;
	}

	bool bApplied = false;
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBBaseResourceComponent* ResourceComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBaseResourceComponent>()
			: nullptr;
		if (ResourceComponent && ResourceComponent->HasResource(ResourceName))
		{
			bApplied |= ApplyResourceBuffToComponent(ResourceComponent, ResourceName, ApplyTo, ModifyType, Value);
		}
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyDamageTakenModifier(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const int32 DamageTakenBonusPercent = FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value));
	if (DamageTakenBonusPercent == 0)
	{
		return false;
	}

	bool bApplied = false;
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBBossStatComponent* BossStatComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBossStatComponent>()
			: nullptr;
		if (BossStatComponent)
		{
			BossStatComponent->ApplyDamageTakenMultiplierPercentDelta(DamageTakenBonusPercent);
			bApplied = true;
		}
	}
	if (bApplied)
	{
		return true;
	}

	const UObject* WorldContextObject = IsValid(Context.WorldContextObject)
		? Context.WorldContextObject.Get()
		: Context.SourceActor.Get();
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (UPBBossStatComponent* BossStatComponent = It->FindComponentByClass<UPBBossStatComponent>())
		{
			BossStatComponent->ApplyDamageTakenMultiplierPercentDelta(DamageTakenBonusPercent);
			bApplied = true;
		}
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyDamageIgnore(
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

	FString IgnoreCountValue = FindParamValue(ParamRows, PBEffectTypes::ParamKey::IgnoreCount);
	if (IgnoreCountValue.IsEmpty())
	{
		IgnoreCountValue = FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value);
	}
	const int32 IgnoreCount = FMath::Max(1, FCString::Atoi(*IgnoreCountValue));

	bool bApplied = false;
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBBallResourceComponent* ResourceComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBallResourceComponent>()
			: nullptr;
		if (ResourceComponent && ResourceComponent->HasResource(ResourceName))
		{
			ResourceComponent->AddDamageIgnoreCount(ResourceName, IgnoreCount);
			bApplied = true;
		}
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyReviveOnZero(
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

	const int32 Count = FMath::Max(1, FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Count)));
	const float ReviveValue = FMath::Max(1.0f, FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::ReviveValue)));

	bool bApplied = false;
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBBallResourceComponent* ResourceComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBallResourceComponent>()
			: nullptr;
		if (ResourceComponent && ResourceComponent->HasResource(ResourceName))
		{
			ResourceComponent->AddReviveOnZeroCount(ResourceName, Count, ReviveValue);
			bApplied = true;
		}
	}

	return bApplied;
}

bool UPBEffectHandler::ApplyPostDamageHeal(
	const FName,
	const FPBEffectTableRow&,
	const TArray<FPBEffectParamRow>& ParamRows,
	const FPBEffectContext& Context)
{
	const FName ParamResourceName = NormalizeResourceName(FindParamName(ParamRows, PBEffectTypes::ParamKey::ResourceName));
	const FName ResourceName = ParamResourceName.IsNone() ? PBResourceNames::Health : ParamResourceName;
	const int32 Count = FMath::Max(1, FCString::Atoi(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Count)));
	const float Value = FMath::Max(1.0f, FCString::Atof(*FindParamValue(ParamRows, PBEffectTypes::ParamKey::Value)));

	bool bApplied = false;
	for (AActor* TargetActor : Context.TargetActors)
	{
		UPBBallResourceComponent* ResourceComponent = IsValid(TargetActor)
			? TargetActor->FindComponentByClass<UPBBallResourceComponent>()
			: nullptr;
		if (ResourceComponent && ResourceComponent->HasResource(ResourceName))
		{
			ResourceComponent->AddPostDamageHealCount(ResourceName, Count, Value);
			bApplied = true;
		}
	}

	return bApplied;
}
