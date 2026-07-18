#pragma once

#include "CoreMinimal.h"

struct FPBGameplayEffectRow;

struct FPBBumperSharedEffectDefinition
{
	FName EffectId = NAME_None;
	FName EffectType = NAME_None;
	FName TargetType = NAME_None;
	FName TargetFilter = NAME_None;
	TMap<FName, FString> Parameters;

	bool TryGetFloat(FName Key, float& OutValue) const;
	bool TryGetInt(FName Key, int32& OutValue) const;
	bool TryGetName(FName Key, FName& OutValue) const;
};

namespace PBBumperSharedEffectAdapter
{
	bool ValidateContract(
		const FPBGameplayEffectRow& EffectRow,
		FName ExpectedEffectType,
		FName ExpectedTargetType,
		FName ExpectedTargetFilter,
		FString& OutError);

	bool Resolve(
		const UObject* WorldContext,
		FName SharedEffectId,
		FName ExpectedEffectType,
		FName ExpectedTargetType,
		FName ExpectedTargetFilter,
		TConstArrayView<FName> RequiredParameters,
		FPBBumperSharedEffectDefinition& OutDefinition,
		FString& OutError);

	bool ResolveResourceName(FName SheetName, FName& OutRuntimeName);
	bool ResolveStatName(FName SheetName, FName& OutRuntimeName);
}
