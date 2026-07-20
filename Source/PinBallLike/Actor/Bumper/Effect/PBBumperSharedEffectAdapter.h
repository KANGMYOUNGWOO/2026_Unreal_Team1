#pragma once

#include "CoreMinimal.h"

struct FPBEffectTableRow;

enum class EPBBumperSharedEffectContractKind : uint8
{
	HandlerCompatible,
	BumperExtension
};

struct FPBBumperSharedEffectDefinition
{
	FName EffectId = NAME_None;
	FName EffectType = NAME_None;
	FName TargetType = NAME_None;
	FName TargetFilter = NAME_None;
	EPBBumperSharedEffectContractKind ContractKind = EPBBumperSharedEffectContractKind::HandlerCompatible;
	TMap<FName, FString> Parameters;

	bool TryGetFloat(FName Key, float& OutValue) const;
	bool TryGetInt(FName Key, int32& OutValue) const;
	bool TryGetName(FName Key, FName& OutValue) const;
};

namespace PBBumperSharedEffectAdapter
{
	PINBALLLIKE_API bool ValidateContract(
		const FPBEffectTableRow& EffectRow,
		FName ExpectedEffectType,
		FName ExpectedTargetType,
		FName ExpectedTargetFilter,
		FString& OutError);

	PINBALLLIKE_API bool IsHandlerCompatibleEffectType(FName EffectType);
	PINBALLLIKE_API bool IsBumperExtensionEffectType(FName EffectType);

	PINBALLLIKE_API bool Resolve(
		const UObject* WorldContext,
		FName SharedEffectId,
		FName ExpectedEffectType,
		FName ExpectedTargetType,
		FName ExpectedTargetFilter,
		TConstArrayView<FName> RequiredParameters,
		FPBBumperSharedEffectDefinition& OutDefinition,
		FString& OutError);

	PINBALLLIKE_API bool ResolveResourceName(FName SheetName, FName& OutRuntimeName);
	PINBALLLIKE_API bool ResolveStatName(FName SheetName, FName& OutRuntimeName);
}
