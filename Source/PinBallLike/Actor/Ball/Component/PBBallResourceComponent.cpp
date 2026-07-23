#include "PBBallResourceComponent.h"

#include "PBBallComboComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"
#include "PinBallLike/Table/StatusEffect/PBStatusEffectAssetIds.h"

void UPBBallResourceComponent::TakeDamage(const int32 Damage)
{
	TakeDamageInternal(Damage, true);
}

void UPBBallResourceComponent::TakeSelfCollisionDamage(const int32 Damage)
{
	TakeDamageInternal(Damage, false);
}

void UPBBallResourceComponent::TakeDamageInternal(const int32 Damage, const bool bResetComboOnAppliedDamage)
{
	if (Damage <= 0)
	{
		return;
	}

	const AActor* OwnerActor = GetOwner();
	const UPBStatusEffectComponent* StatusEffectComponent = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UPBStatusEffectComponent>()
		: nullptr;
	if (StatusEffectComponent
		&& StatusEffectComponent->HasStatusEffect(PBStatusEffectAssetIds::StatusEffect::InvincibleSkill))
	{
		return;
	}

	float RemainingDamage = static_cast<float>(Damage);
	if (HasResource(PBResourceNames::Shield))
	{
		const float CurrentShield = GetResourceCurrent(PBResourceNames::Shield);
		const float AbsorbedDamage = FMath::Min(CurrentShield, RemainingDamage);
		if (AbsorbedDamage > 0.0f)
		{
			ApplyResourceDelta(PBResourceNames::Shield, -AbsorbedDamage);
			RemainingDamage -= AbsorbedDamage;
		}
	}

	if (RemainingDamage <= 0.0f)
	{
		return;
	}

	if (ConsumeDamageIgnoreCount(PBResourceNames::Health))
	{
		return;
	}

	const float PreviousHealth = GetResourceCurrent(PBResourceNames::Health);
	ApplyResourceDelta(PBResourceNames::Health, -RemainingDamage);
	const float CurrentHealth = GetResourceCurrent(PBResourceNames::Health);
	const int32 AppliedDamage = FMath::RoundToInt(PreviousHealth - CurrentHealth);
	if (AppliedDamage > 0 && bResetComboOnAppliedDamage)
	{
		ResetComboAfterAppliedDamage();
	}

	if (AppliedDamage > 0 && UGameplayMessageSubsystem::HasInstance(this))
	{
		FPBDamageLogMessage Message;
		Message.Style = EPBDamageLogStyle::EnemyAttack;
		Message.DamageAmount = AppliedDamage;
		Message.HitLocation = IsValid(OwnerActor)
			? OwnerActor->GetActorLocation() + FVector(0.0f, -100.0f, 0.0f)
			: FVector::ZeroVector;
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			GameplayTags::Event_UI_DamageLog_Requested,
			Message);
	}

	if (TryReviveOnZero(PBResourceNames::Health))
	{
		return;
	}

	TryApplyPostDamageHeal(PBResourceNames::Health, PreviousHealth);
}

void UPBBallResourceComponent::ResetComboAfterAppliedDamage() const
{
	const AActor* OwnerActor = GetOwner();
	UPBBallComboComponent* ComboComponent = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UPBBallComboComponent>()
		: nullptr;
	if (!ComboComponent)
	{
		return;
	}

	ComboComponent->ResetCombo();
}

void UPBBallResourceComponent::AddDamageIgnoreCount(const FName ResourceName, const int32 Count)
{
	AddResourceDamageRule(EPBBallResourceDamageRuleKind::DamageIgnore, ResourceName, Count, 0.0f);
}

int32 UPBBallResourceComponent::GetDamageIgnoreCount(const FName ResourceName) const
{
	const FPBBallResourceDamageRule* Rule =
		FindResourceDamageRule(EPBBallResourceDamageRuleKind::DamageIgnore, ResourceName);
	return Rule ? FMath::Max(0, Rule->Count) : 0;
}

void UPBBallResourceComponent::AddReviveOnZeroCount(
	const FName ResourceName,
	const int32 Count,
	const float ReviveValue)
{
	AddResourceDamageRule(EPBBallResourceDamageRuleKind::ReviveOnZero, ResourceName, Count, ReviveValue);
}

void UPBBallResourceComponent::AddPostDamageHealCount(
	const FName ResourceName,
	const int32 Count,
	const float HealValue)
{
	AddResourceDamageRule(EPBBallResourceDamageRuleKind::PostDamageHeal, ResourceName, Count, HealValue);
}

void UPBBallResourceComponent::AddResourceDamageRule(
	const EPBBallResourceDamageRuleKind Kind,
	const FName ResourceName,
	const int32 Count,
	const float Value)
{
	if (ResourceName.IsNone() || Count <= 0)
	{
		return;
	}
	if (Kind != EPBBallResourceDamageRuleKind::DamageIgnore && Value <= 0.0f)
	{
		return;
	}

	FPBBallResourceDamageRule* Rule = FindResourceDamageRule(Kind, ResourceName);
	if (!Rule)
	{
		Rule = &DamageRules.AddDefaulted_GetRef();
		Rule->Kind = Kind;
		Rule->ResourceName = ResourceName;
	}

	Rule->Count = FMath::Max(0, Rule->Count) + Count;
	Rule->Value = Value;
}

FPBBallResourceDamageRule* UPBBallResourceComponent::FindResourceDamageRule(
	const EPBBallResourceDamageRuleKind Kind,
	const FName ResourceName)
{
	return DamageRules.FindByPredicate(
		[Kind, ResourceName](const FPBBallResourceDamageRule& Rule)
		{
			return Rule.Kind == Kind && Rule.ResourceName == ResourceName;
		});
}

const FPBBallResourceDamageRule* UPBBallResourceComponent::FindResourceDamageRule(
	const EPBBallResourceDamageRuleKind Kind,
	const FName ResourceName) const
{
	return DamageRules.FindByPredicate(
		[Kind, ResourceName](const FPBBallResourceDamageRule& Rule)
		{
			return Rule.Kind == Kind && Rule.ResourceName == ResourceName;
		});
}

bool UPBBallResourceComponent::ConsumeResourceDamageRule(
	const EPBBallResourceDamageRuleKind Kind,
	const FName ResourceName,
	float& OutValue)
{
	for (int32 Index = 0; Index < DamageRules.Num(); ++Index)
	{
		FPBBallResourceDamageRule& Rule = DamageRules[Index];
		if (Rule.Kind != Kind || Rule.ResourceName != ResourceName || Rule.Count <= 0)
		{
			continue;
		}

		OutValue = Rule.Value;
		--Rule.Count;
		if (Rule.Count <= 0)
		{
			DamageRules.RemoveAtSwap(Index);
		}

		return true;
	}

	return false;
}

bool UPBBallResourceComponent::ConsumeDamageIgnoreCount(const FName ResourceName)
{
	float UnusedValue = 0.0f;
	return ConsumeResourceDamageRule(EPBBallResourceDamageRuleKind::DamageIgnore, ResourceName, UnusedValue);
}

bool UPBBallResourceComponent::TryReviveOnZero(const FName ResourceName)
{
	if (GetResourceCurrent(ResourceName) > 0.0f)
	{
		return false;
	}

	float ReviveValue = 1.0f;
	if (!ConsumeResourceDamageRule(EPBBallResourceDamageRuleKind::ReviveOnZero, ResourceName, ReviveValue))
	{
		return false;
	}

	const float ClampedReviveValue = FMath::Clamp(ReviveValue, 0.0f, GetResourceMax(ResourceName));
	if (ClampedReviveValue <= 0.0f)
	{
		return false;
	}

	SetResourceCurrent(ResourceName, ClampedReviveValue);
	OnResourceRevived.Broadcast(ResourceName, GetResourceCurrent(ResourceName));
	return true;
}

void UPBBallResourceComponent::TryApplyPostDamageHeal(
	const FName ResourceName,
	const float PreviousCurrent)
{
	const float Current = GetResourceCurrent(ResourceName);
	if (Current <= 0.0f || Current >= PreviousCurrent)
	{
		return;
	}

	float Value = 1.0f;
	if (!ConsumeResourceDamageRule(EPBBallResourceDamageRuleKind::PostDamageHeal, ResourceName, Value))
	{
		return;
	}

	if (Value > 0.0f)
	{
		ApplyResourceDelta(ResourceName, Value);
	}
}
