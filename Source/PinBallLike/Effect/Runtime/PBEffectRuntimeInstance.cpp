#include "PinBallLike/Effect/Runtime/PBEffectRuntimeInstance.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "PinBallLike/Actor/Ball/Component/PBBallResourceComponent.h"
#include "PinBallLike/Actor/Ball/Component/PBBallEffectRuntimeComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"

void UPBEffectRuntimeInstanceBase::Initialize(UPBBallEffectRuntimeComponent* InOwnerComponent)
{
	OwnerComponent = InOwnerComponent;
}

AActor* UPBEffectRuntimeInstanceBase::GetOwnerActor() const
{
	return OwnerComponent ? OwnerComponent->GetOwner() : nullptr;
}

UWorld* UPBEffectRuntimeInstanceBase::GetWorld() const
{
	if (const AActor* OwnerActor = GetOwnerActor())
	{
		return OwnerActor->GetWorld();
	}

	return nullptr;
}

UPBBaseStatComponent* UPBEffectRuntimeInstanceBase::GetOwnerStatComponent() const
{
	return GetOwnerActor() ? GetOwnerActor()->FindComponentByClass<UPBBaseStatComponent>() : nullptr;
}

UPBBaseResourceComponent* UPBEffectRuntimeInstanceBase::GetOwnerResourceComponent() const
{
	return GetOwnerActor() ? GetOwnerActor()->FindComponentByClass<UPBBaseResourceComponent>() : nullptr;
}

UPBBallResourceComponent* UPBEffectRuntimeInstanceBase::GetOwnerBallResourceComponent() const
{
	return GetOwnerActor() ? GetOwnerActor()->FindComponentByClass<UPBBallResourceComponent>() : nullptr;
}

void UPBEffectRuntimeInstanceBase::ApplyStatDelta(
	const FName StatName,
	const FName ModifyType,
	const float Value) const
{
	UPBBaseStatComponent* StatComponent = GetOwnerStatComponent();
	if (!StatComponent || StatName.IsNone() || ModifyType.IsNone())
	{
		return;
	}

	const int32 CurrentValue = StatComponent->GetStat(StatName);
	StatComponent->SetStat(StatName, CalculateModifiedIntValue(CurrentValue, ModifyType, Value));
}

void UPBEffectRuntimeInstanceBase::ApplyTemporaryStatBuff(
	const FName StatName,
	const FName ModifyType,
	const float Value,
	const float Duration)
{
	UPBBaseStatComponent* StatComponent = GetOwnerStatComponent();
	if (!StatComponent || StatName.IsNone() || ModifyType.IsNone())
	{
		return;
	}

	const int32 PreviousValue = StatComponent->GetStat(StatName);
	StatComponent->SetStat(StatName, CalculateModifiedIntValue(PreviousValue, ModifyType, Value));

	UWorld* World = GetWorld();
	if (!World || Duration <= 0.0f)
	{
		return;
	}

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateUObject(
			this,
			&UPBEffectRuntimeInstanceBase::RestoreStatValue,
			StatName,
			PreviousValue),
		Duration,
		false);
}

void UPBEffectRuntimeInstanceBase::RestoreStatValue(const FName StatName, const int32 PreviousValue)
{
	if (UPBBaseStatComponent* StatComponent = GetOwnerStatComponent())
	{
		StatComponent->SetStat(StatName, PreviousValue);
	}
}

AActor* UPBEffectRuntimeInstanceBase::FindNearestEnemy(AActor* SourceActor) const
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(SourceActor))
	{
		return nullptr;
	}

	AActor* NearestEnemy = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();
	const FVector SourceLocation = SourceActor->GetActorLocation();
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (!IsValid(Candidate) || !Candidate->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(SourceLocation, Candidate->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestEnemy = Candidate;
		}
	}

	return NearestEnemy;
}

void UPBEffectRuntimeInstanceBase::ApplyDamageToEnemy(AActor* EnemyActor, const int32 DamageAmount) const
{
	if (IsValid(EnemyActor)
		&& DamageAmount > 0
		&& EnemyActor->GetClass()->ImplementsInterface(UBossInterface::StaticClass()))
	{
		IBossInterface::Execute_DamageToBoss(EnemyActor, DamageAmount);
	}
}

int32 UPBEffectRuntimeInstanceBase::CalculateModifiedIntValue(
	const int32 CurrentValue,
	const FName ModifyType,
	const float Value)
{
	if (ModifyType == PBEffectTypes::ModifyType::Set)
	{
		return FMath::RoundToInt(Value);
	}

	if (ModifyType == PBEffectTypes::ModifyType::PercentAdd)
	{
		return FMath::RoundToInt(static_cast<float>(CurrentValue) * (1.0f + Value * 0.01f));
	}

	return CurrentValue + FMath::RoundToInt(Value);
}

void UPBFirstAttackEffectRuntimeInstance::Setup(const float InAttackPercent, const int32 InExtraDamage)
{
	AttackPercent = InAttackPercent;
	ExtraDamage = FMath::Max(0, InExtraDamage);
}

int32 UPBFirstAttackEffectRuntimeInstance::ModifyCollisionDamage(const int32 CurrentDamage)
{
	if (bAttackConsumed)
	{
		return CurrentDamage;
	}

	bAttackConsumed = true;
	const float ModifiedDamage = static_cast<float>(CurrentDamage) * (1.0f + AttackPercent * 0.01f);
	return FMath::Max(0, FMath::RoundToInt(ModifiedDamage));
}

void UPBFirstAttackEffectRuntimeInstance::HandleEnemyHit(AActor* EnemyActor)
{
	if (bExtraDamageConsumed || ExtraDamage <= 0)
	{
		return;
	}

	bExtraDamageConsumed = true;
	ApplyDamageToEnemy(EnemyActor, ExtraDamage);
}

void UPBComboStatBuffEffectRuntimeInstance::Setup(
	const FName InStatName,
	const FName InModifyType,
	const float InValue,
	const int32 InRequiredCombo)
{
	StatName = InStatName;
	ModifyType = InModifyType;
	Value = InValue;
	RequiredCombo = InRequiredCombo;
}

void UPBComboStatBuffEffectRuntimeInstance::HandleComboChanged(const int32 CurrentCombo)
{
	if (!bApplied && RequiredCombo > 0 && CurrentCombo >= RequiredCombo)
	{
		ApplyStatDelta(StatName, ModifyType, Value);
		bApplied = true;
	}
}


void UPBComboPeriodStatBuffEffectRuntimeInstance::Setup(FName InStatName, FName InModifyType, float InValue,
	int32 InRequiredCombo)
{
	StatName = InStatName;
	ModifyType = InModifyType;
	Value = InValue;
	RequiredCombo = InRequiredCombo;
}

void UPBComboPeriodStatBuffEffectRuntimeInstance::HandleComboChanged(int32 CurrentCombo)
{
	if (RequiredCombo <= 0)
	{
		return;
	}

	const int32 CurrentStep = CurrentCombo / RequiredCombo;

	if (CurrentStep <= LastAppliedStep)
	{
		return;
	}

	const int32 NewStepCount = CurrentStep - LastAppliedStep;

	for (int32 Index = 0; Index < NewStepCount; ++Index)
	{
		ApplyStatDelta(StatName, ModifyType, Value);
	}

	LastAppliedStep = CurrentStep;
}

void UPBSkillDamageEffectRuntimeInstance::Setup(const float InPercent)
{
	Percent = InPercent;
}

int32 UPBSkillDamageEffectRuntimeInstance::ModifySkillDamage(const int32 CurrentDamage) const
{
	const float ModifiedDamage = static_cast<float>(CurrentDamage) * (1.0f + Percent * 0.01f);
	return FMath::Max(0, FMath::RoundToInt(ModifiedDamage));
}

void UPBOnHitStatStackEffectRuntimeInstance::Setup(
	const FName InStatName,
	const FName InModifyType,
	const float InValue)
{
	StatName = InStatName;
	ModifyType = InModifyType;
	Value = InValue;
}

void UPBOnHitStatStackEffectRuntimeInstance::HandleEnemyHit(AActor*)
{
	ApplyStatDelta(StatName, ModifyType, Value);
}

void UPBOnHitDamageIgnoreEffectRuntimeInstance::Setup(
	const FName InResourceName,
	const int32 InIgnoreCount,
	const float InChancePercent)
{
	ResourceName = InResourceName.IsNone() ? PBResourceNames::Health : InResourceName;
	IgnoreCount = FMath::Max(1, InIgnoreCount);
	ChancePercent = FMath::Clamp(InChancePercent, 0.0f, 100.0f);
}

void UPBOnHitDamageIgnoreEffectRuntimeInstance::HandleEnemyHit(AActor*)
{
	if (bRolled || IgnoreCount <= 0)
	{
		return;
	}

	bRolled = true;
	if (FMath::FRandRange(0.0f, 100.0f) > ChancePercent)
	{
		return;
	}

	if (UPBBallResourceComponent* ResourceComponent = GetOwnerBallResourceComponent())
	{
		ResourceComponent->AddDamageIgnoreCount(ResourceName, IgnoreCount);
	}
}

void UPBComboExtraDamageEffectRuntimeInstance::Setup(const int32 InComboInterval, const int32 InDamageAmount)
{
	ComboInterval = FMath::Max(0, InComboInterval);
	DamageAmount = FMath::Max(0, InDamageAmount);
}

void UPBComboExtraDamageEffectRuntimeInstance::HandleComboChanged(const int32 CurrentCombo)
{
	if (ComboInterval <= 0
		|| DamageAmount <= 0
		|| CurrentCombo <= 0
		|| CurrentCombo % ComboInterval != 0
		|| CurrentCombo == LastAppliedCombo)
	{
		return;
	}

	LastAppliedCombo = CurrentCombo;
	ApplyDamageToEnemy(FindNearestEnemy(GetOwnerActor()), DamageAmount);
}

void UPBAttackPercentEffectRuntimeInstance::Setup(const float InPercent)
{
	Percent = InPercent;
}

int32 UPBAttackPercentEffectRuntimeInstance::ModifyCollisionDamage(const int32 CurrentDamage)
{
	const float ModifiedDamage = static_cast<float>(CurrentDamage) * (1.0f + Percent * 0.01f);
	return FMath::Max(0, FMath::RoundToInt(ModifiedDamage));
}

void UPBReviveStatBuffEffectRuntimeInstance::Setup(
	const FName InStatName,
	const FName InModifyType,
	const float InValue)
{
	StatName = InStatName;
	ModifyType = InModifyType;
	Value = InValue;
}

void UPBReviveStatBuffEffectRuntimeInstance::HandleResourceRevived(
	const FName ResourceName,
	const float)
{
	if (ResourceName == PBResourceNames::Health)
	{
		ApplyStatDelta(StatName, ModifyType, Value);
	}
}

void UPBFirstHitTimedStatBuffEffectRuntimeInstance::Setup(
	const FName InStatName,
	const FName InModifyType,
	const float InValue,
	const float InDuration)
{
	StatName = InStatName;
	ModifyType = InModifyType;
	Value = InValue;
	Duration = InDuration;
}

void UPBFirstHitTimedStatBuffEffectRuntimeInstance::HandleEnemyHit(AActor*)
{
	if (bApplied)
	{
		return;
	}

	bApplied = true;
	ApplyTemporaryStatBuff(StatName, ModifyType, Value, Duration);
}

void UPBSwitchNextHitDamageEffectRuntimeInstance::Setup(const float InAttackPercent)
{
	AttackPercent = InAttackPercent;
}

int32 UPBSwitchNextHitDamageEffectRuntimeInstance::ModifyCollisionDamage(const int32 CurrentDamage)
{
	if (bConsumed)
	{
		return CurrentDamage;
	}

	bConsumed = true;
	const float ModifiedDamage = static_cast<float>(CurrentDamage) * (1.0f + AttackPercent * 0.01f);
	return FMath::Max(0, FMath::RoundToInt(ModifiedDamage));
}

void UPBSwitchTimedStatBuffEffectRuntimeInstance::Setup(
	const FName InStatName,
	const FName InModifyType,
	const float InValue,
	const float InDuration)
{
	StatName = InStatName;
	ModifyType = InModifyType;
	Value = InValue;
	Duration = InDuration;
}

void UPBSwitchTimedStatBuffEffectRuntimeInstance::ApplyNow()
{
	if (bApplied)
	{
		return;
	}

	bApplied = true;
	ApplyTemporaryStatBuff(StatName, ModifyType, Value, Duration);
}
