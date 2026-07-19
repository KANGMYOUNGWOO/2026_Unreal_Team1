#include "PBTimedAreaDamageComponent.h"

#include "PinBallLike/Interface/BossInterface.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

UPBTimedAreaDamageComponent::UPBTimedAreaDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBTimedAreaDamageComponent::InitializeDamageArea(
	UPrimitiveComponent* InDamageArea,
	AActor* InIgnoredActor)
{
	DeactivateEffect();
	UnbindDamageArea();

	DamageArea = InDamageArea;
	IgnoredActor = InIgnoredActor;

	if (DamageArea)
	{
		DamageArea->OnComponentBeginOverlap.AddUniqueDynamic(
			this,
			&UPBTimedAreaDamageComponent::HandleDamageAreaBeginOverlap);
		DamageArea->OnComponentEndOverlap.AddUniqueDynamic(
			this,
			&UPBTimedAreaDamageComponent::HandleDamageAreaEndOverlap);
	}
}

void UPBTimedAreaDamageComponent::ConfigureDamage(
	const int32 InDamageAmount,
	const float InDuration,
	const int32 InDamageCount)
{
	DamageAmount = FMath::Max(0, InDamageAmount);
	Duration = FMath::Max(0.0f, InDuration);
	DamageCount = FMath::Max(0, InDamageCount);
	DamageInterval = DamageCount > 0 ? Duration / static_cast<float>(DamageCount) : 0.0f;
}

bool UPBTimedAreaDamageComponent::ActivateEffect()
{
	if (bIsActive
		|| !GetWorld()
		|| !DamageArea
		|| DamageAmount <= 0
		|| Duration <= 0.0f
		|| DamageCount <= 0
		|| DamageInterval <= 0.0f)
	{
		return false;
	}

	bIsActive = true;
	DamageArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TArray<AActor*> InitialTargets;
	DamageArea->GetOverlappingActors(InitialTargets);
	for (AActor* Target : InitialTargets)
	{
		AddOverlappingTarget(Target);
	}

	StartTimers();
	return true;
}

void UPBTimedAreaDamageComponent::DeactivateEffect()
{
	StopTimers();

	if (DamageArea)
	{
		DamageArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	bIsActive = false;
	TargetDamageStates.Reset();
}

void UPBTimedAreaDamageComponent::UpdateOverlappingTargetDamage()
{
	const UWorld* World = GetWorld();
	if (!bIsActive || !World || !DamageArea)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	for (auto It = TargetDamageStates.CreateIterator(); It; ++It)
	{
		AActor* Target = It.Key().Get();
		FTargetDamageState& State = It.Value();

		if (!IsValidTarget(Target) || !DamageArea->IsOverlappingActor(Target))
		{
			It.RemoveCurrent();
			continue;
		}

		if (State.AppliedHitCount >= DamageCount || CurrentTime < State.NextDamageTime)
		{
			continue;
		}

		if (ApplyDamage(Target, DamageAmount))
		{
			++State.AppliedHitCount;
			State.NextDamageTime = CurrentTime + DamageInterval;
			OnAreaDamageApplied.Broadcast(Target, DamageAmount, CalculateHitLocation(Target));
		}
	}
}

void UPBTimedAreaDamageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateEffect();
	UnbindDamageArea();
	IgnoredActor.Reset();

	Super::EndPlay(EndPlayReason);
}

void UPBTimedAreaDamageComponent::HandleDamageAreaBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComponent);
	static_cast<void>(OtherBodyIndex);
	static_cast<void>(bFromSweep);
	static_cast<void>(SweepResult);

	AddOverlappingTarget(OtherActor);
}

void UPBTimedAreaDamageComponent::HandleDamageAreaEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComponent);
	static_cast<void>(OtherBodyIndex);

	if (!OtherActor || (DamageArea && DamageArea->IsOverlappingActor(OtherActor)))
	{
		return;
	}

	TargetDamageStates.Remove(TWeakObjectPtr<AActor>(OtherActor));
}

void UPBTimedAreaDamageComponent::AddOverlappingTarget(AActor* Target)
{
	const UWorld* World = GetWorld();
	const TWeakObjectPtr<AActor> TargetKey(Target);
	if (!World
		|| !bIsActive
		|| !IsValidTarget(Target)
		|| TargetDamageStates.Contains(TargetKey))
	{
		return;
	}

	if (!ApplyDamage(Target, DamageAmount))
	{
		return;
	}

	FTargetDamageState& State = TargetDamageStates.Add(TargetKey);
	State.AppliedHitCount = 1;
	State.NextDamageTime = World->GetTimeSeconds() + DamageInterval;
	OnAreaDamageApplied.Broadcast(Target, DamageAmount, CalculateHitLocation(Target));
}

FVector UPBTimedAreaDamageComponent::CalculateHitLocation(const AActor* Target) const
{
	if (!IsValid(Target))
	{
		return DamageArea ? DamageArea->GetComponentLocation() : FVector::ZeroVector;
	}

	FVector HitLocation = DamageArea
		? DamageArea->GetComponentLocation()
		: Target->GetActorLocation();
	if (DamageArea)
	{
		FVector ClosestPoint;
		if (DamageArea->GetClosestPointOnCollision(
			Target->GetActorLocation(),
			ClosestPoint) >= 0.0f)
		{
			HitLocation = ClosestPoint;
		}
	}

	return HitLocation;
}

bool UPBTimedAreaDamageComponent::IsValidTarget(AActor* Target) const
{
	return IsValid(Target)
		&& Target != GetOwner()
		&& Target != IgnoredActor.Get()
		&& Target->GetClass()->ImplementsInterface(UBossInterface::StaticClass())
		&& CanApplyDamage(Target, DamageAmount);
}

void UPBTimedAreaDamageComponent::StartTimers()
{
	StopTimers();

	const float SafeCheckInterval = FMath::Min(
		FMath::Max(DamageCheckInterval, 0.01f),
		DamageInterval);

	GetWorld()->GetTimerManager().SetTimer(
		DamageCheckTimerHandle,
		this,
		&UPBTimedAreaDamageComponent::UpdateOverlappingTargetDamage,
		SafeCheckInterval,
		true);

	GetWorld()->GetTimerManager().SetTimer(
		DurationTimerHandle,
		this,
		&UPBTimedAreaDamageComponent::FinishEffect,
		Duration,
		false);
}

void UPBTimedAreaDamageComponent::StopTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageCheckTimerHandle);
		World->GetTimerManager().ClearTimer(DurationTimerHandle);
	}
}

void UPBTimedAreaDamageComponent::FinishEffect()
{
	if (!bIsActive)
	{
		return;
	}

	DeactivateEffect();
	OnEffectFinished.Broadcast();
}

void UPBTimedAreaDamageComponent::UnbindDamageArea()
{
	if (!DamageArea)
	{
		return;
	}

	DamageArea->OnComponentBeginOverlap.RemoveAll(this);
	DamageArea->OnComponentEndOverlap.RemoveAll(this);
	DamageArea = nullptr;
}
