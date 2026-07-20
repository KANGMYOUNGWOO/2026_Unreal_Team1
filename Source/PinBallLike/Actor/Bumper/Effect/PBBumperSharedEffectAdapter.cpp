#include "PBBumperSharedEffectAdapter.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

namespace
{
	const TSet<FName>& GetHandlerCompatibleEffectTypes()
	{
		static const TSet<FName> SupportedTypes =
		{
			PBEffectTypes::EffectType::ResourceCostStatBuff,
			PBEffectTypes::EffectType::ComboExtraDamage,
			PBEffectTypes::EffectType::PostDamageHeal,
			PBEffectTypes::EffectType::StatBuff
		};
		return SupportedTypes;
	}

	const TSet<FName>& GetBumperExtensionEffectTypes()
	{
		static const TSet<FName> ExtensionTypes =
		{
			TEXT("VelocityScaledDamage")
		};
		return ExtensionTypes;
	}

	const UPBTableDataSubsystem* ResolveTableSubsystem(const UObject* WorldContext)
	{
		const UWorld* World = IsValid(WorldContext) ? WorldContext->GetWorld() : nullptr;
		const UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;
		return IsValid(GameInstance)
			? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
			: nullptr;
	}
}

bool PBBumperSharedEffectAdapter::ValidateContract(
	const FPBEffectTableRow& EffectRow,
	const FName ExpectedEffectType,
	const FName ExpectedTargetType,
	const FName ExpectedTargetFilter,
	FString& OutError)
{
	OutError.Reset();
	if (!IsHandlerCompatibleEffectType(EffectRow.EffectType)
		&& !IsBumperExtensionEffectType(EffectRow.EffectType))
	{
		OutError = FString::Printf(
			TEXT("EffectType '%s' is not supported by the bumper adapter."),
			*EffectRow.EffectType.ToString());
		return false;
	}

	if (!ExpectedEffectType.IsNone() && EffectRow.EffectType != ExpectedEffectType)
	{
		OutError = FString::Printf(
			TEXT("EffectType mismatch. Expected='%s' Actual='%s'."),
			*ExpectedEffectType.ToString(),
			*EffectRow.EffectType.ToString());
		return false;
	}
	if (!ExpectedTargetType.IsNone() && EffectRow.TargetType != ExpectedTargetType)
	{
		OutError = FString::Printf(
			TEXT("TargetType mismatch. Expected='%s' Actual='%s'."),
			*ExpectedTargetType.ToString(),
			*EffectRow.TargetType.ToString());
		return false;
	}
	if (!ExpectedTargetFilter.IsNone() && EffectRow.TargetFilter != ExpectedTargetFilter)
	{
		OutError = FString::Printf(
			TEXT("TargetFilter mismatch. Expected='%s' Actual='%s'."),
			*ExpectedTargetFilter.ToString(),
			*EffectRow.TargetFilter.ToString());
		return false;
	}

	return true;
}

bool PBBumperSharedEffectAdapter::IsHandlerCompatibleEffectType(const FName EffectType)
{
	return GetHandlerCompatibleEffectTypes().Contains(EffectType);
}

bool PBBumperSharedEffectAdapter::IsBumperExtensionEffectType(const FName EffectType)
{
	return GetBumperExtensionEffectTypes().Contains(EffectType);
}

bool FPBBumperSharedEffectDefinition::TryGetFloat(const FName Key, float& OutValue) const
{
	OutValue = 0.0f;
	const FString* Value = Parameters.Find(Key);
	if (!Value || !LexTryParseString(OutValue, **Value) || !FMath::IsFinite(OutValue))
	{
		OutValue = 0.0f;
		return false;
	}
	return true;
}

bool FPBBumperSharedEffectDefinition::TryGetInt(const FName Key, int32& OutValue) const
{
	OutValue = 0;
	const FString* Value = Parameters.Find(Key);
	return Value && LexTryParseString(OutValue, **Value);
}

bool FPBBumperSharedEffectDefinition::TryGetName(const FName Key, FName& OutValue) const
{
	OutValue = NAME_None;
	const FString* Value = Parameters.Find(Key);
	if (!Value)
	{
		return false;
	}

	const FString TrimmedValue = Value->TrimStartAndEnd();
	if (TrimmedValue.IsEmpty() || TrimmedValue.Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	OutValue = FName(*TrimmedValue);
	return true;
}

bool PBBumperSharedEffectAdapter::Resolve(
	const UObject* WorldContext,
	const FName SharedEffectId,
	const FName ExpectedEffectType,
	const FName ExpectedTargetType,
	const FName ExpectedTargetFilter,
	const TConstArrayView<FName> RequiredParameters,
	FPBBumperSharedEffectDefinition& OutDefinition,
	FString& OutError)
{
	OutDefinition = FPBBumperSharedEffectDefinition();
	OutError.Reset();
	if (SharedEffectId.IsNone())
	{
		OutError = TEXT("SharedEffectId is empty.");
		return false;
	}

	const UPBTableDataSubsystem* TableSubsystem = ResolveTableSubsystem(WorldContext);
	if (!IsValid(TableSubsystem))
	{
		OutError = TEXT("PBTableDataSubsystem is unavailable.");
		return false;
	}

	FPBEffectTableRow EffectRow;
	if (!TableSubsystem->FindEffectRow(SharedEffectId, EffectRow))
	{
		OutError = FString::Printf(
			TEXT("Effect row '%s' is missing from the canonical Effect table."),
			*SharedEffectId.ToString());
		return false;
	}

	if (!ValidateContract(
		EffectRow,
		ExpectedEffectType,
		ExpectedTargetType,
		ExpectedTargetFilter,
		OutError))
	{
		return false;
	}

	TArray<FPBEffectParamRow> ParamRows;
	if (!TableSubsystem->GetEffectParamRows(SharedEffectId, ParamRows))
	{
		OutError = FString::Printf(
			TEXT("Gameplay Effect parameters for '%s' are missing."),
			*SharedEffectId.ToString());
		return false;
	}

	OutDefinition.EffectId = SharedEffectId;
	OutDefinition.EffectType = EffectRow.EffectType;
	OutDefinition.TargetType = EffectRow.TargetType;
	OutDefinition.TargetFilter = EffectRow.TargetFilter;
	OutDefinition.ContractKind = IsBumperExtensionEffectType(EffectRow.EffectType)
		? EPBBumperSharedEffectContractKind::BumperExtension
		: EPBBumperSharedEffectContractKind::HandlerCompatible;
	for (const FPBEffectParamRow& ParamRow : ParamRows)
	{
		if (OutDefinition.Parameters.Contains(ParamRow.ParamKey))
		{
			OutError = FString::Printf(
				TEXT("Duplicate parameter '%s' in effect '%s'."),
				*ParamRow.ParamKey.ToString(),
				*SharedEffectId.ToString());
			OutDefinition = FPBBumperSharedEffectDefinition();
			return false;
		}
		OutDefinition.Parameters.Add(ParamRow.ParamKey, ParamRow.ParamValue);
	}

	for (const FName RequiredParameter : RequiredParameters)
	{
		const FString* Value = OutDefinition.Parameters.Find(RequiredParameter);
		if (!Value || Value->TrimStartAndEnd().IsEmpty())
		{
			OutError = FString::Printf(
				TEXT("Required parameter '%s' is missing from effect '%s'."),
				*RequiredParameter.ToString(),
				*SharedEffectId.ToString());
			OutDefinition = FPBBumperSharedEffectDefinition();
			return false;
		}
	}

	return true;
}

bool PBBumperSharedEffectAdapter::ResolveResourceName(
	const FName SheetName,
	FName& OutRuntimeName)
{
	OutRuntimeName = NAME_None;
	if (SheetName == TEXT("Health") || SheetName == PBResourceNames::Health)
	{
		OutRuntimeName = PBResourceNames::Health;
	}
	else if (SheetName == TEXT("Mana") || SheetName == PBResourceNames::Mana)
	{
		OutRuntimeName = PBResourceNames::Mana;
	}
	else if (SheetName == PBResourceNames::Shield)
	{
		OutRuntimeName = PBResourceNames::Shield;
	}
	return !OutRuntimeName.IsNone();
}

bool PBBumperSharedEffectAdapter::ResolveStatName(
	const FName SheetName,
	FName& OutRuntimeName)
{
	OutRuntimeName = NAME_None;
	if (SheetName == PBStatNames::Attack)
	{
		OutRuntimeName = PBStatNames::Attack;
	}
	else if (SheetName == TEXT("ManaRecovery") || SheetName == PBStatNames::ManaRegen)
	{
		OutRuntimeName = PBStatNames::ManaRegen;
	}
	return !OutRuntimeName.IsNone();
}
