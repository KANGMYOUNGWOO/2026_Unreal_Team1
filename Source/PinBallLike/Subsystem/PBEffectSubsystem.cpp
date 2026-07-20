#include "PBEffectSubsystem.h"

#include "PinBallLike/Effect/Handlers/PBEffectHandler.h"
#include "PinBallLike/Effect/TargetResolver/PBEffectTargetResolver.h"
#include "PinBallLike/Struct/Synergy/PBSynergyState.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSynergyService.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectSetRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

void UPBEffectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EffectHandler = NewObject<UPBEffectHandler>(this);
	RegisterTargetResolver(NewObject<UPBEffectTargetResolver>(this));
}

void UPBEffectSubsystem::Deinitialize()
{
	EffectHandler = nullptr;
	TargetResolverMap.Reset();
	Super::Deinitialize();
}

bool UPBEffectSubsystem::ApplyEffect(const FName EffectId, const FPBEffectContext& Context)
{
	if (EffectId.IsNone())
	{
		return false;
	}

	const UPBTableDataSubsystem* TableDataSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!TableDataSubsystem)
	{
		return false;
	}

	TArray<FPBEffectSetRow> EffectSetRows;
	if (TableDataSubsystem->GetEffectSetRows(EffectId, EffectSetRows))
	{
		bool bAppliedAny = false;
		for (const FPBEffectSetRow& EffectSetRow : EffectSetRows)
		{
			if (ApplySingleEffect(EffectSetRow.EffectId, Context))
			{
				bAppliedAny = true;
			}
		}

		return bAppliedAny;
	}

	return ApplySingleEffect(EffectId, Context);
}

int32 UPBEffectSubsystem::ApplyEffects(const TArray<FName>& EffectIds, const FPBEffectContext& Context)
{
	int32 AppliedCount = 0;
	for (const FName EffectId : EffectIds)
	{
		if (ApplyEffect(EffectId, Context))
		{
			++AppliedCount;
		}
	}

	return AppliedCount;
}

int32 UPBEffectSubsystem::NotifyTrigger(
	const FGameplayTag TriggerTag,
	const FPBEffectContext& Context)
{
	if (!TriggerTag.IsValid())
	{
		return 0;
	}

	TArray<FName> EffectSetIds;
	GetActiveEffectSetIdsFromSynergies(EffectSetIds);

	int32 AppliedCount = 0;
	for (const FName EffectSetId : EffectSetIds)
	{
		if (ApplyEffectForTrigger(EffectSetId, TriggerTag, Context))
		{
			++AppliedCount;
		}
	}

	return AppliedCount;
}

bool UPBEffectSubsystem::ApplySingleEffect(const FName EffectId, const FPBEffectContext& Context)
{
	if (EffectId.IsNone())
	{
		return false;
	}

	const UPBTableDataSubsystem* TableDataSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!TableDataSubsystem)
	{
		return false;
	}

	FPBEffectTableRow EffectRow;
	if (!TableDataSubsystem->FindEffectRow(EffectId, EffectRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] Effect row not found. EffectId=%s"),
			*EffectId.ToString());
		return false;
	}
	if (!ValidateEffectRow(EffectId, EffectRow, false))
	{
		return false;
	}

	TArray<FPBEffectParamRow> ParamRows;
	TableDataSubsystem->GetEffectParamRows(EffectId, ParamRows);

	FPBEffectContext ResolvedContext = Context;
	ResolveTargetActors(EffectRow, Context, ResolvedContext.TargetActors);

	if (!IsValid(EffectHandler))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] EffectHandler not initialized. EffectId=%s"),
			*EffectId.ToString());
		return false;
	}

	return EffectHandler->ExecuteEffect(EffectId, EffectRow, ParamRows, ResolvedContext);
}

bool UPBEffectSubsystem::ApplyEffectForTrigger(
	const FName EffectId,
	const FGameplayTag TriggerTag,
	const FPBEffectContext& Context)
{
	if (EffectId.IsNone() || !TriggerTag.IsValid())
	{
		return false;
	}

	const UPBTableDataSubsystem* TableDataSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!TableDataSubsystem)
	{
		return false;
	}

	TArray<FPBEffectSetRow> EffectSetRows;
	if (TableDataSubsystem->GetEffectSetRows(EffectId, EffectSetRows))
	{
		bool bAppliedAny = false;
		for (const FPBEffectSetRow& EffectSetRow : EffectSetRows)
		{
			if (ApplySingleEffectForTrigger(EffectSetRow.EffectId, TriggerTag, Context))
			{
				bAppliedAny = true;
			}
		}

		return bAppliedAny;
	}

	return ApplySingleEffectForTrigger(EffectId, TriggerTag, Context);
}

bool UPBEffectSubsystem::ApplySingleEffectForTrigger(
	const FName EffectId,
	const FGameplayTag TriggerTag,
	const FPBEffectContext& Context)
{
	if (EffectId.IsNone() || !TriggerTag.IsValid())
	{
		return false;
	}

	const UPBTableDataSubsystem* TableDataSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!TableDataSubsystem)
	{
		return false;
	}

	FPBEffectTableRow EffectRow;
	if (!TableDataSubsystem->FindEffectRow(EffectId, EffectRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] Effect row not found. EffectId=%s"),
			*EffectId.ToString());
		return false;
	}
	if (!ValidateEffectRow(EffectId, EffectRow, true))
	{
		return false;
	}

	if (!EffectRow.TriggerTag.MatchesTagExact(TriggerTag))
	{
		return false;
	}

	return ApplySingleEffect(EffectId, Context);
}

bool UPBEffectSubsystem::ValidateEffectRow(
	const FName EffectId,
	const FPBEffectTableRow& EffectRow,
	const bool bRequireTriggerTag) const
{
	if (EffectRow.EffectType.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] EffectType is empty. EffectId=%s"),
			*EffectId.ToString());
		return false;
	}

	if (!IsValid(EffectHandler))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] EffectHandler not initialized. EffectId=%s"),
			*EffectId.ToString());
		return false;
	}

	if (!EffectHandler->IsEffectTypeSupported(EffectRow.EffectType))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] Unsupported EffectType. EffectId=%s EffectType=%s"),
			*EffectId.ToString(),
			*EffectRow.EffectType.ToString());
		return false;
	}

	if (EffectRow.TargetType.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] TargetType is empty. EffectId=%s EffectType=%s"),
			*EffectId.ToString(),
			*EffectRow.EffectType.ToString());
		return false;
	}

	if (!IsValid(FindTargetResolver(EffectRow.TargetType)))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] Unsupported TargetType. EffectId=%s TargetType=%s"),
			*EffectId.ToString(),
			*EffectRow.TargetType.ToString());
		return false;
	}

	if (bRequireTriggerTag && !EffectRow.TriggerTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] TriggerTag is required for trigger path. EffectId=%s EffectType=%s"),
			*EffectId.ToString(),
			*EffectRow.EffectType.ToString());
		return false;
	}

	return true;
}

void UPBEffectSubsystem::GetActiveEffectSetIdsFromSynergies(TArray<FName>& OutEffectSetIds) const
{
	OutEffectSetIds.Reset();

	const UGameInstance* GameInstance = GetGameInstance();
	const UPBBallDeckSubsystem* DeckSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
	const UPBBallDeckSynergyService* SynergyService =
		DeckSubsystem ? DeckSubsystem->GetSynergyService() : nullptr;
	if (!SynergyService)
	{
		return;
	}

	for (const FPBSynergyState& SynergyState : SynergyService->GetActiveSynergyStates())
	{
		if (!SynergyState.ActiveEffectSetId.IsNone())
		{
			OutEffectSetIds.AddUnique(SynergyState.ActiveEffectSetId);
		}
	}
}

void UPBEffectSubsystem::RegisterTargetResolver(UPBEffectTargetResolver* Resolver)
{
	if (!IsValid(Resolver))
	{
		return;
	}

	TArray<FName> TargetTypes;
	Resolver->GetSupportedTargetTypes(TargetTypes);
	for (const FName TargetType : TargetTypes)
	{
		if (!TargetType.IsNone())
		{
			TargetResolverMap.Add(TargetType, Resolver);
		}
	}
}

UPBEffectTargetResolver* UPBEffectSubsystem::FindTargetResolver(const FName TargetType) const
{
	const TObjectPtr<UPBEffectTargetResolver>* Resolver = TargetResolverMap.Find(TargetType);
	return Resolver ? Resolver->Get() : nullptr;
}

void UPBEffectSubsystem::ResolveTargetActors(
	const FPBEffectTableRow& EffectRow,
	const FPBEffectContext& Context,
	TArray<TObjectPtr<AActor>>& OutTargetActors) const
{
	OutTargetActors.Reset();

	UPBEffectTargetResolver* Resolver = FindTargetResolver(EffectRow.TargetType);
	if (!IsValid(Resolver))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Effect] TargetResolver not found. TargetType=%s"),
			*EffectRow.TargetType.ToString());
		return;
	}

	Resolver->ResolveTargetActors(EffectRow, Context, OutTargetActors);
}
