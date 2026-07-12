// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPartyDeathComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

UPBPartyDeathComponent::UPBPartyDeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBPartyDeathComponent::InitializeDependencies(FPBPartyDeathDependencies InDependencies)
{
	Dependencies = MoveTemp(InDependencies);
}

void UPBPartyDeathComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindPartyBalls();
	Super::EndPlay(EndPlayReason);
}

void UPBPartyDeathComponent::BindPartyBalls()
{
	UnbindPartyBalls();

	ObservedPartyBalls = GetPartyBalls();
	for (const TObjectPtr<APBBallBase>& Ball : ObservedPartyBalls)
	{
		if (!IsValid(Ball.Get()))
		{
			continue;
		}

		if (UPBBaseResourceComponent* ResourceComponent = Ball->GetResourceComponent())
		{
			ResourceComponent->OnResourceCurrentChanged.AddUObject(
				this,
				&UPBPartyDeathComponent::HandlePartyBallResourceCurrentChanged);
		}
	}
}

void UPBPartyDeathComponent::UnbindPartyBalls()
{
	const TArray<TObjectPtr<APBBallBase>> BoundPartyBalls = ObservedPartyBalls;
	for (const TObjectPtr<APBBallBase>& Ball : BoundPartyBalls)
	{
		UnbindPartyBall(Ball.Get());
	}
	ObservedPartyBalls.Reset();
}

void UPBPartyDeathComponent::UnbindPartyBall(APBBallBase* Ball)
{
	if (!IsValid(Ball))
	{
		return;
	}

	if (UPBBaseResourceComponent* ResourceComponent = Ball->GetResourceComponent())
	{
		ResourceComponent->OnResourceCurrentChanged.RemoveAll(this);
	}

	ObservedPartyBalls.RemoveAll([Ball](const TObjectPtr<APBBallBase>& ObservedBall)
	{
		return ObservedBall.Get() == Ball;
	});
}

void UPBPartyDeathComponent::ResetPartyDeathState()
{
	bAllBallsDeadBroadcasted = false;
}

void UPBPartyDeathComponent::DetectDeadPartyBalls()
{
	TArray<APBBallBase*> DeadBalls;
	for (const TObjectPtr<APBBallBase>& Ball : ObservedPartyBalls)
	{
		if (!IsValid(Ball.Get()))
		{
			continue;
		}

		const UPBBaseResourceComponent* ResourceComponent = Ball->GetResourceComponent();
		if (ResourceComponent && ResourceComponent->IsDead())
		{
			DeadBalls.Add(Ball.Get());
		}
	}

	for (APBBallBase* DeadBall : DeadBalls)
	{
		HandleDeadPartyBall(DeadBall);
	}
}

const TArray<TObjectPtr<APBBallBase>>& UPBPartyDeathComponent::GetPartyBalls() const
{
	return Dependencies.GetPartyBalls ? Dependencies.GetPartyBalls() : EmptyPartyBalls;
}

void UPBPartyDeathComponent::HandleDeadPartyBall(APBBallBase* DeadBall)
{
	if (!IsValid(DeadBall)
		|| bAllBallsDeadBroadcasted
		|| (Dependencies.IsLauncherActive && Dependencies.IsLauncherActive())
		|| !Dependencies.ContainsPartyBall
		|| !Dependencies.ContainsPartyBall(DeadBall))
	{
		return;
	}

	const APBBallBase* CurrentLeaderBall = Dependencies.GetLeaderBall ? Dependencies.GetLeaderBall() : nullptr;
	const bool bDeadLeader = DeadBall == CurrentLeaderBall;
	const FVector DeathLocation = DeadBall->GetActorLocation();
	FVector InheritedVelocity = FVector::ZeroVector;
	if (bDeadLeader)
	{
		if (const IMovable* DeadLeaderMovable = PBInterfaceUtils::FindInterface<IMovable>(DeadBall))
		{
			InheritedVelocity = DeadLeaderMovable->GetVelocity();
			InheritedVelocity.Z = 0.0f;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[PartyDeath] Party ball died. Party=%s Ball=%s bLeader=%s Location=%s InheritedVelocity=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(DeadBall),
		bDeadLeader ? TEXT("true") : TEXT("false"),
		*DeathLocation.ToString(),
		*InheritedVelocity.ToString());

	if (Dependencies.SpawnBallDeathEffect)
	{
		Dependencies.SpawnBallDeathEffect(DeadBall);
	}

	UnbindPartyBall(DeadBall);
	if (Dependencies.RemovePartyBall)
	{
		Dependencies.RemovePartyBall(DeadBall);
	}

	if (bDeadLeader)
	{
		APBBallBase* NewLeaderBall = Dependencies.GetLeaderBall ? Dependencies.GetLeaderBall() : nullptr;
		if (IsValid(NewLeaderBall) && Dependencies.StartLeaderPromotion)
		{
			Dependencies.StartLeaderPromotion(NewLeaderBall, DeathLocation, InheritedVelocity);
		}
	}
	else if (Dependencies.RefreshPartyOrder)
	{
		Dependencies.RefreshPartyOrder();
	}

	if (Dependencies.IsPartyEmpty && Dependencies.IsPartyEmpty())
	{
		if (Dependencies.StopLeaderPromotion)
		{
			Dependencies.StopLeaderPromotion();
		}
		bAllBallsDeadBroadcasted = true;
		UnbindPartyBalls();
		if (Dependencies.BroadcastPartyAllBallsDead)
		{
			Dependencies.BroadcastPartyAllBallsDead();
		}
	}
}

void UPBPartyDeathComponent::HandlePartyBallResourceCurrentChanged(
	const FName ResourceName,
	float CurrentValue)
{
	(void)CurrentValue;

	if (ResourceName != PBResourceNames::Health)
	{
		return;
	}

	DetectDeadPartyBalls();
}
