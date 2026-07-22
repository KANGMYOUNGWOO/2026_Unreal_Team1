#include "PBBallDashComponent.h"

#include "PBBallPhysicsComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Subsystem/PBSoundSubsystem.h"

UPBBallDashComponent::UPBBallDashComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBallDashComponent::InitializeDependencies(UPBBallPhysicsComponent* InPhysicsComponent)
{
	PhysicsComponent = InPhysicsComponent;
}

bool UPBBallDashComponent::DashInDirection(FVector Direction, const float Speed)
{
	if (!PhysicsComponent || Speed <= 0.0f)
	{
		return false;
	}

	Direction.Z = 0.0f;
	const FVector DashDirection = Direction.GetSafeNormal();
	if (DashDirection.IsNearlyZero())
	{
		return false;
	}

	PhysicsComponent->Launch(DashDirection, Speed);
	PlayDashSound();

	return true;
}

bool UPBBallDashComponent::DashToBoss(const float Speed)
{
	return DashInDirection(ResolveDashDirectionToActor(FindBossTarget()), Speed);
}

bool UPBBallDashComponent::DashToBossWithDefaultSpeed()
{
	return DashToBoss(DefaultDashSpeed);
}

AActor* UPBBallDashComponent::FindBossTarget() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<APBBossBase> It(World); It; ++It)
	{
		APBBossBase* Boss = *It;
		if (IsValid(Boss) && !Boss->IsDead())
		{
			return Boss;
		}
	}

	return nullptr;
}

FVector UPBBallDashComponent::ResolveDashDirectionToActor(const AActor* TargetActor) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !IsValid(TargetActor))
	{
		return FVector::ZeroVector;
	}

	FVector Direction = TargetActor->GetActorLocation() - OwnerActor->GetActorLocation();
	Direction.Z = 0.0f;
	return Direction;
}

void UPBBallDashComponent::PlayDashSound() const
{
	if (!DashSound)
	{
		return;
	}

	if (UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this))
	{
		SoundSubsystem->PlaySFX(DashSound, DashSoundVolume, 1.0f, DashSoundStartTime);
	}
}
