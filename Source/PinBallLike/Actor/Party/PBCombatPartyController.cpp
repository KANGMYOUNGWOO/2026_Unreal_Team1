// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCombatPartyController.h"

#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Component/PBLeaderPromotionComponent.h"
#include "Component/PBPartyBattleMessageComponent.h"
#include "Component/PBPartyDeathComponent.h"
#include "Component/PBPartyDeploymentComponent.h"
#include "Component/PBPartyLauncherComponent.h"
#include "Component/PBSnakeFormationComponent.h"
#include "Engine/GameInstance.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"

APBCombatPartyController::APBCombatPartyController()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* PartyRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PartyRoot"));
	SetRootComponent(PartyRootComponent);

	SnakeFormationComponent = CreateDefaultSubobject<UPBSnakeFormationComponent>(TEXT("SnakeFormationComponent"));
	PartyBattleMessageComponent = CreateDefaultSubobject<UPBPartyBattleMessageComponent>(TEXT("PartyBattleMessageComponent"));
	PartyDeathComponent = CreateDefaultSubobject<UPBPartyDeathComponent>(TEXT("PartyDeathComponent"));
	PartyDeploymentComponent = CreateDefaultSubobject<UPBPartyDeploymentComponent>(TEXT("PartyDeploymentComponent"));
	LeaderPromotionComponent = CreateDefaultSubobject<UPBLeaderPromotionComponent>(TEXT("LeaderPromotionComponent"));
	PartyLauncherComponent = CreateDefaultSubobject<UPBPartyLauncherComponent>(TEXT("PartyLauncherComponent"));

	FPBSnakeFormationDependencies SnakeFormationDependencies;
	SnakeFormationDependencies.GetValidPartyBalls = [this]()
	{
		return GetValidPartyBalls();
	};
	SnakeFormationDependencies.IsLauncherActive = [this]()
	{
		return IsLauncherActive();
	};
	SnakeFormationDependencies.ApplyPartyRoles = [this]()
	{
		ApplyPartyRoles();
	};
	SnakeFormationDependencies.HidePartyBallsForLaunchReady = [this]()
	{
		if (PartyLauncherComponent)
		{
			PartyLauncherComponent->HidePartyBallsForLaunchReady();
		}
	};
	SnakeFormationComponent->InitializeDependencies(MoveTemp(SnakeFormationDependencies));

	FPBLeaderPromotionDependencies LeaderPromotionDependencies;
	LeaderPromotionDependencies.RebuildPartyRoles = [this]()
	{
		RebuildPartyRoles();
	};
	LeaderPromotionDependencies.ClearPartyRoles = [this]()
	{
		ClearPartyRoles();
	};
	LeaderPromotionDependencies.ApplyPartyRoles = [this]()
	{
		ApplyPartyRoles();
	};
	LeaderPromotionDependencies.GetValidPartyBalls = [this]()
	{
		return GetValidPartyBalls();
	};
	LeaderPromotionComponent->InitializeDependencies(MoveTemp(LeaderPromotionDependencies), SnakeFormationComponent.Get());

	FPBPartyDeathDependencies DeathDependencies;
	DeathDependencies.GetPartyBalls = [this]() -> const TArray<TObjectPtr<APBBallBase>>&
	{
		return GetPartyBalls();
	};
	DeathDependencies.IsLauncherActive = [this]()
	{
		return IsLauncherActive();
	};
	DeathDependencies.ContainsPartyBall = [this](APBBallBase* Ball)
	{
		return ContainsPartyBall(Ball);
	};
	DeathDependencies.GetLeaderBall = [this]()
	{
		return GetLeaderBall();
	};
	DeathDependencies.RemovePartyBall = [this](APBBallBase* Ball)
	{
		RemovePartyBall(Ball);
	};
	DeathDependencies.IsPartyEmpty = [this]()
	{
		return IsPartyEmpty();
	};
	DeathDependencies.RefreshPartyOrder = [this]()
	{
		RefreshPartyOrder();
	};
	DeathDependencies.StartLeaderPromotion = [this](APBBallBase* NewLeaderBall, FVector TargetLocation, FVector InheritedVelocity)
	{
		if (LeaderPromotionComponent)
		{
			LeaderPromotionComponent->StartLeaderPromotion(NewLeaderBall, TargetLocation, InheritedVelocity);
		}
	};
	DeathDependencies.StopLeaderPromotion = [this]()
	{
		if (LeaderPromotionComponent)
		{
			LeaderPromotionComponent->StopPromotion();
		}
	};
	DeathDependencies.SpawnBallDeathEffect = [this](APBBallBase* DeadBall)
	{
		SpawnBallDeathEffect(DeadBall);
	};
	DeathDependencies.BroadcastPartyAllBallsDead = [this]()
	{
		BroadcastPartyAllBallsDead();
	};
	PartyDeathComponent->InitializeDependencies(MoveTemp(DeathDependencies));

	FPBPartyLauncherDependencies LauncherDependencies;
	LauncherDependencies.GetPartyBalls = [this]() -> const TArray<TObjectPtr<APBBallBase>>&
	{
		return GetPartyBalls();
	};
	LauncherDependencies.GetLeaderBall = [this]()
	{
		return GetLeaderBall();
	};
	LauncherDependencies.RefreshPartyOrder = [this]()
	{
		RefreshPartyOrder();
	};
	LauncherDependencies.ResetPartyDeathState = [this]()
	{
		if (PartyDeathComponent)
		{
			PartyDeathComponent->ResetPartyDeathState();
		}
	};
	LauncherDependencies.BindPartyDeathEvents = [this]()
	{
		if (PartyDeathComponent)
		{
			PartyDeathComponent->BindPartyBalls();
		}
	};
	LauncherDependencies.SetSnakeFormationActive = [this](const bool bActive)
	{
		if (SnakeFormationComponent)
		{
			SnakeFormationComponent->SetComponentTickEnabled(bActive);
		}
	};
	PartyLauncherComponent->InitializeDependencies(MoveTemp(LauncherDependencies));

	FPBPartyDeploymentDependencies DeploymentDependencies;
	DeploymentDependencies.UnbindPartyBalls = [this]()
	{
		if (PartyDeathComponent)
		{
			PartyDeathComponent->UnbindPartyBalls();
		}
	};
	DeploymentDependencies.ResetPartyDeathState = [this]()
	{
		if (PartyDeathComponent)
		{
			PartyDeathComponent->ResetPartyDeathState();
		}
	};
	DeploymentDependencies.StopLeaderPromotion = [this]()
	{
		if (LeaderPromotionComponent)
		{
			LeaderPromotionComponent->StopPromotion();
		}
	};
	DeploymentDependencies.SetLauncherActive = [this](const bool bActive)
	{
		if (PartyLauncherComponent)
		{
			PartyLauncherComponent->SetLauncherActive(bActive);
		}
	};
	DeploymentDependencies.ClearSnakeFormation = [this]()
	{
		if (SnakeFormationComponent)
		{
			SnakeFormationComponent->ClearFormation();
		}
	};
	DeploymentDependencies.ClearPartyRoles = [this]()
	{
		ClearPartyRoles();
	};
	DeploymentDependencies.DestroyPartyBalls = [this]()
	{
		DestroyPartyBalls();
	};
	DeploymentDependencies.SetPartyBalls = [this](const TArray<TObjectPtr<APBBallBase>>& InPartyBalls)
	{
		SetPartyBalls(InPartyBalls);
	};
	DeploymentDependencies.RefreshPartyOrder = [this]()
	{
		RefreshPartyOrder();
	};
	DeploymentDependencies.GetSpawnTransform = [this]()
	{
		return FTransform(GetActorRotation(), GetActorLocation());
	};
	DeploymentDependencies.GetSpawnOwner = [this]() -> AActor*
	{
		return this;
	};
	PartyDeploymentComponent->InitializeDependencies(MoveTemp(DeploymentDependencies));

	FPBPartyBattleMessageDependencies BattleMessageDependencies;
	BattleMessageDependencies.PrepareForDeployment = [this]()
	{
		if (PartyDeploymentComponent)
		{
			PartyDeploymentComponent->PrepareForDeployment();
		}
	};
	BattleMessageDependencies.LaunchPartyFromReadyPosition = [this]()
	{
		return PartyLauncherComponent && PartyLauncherComponent->LaunchPartyFromReadyPosition();
	};
	BattleMessageDependencies.RequestUseSkill = [this](const int32 SkillInputValue)
	{
		RequestUseSkill(SkillInputValue);
	};
	BattleMessageDependencies.GetPartyActor = [this]() -> AActor*
	{
		return this;
	};
	PartyBattleMessageComponent->InitializeDependencies(MoveTemp(BattleMessageDependencies));
}

void APBCombatPartyController::InitializeFromDeck()
{
	if (PartyDeploymentComponent)
	{
		PartyDeploymentComponent->RebuildPartyFromDeployment();
	}
}

void APBCombatPartyController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPartyRoles();
	DestroyPartyBalls();
	Super::EndPlay(EndPlayReason);
}

void APBCombatPartyController::SpawnBallDeathEffect(APBBallBase* DeadBall) const
{
	if (!BallDeathEffect || !IsValid(DeadBall))
	{
		return;
	}

	UGameplayStatics::SpawnEmitterAtLocation(
		this,
		BallDeathEffect,
		DeadBall->GetActorLocation(),
		DeadBall->GetActorRotation());
}

void APBCombatPartyController::BroadcastPartyAllBallsDead()
{
	if (PartyBattleMessageComponent)
	{
		PartyBattleMessageComponent->BroadcastPartyAllBallsDead(this);
	}
}

void APBCombatPartyController::RefreshPartyOrder()
{
	if (SnakeFormationComponent)
	{
		SnakeFormationComponent->RefreshPartyOrder();
	}
}

bool APBCombatPartyController::LaunchPartyFromReadyPosition()
{
	return PartyLauncherComponent && PartyLauncherComponent->LaunchPartyFromReadyPosition();
}

void APBCombatPartyController::PrepareForDeployment()
{
	if (PartyDeploymentComponent)
	{
		PartyDeploymentComponent->PrepareForDeployment();
	}
}

void APBCombatPartyController::SetLauncherActive(const bool bNewLauncherActive)
{
	if (PartyLauncherComponent)
	{
		PartyLauncherComponent->SetLauncherActive(bNewLauncherActive);
	}
}

bool APBCombatPartyController::IsLauncherActive() const
{
	return PartyLauncherComponent && PartyLauncherComponent->IsLauncherActive();
}

void APBCombatPartyController::SetPartyBalls(const TArray<TObjectPtr<APBBallBase>>& InPartyBalls)
{
	PartyBalls = InPartyBalls;
	RebuildPartyRoles();
}

void APBCombatPartyController::RemovePartyBall(APBBallBase* Ball)
{
	if (!IsValid(Ball))
	{
		return;
	}

	PartyBalls.RemoveAll([Ball](const TObjectPtr<APBBallBase>& PartyBall)
	{
		return PartyBall.Get() == Ball;
	});
	FollowerBalls.RemoveAll([Ball](const TObjectPtr<APBBallBase>& FollowerBall)
	{
		return FollowerBall.Get() == Ball;
	});
	if (Ball == LeaderBall.Get())
	{
		LeaderBall = nullptr;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBBallDeckSubsystem* DeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>())
		{
			const UPBBaseResourceComponent* ResourceComponent = Ball->GetResourceComponent();
			const float CurrentMana = ResourceComponent
				? ResourceComponent->GetResourceCurrent(PBResourceNames::Mana)
				: 0.0f;
			DeckSubsystem->SetOwnedBallSavedMana(Ball->GetBallInstanceId(), CurrentMana);
		}
	}

	Ball->SetCombatRole(EPBBallPartyRole::None);
	Ball->Destroy();
	RebuildPartyRoles();
}

void APBCombatPartyController::DestroyPartyBalls()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBBallDeckSubsystem* DeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>())
		{
			for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
			{
				if (IsValid(Ball.Get()))
				{
					const UPBBaseResourceComponent* ResourceComponent = Ball->GetResourceComponent();
					const float CurrentMana = ResourceComponent
						? ResourceComponent->GetResourceCurrent(PBResourceNames::Mana)
						: 0.0f;
					DeckSubsystem->SetOwnedBallSavedMana(Ball->GetBallInstanceId(), CurrentMana);
				}
			}
		}
	}

	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (IsValid(Ball.Get()))
		{
			Ball->Destroy();
		}
	}

	PartyBalls.Reset();
	LeaderBall = nullptr;
	FollowerBalls.Reset();
}

void APBCombatPartyController::RebuildPartyRoles()
{
	LeaderBall = PartyBalls.IsValidIndex(0) ? PartyBalls[0] : nullptr;
	FollowerBalls.Reset();
	for (int32 BallIndex = 1; BallIndex < PartyBalls.Num(); ++BallIndex)
	{
		if (IsValid(PartyBalls[BallIndex].Get()))
		{
			FollowerBalls.Add(PartyBalls[BallIndex]);
		}
	}
}

void APBCombatPartyController::ApplyPartyRoles()
{
	if (LeaderBall)
	{
		LeaderBall->SetCombatRole(EPBBallPartyRole::Leader);
	}

	for (const TObjectPtr<APBBallBase>& FollowerBall : FollowerBalls)
	{
		if (FollowerBall)
		{
			FollowerBall->SetCombatRole(EPBBallPartyRole::Follower);
		}
	}
}

void APBCombatPartyController::ClearPartyRoles()
{
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (Ball)
		{
			Ball->SetCombatRole(EPBBallPartyRole::None);
		}
	}
}

bool APBCombatPartyController::ContainsPartyBall(APBBallBase* Ball) const
{
	return IsValid(Ball) && PartyBalls.ContainsByPredicate([Ball](const TObjectPtr<APBBallBase>& PartyBall)
	{
		return PartyBall.Get() == Ball;
	});
}

TArray<APBBallBase*> APBCombatPartyController::GetValidPartyBalls() const
{
	TArray<APBBallBase*> ValidPartyBalls;
	ValidPartyBalls.Reserve(PartyBalls.Num());
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (IsValid(Ball.Get()))
		{
			ValidPartyBalls.Add(Ball.Get());
		}
	}
	return ValidPartyBalls;
}

void APBCombatPartyController::RequestUseSkill(const int32 SkillInputValue)
{
	const int32 SlotIndex = SkillInputValue - 1;
	if (SlotIndex < 0)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UPBBallDeckSubsystem* DeckSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
	if (!DeckSubsystem)
	{
		return;
	}

	const int32 BallInstanceId = DeckSubsystem->GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, SlotIndex);
	if (BallInstanceId == INDEX_NONE)
	{
		return;
	}

	APBBallBase* Ball = FindPartyBallByInstanceId(BallInstanceId);
	if (!IsValid(Ball))
	{
		return;
	}

	Ball->TryActivateSkill();
}

APBBallBase* APBCombatPartyController::FindPartyBallByInstanceId(const int32 BallInstanceId) const
{
	if (BallInstanceId == INDEX_NONE)
	{
		return nullptr;
	}

	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (IsValid(Ball.Get()) && Ball->GetBallInstanceId() == BallInstanceId)
		{
			return Ball.Get();
		}
	}

	return nullptr;
}
