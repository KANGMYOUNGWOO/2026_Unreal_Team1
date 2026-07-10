// Fill out your copyright notice in the Description page of Project Settings.

#include "PBBaseStatusEffect.h"

#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Struct/StatusEffect/PBStatusEffectTriggerEvents.h"

void UPBBaseStatusEffect::InitializeStatusEffect(
	UPBStatusEffectComponent* InOwnerComponent,
	const FPBStatusEffectRow& InStatusEffectRow,
	const TArray<FPBStatusEffectModifierRow>& InModifierRows,
	const TArray<FPBStatusEffectTriggerRow>& InTriggerRows)
{
	OwnerComponent = InOwnerComponent;
	StatusEffectRow = InStatusEffectRow;
	ModifierRows = InModifierRows;
	TriggerRows = InTriggerRows;
	StackCount = 1;
	RefreshDuration();
}

AActor* UPBBaseStatusEffect::GetOwnerActor() const
{
	const UPBStatusEffectComponent* Component = OwnerComponent.Get();
	return IsValid(Component) ? Component->GetOwner() : nullptr;
}

bool UPBBaseStatusEffect::HasTag(const FName Tag) const
{
	if (Tag.IsNone())
	{
		return false;
	}

	TArray<FString> TagStrings;
	StatusEffectRow.Tags.ParseIntoArray(TagStrings, TEXT(","), true);
	for (FString& TagString : TagStrings)
	{
		TagString.TrimStartAndEndInline();
		if (FName(*TagString) == Tag)
		{
			return true;
		}
	}

	return false;
}

void UPBBaseStatusEffect::RefreshDuration()
{
	IntervalElapsed = 0.0f;
	RemainingDuration = StatusEffectRow.DurationPolicy == EPBStatusEffectDurationPolicy::Seconds
		? FMath::Max(0.0f, StatusEffectRow.DurationValue)
		: 0.0f;
}

bool UPBBaseStatusEffect::IsExpired() const
{
	return StatusEffectRow.DurationPolicy == EPBStatusEffectDurationPolicy::Seconds
		&& RemainingDuration <= 0.0f;
}

void UPBBaseStatusEffect::TickStatusEffect(const float DeltaTime)
{
	if (StatusEffectRow.Interval > 0.0f)
	{
		IntervalElapsed += DeltaTime;
		while (IntervalElapsed >= StatusEffectRow.Interval)
		{
			IntervalElapsed -= StatusEffectRow.Interval;
			ExecuteStatusEffect(PBStatusEffectTriggerEvents::Interval);
		}
	}

	if (StatusEffectRow.DurationPolicy == EPBStatusEffectDurationPolicy::Seconds)
	{
		RemainingDuration -= DeltaTime;
	}
}

void UPBBaseStatusEffect::AddStack()
{
	RefreshDuration();

	if (StatusEffectRow.StackType != EPBStatusEffectStackType::Add)
	{
		return;
	}

	++StackCount;
}

void UPBBaseStatusEffect::ExecuteStatusEffect(const FName TriggerEvent)
{
}
