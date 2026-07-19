#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PBEffectTargetResolver.generated.h"

struct FPBEffectContext;
struct FPBEffectTableRow;

UCLASS()
class PINBALLLIKE_API UPBEffectTargetResolver : public UObject
{
	GENERATED_BODY()

public:
	void GetSupportedTargetTypes(TArray<FName>& OutTargetTypes) const;
	void ResolveTargetActors(
		const FPBEffectTableRow& EffectRow,
		const FPBEffectContext& Context,
		TArray<TObjectPtr<AActor>>& OutTargetActors) const;

private:
	void AddMatchingContextActors(
		const FPBEffectTableRow& EffectRow,
		const FPBEffectContext& Context,
		TArray<TObjectPtr<AActor>>& OutTargetActors) const;

	bool DoesActorMatchTargetType(const AActor* Actor, FName TargetType) const;
	bool DoesActorMatchTargetFilter(const AActor* Actor, FName TargetFilter) const;
};
