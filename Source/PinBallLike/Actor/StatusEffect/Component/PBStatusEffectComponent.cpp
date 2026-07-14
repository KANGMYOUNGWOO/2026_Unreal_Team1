// Fill out your copyright notice in the Description page of Project Settings.


#include "PBStatusEffectComponent.h"

#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/Factory/PBStatusEffectFactory.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"
#include "PinBallLike/Utils/PBSubsystemUtils.h"

UPBStatusEffectComponent::UPBStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPBStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPBStatusEffectComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<TObjectPtr<UPBBaseStatusEffect>> EffectSnapshot = ActiveStatusEffects;
	for (UPBBaseStatusEffect* StatusEffect : EffectSnapshot)
	{
		if (IsValid(StatusEffect) && ActiveStatusEffects.ContainsByPredicate(
			[StatusEffect](const TObjectPtr<UPBBaseStatusEffect>& ActiveStatusEffect)
			{
				return ActiveStatusEffect.Get() == StatusEffect;
			}))
		{
			StatusEffect->TickStatusEffect(DeltaTime);
		}
	}

	RemoveExpiredEffects();
}

bool UPBStatusEffectComponent::ApplyStatusEffect(const FName StatusEffectId)
{
	if (StatusEffectId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatusEffect] Apply failed. StatusEffectId is none. Owner=%s Component=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(this));
		return false;
	}

	if (UPBBaseStatusEffect* ExistingEffect = GetStatusEffect(StatusEffectId))
	{
		ExistingEffect->AddStack();
		ExistingEffect->ExecuteStatusEffect(PBStatusEffectTriggerEvents::Stacked);
		OnStatusEffectApplied.Broadcast(StatusEffectId, ExistingEffect->GetStackCount());
		return true;
	}

	UPBBaseStatusEffect* NewStatusEffect = CreateStatusEffect(StatusEffectId);
	if (!IsValid(NewStatusEffect))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatusEffect] Apply failed. CreateStatusEffect returned invalid. StatusEffectId=%s Owner=%s Component=%s"),
			*StatusEffectId.ToString(),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(this));
		return false;
	}

	ActiveStatusEffects.Add(NewStatusEffect);
	NewStatusEffect->ExecuteStatusEffect(PBStatusEffectTriggerEvents::Applied);
	OnStatusEffectApplied.Broadcast(StatusEffectId, NewStatusEffect->GetStackCount());
	return true;
}

bool UPBStatusEffectComponent::RemoveStatusEffect(const FName StatusEffectId)
{
	const int32 Index = FindStatusEffectIndex(StatusEffectId);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	RemoveStatusEffectAt(Index, false);
	return true;
}

void UPBStatusEffectComponent::RemoveAllStatusEffects()
{
	for (int32 Index = ActiveStatusEffects.Num() - 1; Index >= 0; --Index)
	{
		RemoveStatusEffectAt(Index, false);
	}
}

bool UPBStatusEffectComponent::HasStatusEffect(const FName StatusEffectId) const
{
	return GetStatusEffect(StatusEffectId) != nullptr;
}

bool UPBStatusEffectComponent::HasEffectTag(const FName Tag) const
{
	if (Tag.IsNone())
	{
		return false;
	}

	for (const UPBBaseStatusEffect* StatusEffect : ActiveStatusEffects)
	{
		if (IsValid(StatusEffect) && StatusEffect->HasTag(Tag))
		{
			return true;
		}
	}

	return false;
}

int32 UPBStatusEffectComponent::GetStatusEffectStackCount(const FName StatusEffectId) const
{
	const UPBBaseStatusEffect* StatusEffect = GetStatusEffect(StatusEffectId);
	return IsValid(StatusEffect) ? StatusEffect->GetStackCount() : 0;
}

UPBBaseStatusEffect* UPBStatusEffectComponent::GetStatusEffect(const FName StatusEffectId) const
{
	const int32 Index = FindStatusEffectIndex(StatusEffectId);
	return Index != INDEX_NONE ? ActiveStatusEffects[Index] : nullptr;
}

void UPBStatusEffectComponent::GetActiveStatusEffects(TArray<UPBBaseStatusEffect*>& OutStatusEffects) const
{
	OutStatusEffects.Reset();
	for (UPBBaseStatusEffect* StatusEffect : ActiveStatusEffects)
	{
		if (IsValid(StatusEffect))
		{
			OutStatusEffects.Add(StatusEffect);
		}
	}
}

void UPBStatusEffectComponent::NotifyStatusEffectEvent(
	const FName TriggerEvent)
{
	TArray<TObjectPtr<UPBBaseStatusEffect>> EffectSnapshot = ActiveStatusEffects;
	for (UPBBaseStatusEffect* StatusEffect : EffectSnapshot)
	{
		if (IsValid(StatusEffect) && ActiveStatusEffects.ContainsByPredicate(
			[StatusEffect](const TObjectPtr<UPBBaseStatusEffect>& ActiveStatusEffect)
			{
				return ActiveStatusEffect.Get() == StatusEffect;
			}))
		{
			StatusEffect->ExecuteStatusEffect(TriggerEvent);
		}
	}
}

UPBBaseStatusEffect* UPBStatusEffectComponent::CreateStatusEffect(const FName StatusEffectId)
{
	UPBTableDataSubsystem* TableDataSubsystem = PBSubsystemUtils::GetGameInstanceSubsystem<UPBTableDataSubsystem>(this);
	if (!TableDataSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatusEffect] Create failed. Missing TableDataSubsystem. StatusEffectId=%s Owner=%s Component=%s"),
			*StatusEffectId.ToString(),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(this));
		return nullptr;
	}

	FPBStatusEffectRow StatusEffectRow;
	if (!TableDataSubsystem->FindStatusEffectRow(StatusEffectId, StatusEffectRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StatusEffect] Create failed. StatusEffect row not found. StatusEffectId=%s Owner=%s Component=%s"),
			*StatusEffectId.ToString(),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(this));
		return nullptr;
	}

	TArray<FPBStatusEffectModifierRow> ModifierRows;
	TArray<FPBStatusEffectTriggerRow> TriggerRows;
	TableDataSubsystem->GetStatusEffectModifierRows(StatusEffectId, ModifierRows);
	TableDataSubsystem->GetStatusEffectTriggerRows(StatusEffectId, TriggerRows);

	return PBStatusEffectFactory::CreateStatusEffect(
		this,
		this,
		StatusEffectId,
		StatusEffectRow,
		ModifierRows,
		TriggerRows);
}

int32 UPBStatusEffectComponent::FindStatusEffectIndex(const FName StatusEffectId) const
{
	if (StatusEffectId.IsNone())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < ActiveStatusEffects.Num(); ++Index)
	{
		const UPBBaseStatusEffect* StatusEffect = ActiveStatusEffects[Index];
		if (IsValid(StatusEffect) && StatusEffect->GetStatusEffectId() == StatusEffectId)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void UPBStatusEffectComponent::RemoveExpiredEffects()
{
	for (int32 Index = ActiveStatusEffects.Num() - 1; Index >= 0; --Index)
	{
		const UPBBaseStatusEffect* StatusEffect = ActiveStatusEffects[Index];
		if (IsValid(StatusEffect) && StatusEffect->IsExpired())
		{
			RemoveStatusEffectAt(Index, true);
		}
	}
}

void UPBStatusEffectComponent::RemoveStatusEffectAt(const int32 Index, const bool bExpired)
{
	if (!ActiveStatusEffects.IsValidIndex(Index))
	{
		return;
	}

	UPBBaseStatusEffect* StatusEffect = ActiveStatusEffects[Index];
	if (!IsValid(StatusEffect))
	{
		ActiveStatusEffects.RemoveAt(Index);
		return;
	}

	const FName StatusEffectId = StatusEffect->GetStatusEffectId();
	const int32 StackCount = StatusEffect->GetStackCount();

	StatusEffect->ExecuteStatusEffect(
		bExpired ? PBStatusEffectTriggerEvents::Expired : PBStatusEffectTriggerEvents::Removed);

	ActiveStatusEffects.RemoveAt(Index);
	OnStatusEffectRemoved.Broadcast(StatusEffectId, StackCount);
}
