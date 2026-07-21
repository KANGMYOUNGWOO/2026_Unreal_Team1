#pragma once

#include "CoreMinimal.h"

class UPBTableDataSubsystem;

struct PINBALLLIKE_API FPBCollectionEffectSetProjection
{
	FName EffectSetId = NAME_None;
	FText EffectSummary;
	FText ParameterSummary;
	FText TargetSummary;
	FText ValidationText;
	int32 DeclaredEffectCount = 0;
	int32 ResolvedEffectCount = 0;
	bool bIsValid = false;
};

class PINBALLLIKE_API FPBCollectionEffectSetProjector final
{
public:
	static FPBCollectionEffectSetProjection Build(
		const UPBTableDataSubsystem& TableData,
		FName EffectSetId);
};
