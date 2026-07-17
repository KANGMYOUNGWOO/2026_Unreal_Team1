#include "PBBumperReactiveRepairComponent.h"

#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "TimerManager.h"

namespace
{
	constexpr int32 MaxReactiveRepairTriggers = 100;
	constexpr float MaxReactiveRepairAmount = 100000.0f;
	constexpr float MaxReactiveRepairDuration = 60.0f;
}

UPBBumperReactiveRepairComponent::UPBBumperReactiveRepairComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBumperReactiveRepairComponent::Arm(
	const FName ResourceName,
	const int32 TriggerCount,
	const float HealAmount,
	const float Duration)
{
	UPBBaseResourceComponent* ResourceComponent = ResolveResourceComponent();
	UWorld* World = GetWorld();
	if (!IsValid(ResourceComponent)
		|| !IsValid(World)
		|| ResourceName.IsNone()
		|| !ResourceComponent->HasResource(ResourceName)
		|| TriggerCount <= 0
		|| !FMath::IsFinite(HealAmount)
		|| HealAmount <= 0.0f
		|| !FMath::IsFinite(Duration)
		|| Duration <= 0.0f)
	{
		return false;
	}

	if (!bDelegatesBound)
	{
		ResourceComponent->OnResourceChanged.AddUObject(
			this,
			&UPBBumperReactiveRepairComponent::HandleResourceStructureChanged);
		ResourceComponent->OnResourceCurrentChanged.AddUObject(
			this,
			&UPBBumperReactiveRepairComponent::HandleResourceCurrentChanged);
		bDelegatesBound = true;
	}

	World->GetTimerManager().ClearTimer(ExpirationTimer);
	WatchedResourceName = ResourceName;
	RemainingTriggers = FMath::Clamp(TriggerCount, 1, MaxReactiveRepairTriggers);
	HealPerTrigger = FMath::Clamp(HealAmount, KINDA_SMALL_NUMBER, MaxReactiveRepairAmount);
	LastCurrentValue = ResourceComponent->GetResourceCurrent(ResourceName);
	SuppressedDecreaseCount = 0;
	bArmed = true;
	World->GetTimerManager().SetTimer(
		ExpirationTimer,
		this,
		&UPBBumperReactiveRepairComponent::Disarm,
		FMath::Clamp(Duration, 0.1f, MaxReactiveRepairDuration),
		false);
	return true;
}

void UPBBumperReactiveRepairComponent::SuppressNextIntentionalDecrease(
	const FName ResourceName)
{
	if (bArmed && ResourceName == WatchedResourceName)
	{
		++SuppressedDecreaseCount;
	}
}

void UPBBumperReactiveRepairComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Disarm();
	if (UPBBaseResourceComponent* ResourceComponent = ResolveResourceComponent())
	{
		ResourceComponent->OnResourceChanged.RemoveAll(this);
		ResourceComponent->OnResourceCurrentChanged.RemoveAll(this);
	}
	bDelegatesBound = false;
	Super::EndPlay(EndPlayReason);
}

UPBBaseResourceComponent* UPBBumperReactiveRepairComponent::ResolveResourceComponent()
{
	if (CachedResourceComponent.IsValid())
	{
		return CachedResourceComponent.Get();
	}

	AActor* OwnerActor = GetOwner();
	UPBBaseResourceComponent* ResourceComponent = IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UPBBaseResourceComponent>()
		: nullptr;
	CachedResourceComponent = ResourceComponent;
	return ResourceComponent;
}

void UPBBumperReactiveRepairComponent::Disarm()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpirationTimer);
	}
	bArmed = false;
	RemainingTriggers = 0;
	HealPerTrigger = 0.0f;
	SuppressedDecreaseCount = 0;
}

void UPBBumperReactiveRepairComponent::HandleResourceStructureChanged(
	const FName ChangedResourceName)
{
	if (ChangedResourceName != WatchedResourceName)
	{
		return;
	}

	if (const UPBBaseResourceComponent* ResourceComponent = ResolveResourceComponent())
	{
		LastCurrentValue = ResourceComponent->GetResourceCurrent(WatchedResourceName);
	}
}

void UPBBumperReactiveRepairComponent::HandleResourceCurrentChanged(
	const FName ChangedResourceName,
	const float NewValue)
{
	if (ChangedResourceName != WatchedResourceName)
	{
		return;
	}

	const float PreviousValue = LastCurrentValue;
	LastCurrentValue = NewValue;
	const bool bResourceDecreased = NewValue < PreviousValue - KINDA_SMALL_NUMBER;
	if (bArmed && bResourceDecreased && SuppressedDecreaseCount > 0)
	{
		--SuppressedDecreaseCount;
		UE_LOG(LogTemp, Verbose,
			TEXT("[Bumper] Reactive repair ignored an intentional resource cost. Target=%s RemainingSuppressions=%d"),
			*GetNameSafe(GetOwner()),
			SuppressedDecreaseCount);
		return;
	}

	if (!bArmed
		|| bApplyingHeal
		|| RemainingTriggers <= 0
		|| NewValue <= 0.0f
		|| !bResourceDecreased)
	{
		return;
	}

	UPBBaseResourceComponent* ResourceComponent = ResolveResourceComponent();
	if (!IsValid(ResourceComponent) || ResourceComponent->IsDead())
	{
		return;
	}

	--RemainingTriggers;
	bApplyingHeal = true;
	ResourceComponent->ApplyResourceDelta(WatchedResourceName, HealPerTrigger);
	bApplyingHeal = false;
	LastCurrentValue = ResourceComponent->GetResourceCurrent(WatchedResourceName);

	UE_LOG(LogTemp, Log,
		TEXT("[Bumper] Reactive repair applied. Target=%s Heal=%.2f Remaining=%d"),
		*GetNameSafe(GetOwner()),
		HealPerTrigger,
		RemainingTriggers);

	if (RemainingTriggers <= 0)
	{
		Disarm();
	}
}
