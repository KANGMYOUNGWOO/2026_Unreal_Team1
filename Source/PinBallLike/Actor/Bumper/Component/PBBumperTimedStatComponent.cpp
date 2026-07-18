#include "PBBumperTimedStatComponent.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "TimerManager.h"

namespace
{
	constexpr float MaxTimedAttackPercent = 500.0f;
	constexpr float MaxTimedAttackDuration = 60.0f;
}

UPBBumperTimedStatComponent::UPBBumperTimedStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBumperTimedStatComponent::ApplyTimedAttackPercent(
	FName SourceId,
	const float Percent,
	const float Duration,
	int32& OutSourceBonus)
{
	return ApplyTimedStatPercent(
		SourceId,
		PBStatNames::Attack,
		Percent,
		Duration,
		OutSourceBonus);
}

bool UPBBumperTimedStatComponent::ApplyTimedStatPercent(
	FName SourceId,
	const FName StatName,
	const float Percent,
	const float Duration,
	int32& OutSourceBonus)
{
	OutSourceBonus = 0;
	UPBBaseStatComponent* StatComponent = ResolveStatComponent();
	UWorld* World = GetWorld();
	if (!IsValid(StatComponent)
		|| !IsValid(World)
		|| SourceId.IsNone()
		|| !FMath::IsFinite(Percent)
		|| !FMath::IsFinite(Duration)
		|| Percent <= 0.0f
		|| Duration <= 0.0f
		|| StatName.IsNone()
		|| !StatComponent->HasStat(StatName))
	{
		return false;
	}

	FActiveStatPercent& ActiveBonus = ActiveBonuses.FindOrAdd(SourceId);
	if (!ActiveBonus.StatName.IsNone() && ActiveBonus.StatName != StatName)
	{
		return false;
	}

	World->GetTimerManager().ClearTimer(ActiveBonus.ExpirationTimer);
	ActiveBonus.StatName = StatName;
	ActiveBonus.Percent = FMath::Clamp(Percent, 0.0f, MaxTimedAttackPercent);

	RecalculateAppliedBonus(StatName);
	const int32 AppliedBonus = AppliedBonusesByStat.FindRef(StatName);
	const int32 BaseStatValue = FMath::Max(
		StatComponent->GetStat(StatName) - AppliedBonus,
		0);
	OutSourceBonus = FMath::Max(
		FMath::RoundToInt(BaseStatValue * (ActiveBonus.Percent / 100.0f)),
		0);

	FTimerDelegate ExpirationDelegate;
	ExpirationDelegate.BindUObject(
		this,
		&UPBBumperTimedStatComponent::ExpireSource,
		SourceId);
	World->GetTimerManager().SetTimer(
		ActiveBonus.ExpirationTimer,
		ExpirationDelegate,
		FMath::Clamp(Duration, 0.1f, MaxTimedAttackDuration),
		false);

	return true;
}

void UPBBumperTimedStatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllBonuses();
	Super::EndPlay(EndPlayReason);
}

UPBBaseStatComponent* UPBBumperTimedStatComponent::ResolveStatComponent()
{
	if (CachedStatComponent.IsValid())
	{
		return CachedStatComponent.Get();
	}

	AActor* OwnerActor = GetOwner();
	UPBBaseStatComponent* StatComponent = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UPBBaseStatComponent>()
		: nullptr;
	CachedStatComponent = StatComponent;
	return StatComponent;
}

void UPBBumperTimedStatComponent::RecalculateAppliedBonus(const FName StatName)
{
	UPBBaseStatComponent* StatComponent = ResolveStatComponent();
	if (!IsValid(StatComponent) || StatName.IsNone())
	{
		AppliedBonusesByStat.Remove(StatName);
		return;
	}

	const int32 PreviousBonus = AppliedBonusesByStat.FindRef(StatName);
	const int32 BaseStatValue = FMath::Max(
		StatComponent->GetStat(StatName) - PreviousBonus,
		0);
	float TotalPercent = 0.0f;
	for (const TPair<FName, FActiveStatPercent>& BonusPair : ActiveBonuses)
	{
		if (BonusPair.Value.StatName == StatName)
		{
			TotalPercent += BonusPair.Value.Percent;
		}
	}

	const int32 NewBonus = FMath::Max(
		FMath::RoundToInt(BaseStatValue * (TotalPercent / 100.0f)),
		0);
	StatComponent->ApplyStat(
		StatName,
		NewBonus - PreviousBonus);
	if (NewBonus > 0)
	{
		AppliedBonusesByStat.Add(StatName, NewBonus);
	}
	else
	{
		AppliedBonusesByStat.Remove(StatName);
	}
}

void UPBBumperTimedStatComponent::ExpireSource(const FName SourceId)
{
	FName ExpiredStatName = NAME_None;
	if (FActiveStatPercent* ActiveBonus = ActiveBonuses.Find(SourceId))
	{
		ExpiredStatName = ActiveBonus->StatName;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ActiveBonus->ExpirationTimer);
		}
	}

	if (ActiveBonuses.Remove(SourceId) > 0)
	{
		RecalculateAppliedBonus(ExpiredStatName);
	}
}

void UPBBumperTimedStatComponent::ClearAllBonuses()
{
	if (UWorld* World = GetWorld())
	{
		for (TPair<FName, FActiveStatPercent>& BonusPair : ActiveBonuses)
		{
			World->GetTimerManager().ClearTimer(BonusPair.Value.ExpirationTimer);
		}
	}

	ActiveBonuses.Reset();
	if (UPBBaseStatComponent* StatComponent = ResolveStatComponent())
	{
		for (const TPair<FName, int32>& AppliedBonusPair : AppliedBonusesByStat)
		{
			StatComponent->ApplyStat(AppliedBonusPair.Key, -AppliedBonusPair.Value);
		}
	}
	AppliedBonusesByStat.Reset();
}
