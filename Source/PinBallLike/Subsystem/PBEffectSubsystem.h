#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PBEffectSubsystem.generated.h"

class UPBEffectHandler;
class UPBEffectTargetResolver;

struct FPBEffectParamRow;
struct FPBEffectTableRow;

UCLASS()
class PINBALLLIKE_API UPBEffectSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Effect")
	bool ApplyEffect(FName EffectId, const FPBEffectContext& Context);

	int32 ApplyEffects(const TArray<FName>& EffectIds, const FPBEffectContext& Context);

	UFUNCTION(BlueprintCallable, Category = "Effect")
	int32 NotifyTrigger(FGameplayTag TriggerTag, const FPBEffectContext& Context);

	void RegisterTargetResolver(UPBEffectTargetResolver* Resolver);

private:
	bool ApplySingleEffect(FName EffectId, const FPBEffectContext& Context);
	bool ApplyEffectForTrigger(FName EffectId, FGameplayTag TriggerTag, const FPBEffectContext& Context);
	bool ApplySingleEffectForTrigger(FName EffectId, FGameplayTag TriggerTag, const FPBEffectContext& Context);
	bool ValidateEffectRow(FName EffectId, const FPBEffectTableRow& EffectRow, bool bRequireTriggerTag) const;
	void GetActiveEffectSetIdsFromSynergies(TArray<FName>& OutEffectSetIds) const;
	UPBEffectTargetResolver* FindTargetResolver(FName TargetType) const;
	void ResolveTargetActors(
		const FPBEffectTableRow& EffectRow,
		const FPBEffectContext& Context,
		TArray<TObjectPtr<AActor>>& OutTargetActors) const;

	UPROPERTY(Transient)
	TObjectPtr<UPBEffectHandler> EffectHandler;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UPBEffectTargetResolver>> TargetResolverMap;
};
