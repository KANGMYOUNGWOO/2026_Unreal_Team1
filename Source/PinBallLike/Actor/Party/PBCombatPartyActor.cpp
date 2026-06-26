// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCombatPartyActor.h"

#include "Engine/GameInstance.h"
#include "Component/PBSnakeFormationComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Deck/PBBallDeckSubsystem.h"

APBCombatPartyActor::APBCombatPartyActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SnakeFormationComponent = CreateDefaultSubobject<UPBSnakeFormationComponent>(TEXT("SnakeFormationComponent"));
}

void APBCombatPartyActor::InitializeFromDeck()
{
	RefreshFromDeck();
}

void APBCombatPartyActor::BindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.AddDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.AddDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsReordered);
	DeckSubsystem->OnDeploymentSlotsRotated.AddDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsRotated);
}

void APBCombatPartyActor::UnbindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.RemoveDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsReordered);
	DeckSubsystem->OnDeploymentSlotsRotated.RemoveDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsRotated);
}

void APBCombatPartyActor::HandleDeploymentSlotChanged(int32 SlotIndex, APBBallBase* Ball)
{
	if (SlotIndex < 0)
	{
		return;
	}

	if (SlotIndex == 0)
	{
		if (LeaderBall)
		{
			LeaderBall->SetCombatRole(EPBBallPartyRole::None);
		}
		LeaderBall = Ball;
	}
	else
	{
		const int32 FollowerIndex = SlotIndex - 1;
		FollowerBalls.SetNum(FMath::Max(FollowerBalls.Num(), FollowerIndex + 1));
		if (FollowerBalls[FollowerIndex])
		{
			FollowerBalls[FollowerIndex]->SetCombatRole(EPBBallPartyRole::None);
		}
		FollowerBalls[FollowerIndex] = Ball;
	}

	PartyBalls.SetNum(FMath::Max(PartyBalls.Num(), SlotIndex + 1));
	PartyBalls[SlotIndex] = Ball;
	CompactPartyBalls();
	HandlePartyOrderChanged();
}

void APBCombatPartyActor::HandleDeploymentSlotsReordered()
{
	RefreshFromDeck();
}

void APBCombatPartyActor::HandleDeploymentSlotsRotated()
{
	RefreshFromDeck();
}

void APBCombatPartyActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		DeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>();
	}

	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("PBCombatPartyActor could not find PBBallDeckSubsystem."));
		return;
	}

	InitializeFromDeck();
	BindDeckEvents();
}

void APBCombatPartyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APBCombatPartyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPartyRoles();
	UnbindDeckEvents();
	Super::EndPlay(EndPlayReason);
}

void APBCombatPartyActor::RefreshFromDeck()
{
	ClearPartyRoles();

	if (!DeckSubsystem)
	{
		PartyBalls.Reset();
		LeaderBall = nullptr;
		FollowerBalls.Reset();
		if (SnakeFormationComponent)
		{
			SnakeFormationComponent->ClearFormation();
		}
		return;
	}

	PartyBalls.Reset();
	for (APBBallBase* Ball : DeckSubsystem->GetDeploymentBalls())
	{
		PartyBalls.Add(Ball);
	}

	LeaderBall = DeckSubsystem->GetLeaderBall();

	FollowerBalls.Reset();
	for (APBBallBase* Ball : DeckSubsystem->GetFollowerBalls())
	{
		FollowerBalls.Add(Ball);
	}

	HandlePartyOrderChanged();
}

void APBCombatPartyActor::HandlePartyOrderChanged()
{
	ApplyPartyRoles();

	if (!SnakeFormationComponent)
	{
		return;
	}

	TArray<APBBallBase*> OrderedBalls;
	OrderedBalls.Reserve(PartyBalls.Num());
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (IsValid(Ball.Get()))
		{
			OrderedBalls.Add(Ball.Get());
		}
	}

	SnakeFormationComponent->SetPartyBalls(OrderedBalls);
}

void APBCombatPartyActor::ApplyPartyRoles()
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

void APBCombatPartyActor::ClearPartyRoles()
{
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (Ball)
		{
			Ball->SetCombatRole(EPBBallPartyRole::None);
		}
	}
}

void APBCombatPartyActor::CompactPartyBalls()
{
	PartyBalls.RemoveAll([](const TObjectPtr<APBBallBase>& Ball)
	{
		return Ball == nullptr;
	});

	FollowerBalls.RemoveAll([](const TObjectPtr<APBBallBase>& Ball)
	{
		return Ball == nullptr;
	});
}
