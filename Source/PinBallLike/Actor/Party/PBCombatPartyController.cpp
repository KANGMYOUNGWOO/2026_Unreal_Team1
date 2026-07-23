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
#include "PinBallLike/Actor/Ball/Component/PBBallDashComponent.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PinBallLike/Struct/Effect/PBEffectContext.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBEffectSubsystem.h"
#include "PinBallLike/Subsystem/PBSoundSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/Relic/PBRelicSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallVoiceDataAsset.h"
#include "PinBallLike/Relic/PBRelicCalculator.h"

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
	BattleMessageDependencies.RequestDashToBoss = [this]()
	{
		RequestDashToBoss();
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
	UnbindRelicEvents();
	
	ClearPartyRoles();
	DestroyPartyBalls();
	Super::EndPlay(EndPlayReason);
}

void APBCombatPartyController::SpawnBallDeathEffect(APBBallBase* DeadBall) const
{
	if (!IsValid(DeadBall))
	{
		return;
	}

	if (BallDeathEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			BallDeathEffect,
			DeadBall->GetActorLocation(),
			DeadBall->GetActorRotation());
	}

	if (USoundBase* BallDeathSound = ResolveBallDeathSound(DeadBall))
	{
		if (UPBSoundSubsystem* SoundSubsystem = UPBSoundSubsystem::Get(this))
		{
			SoundSubsystem->PlaySFX(BallDeathSound, BallVoiceData->DeathSoundVolume);
		}
	}
}

USoundBase* APBCombatPartyController::ResolveBallDeathSound(const APBBallBase* DeadBall) const
{
	if (!IsValid(DeadBall) || !BallVoiceData)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	if (!TableDataSubsystem)
	{
		return nullptr;
	}

	FPBBallTableRow BallRow;
	if (!TableDataSubsystem->FindBallRow(DeadBall->GetBallId(), BallRow))
	{
		return nullptr;
	}

	return BallVoiceData->GetRandomDeathSound(BallRow.VoiceType);
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
	
	RefreshPartyRelicStats();
	RebuildPartyRoles();
	ApplyActiveSynergyEffects();
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

void APBCombatPartyController::BeginPlay()
{
	AActor::BeginPlay();
	
	BindRelicEvents();
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

void APBCombatPartyController::ApplyActiveSynergyEffects()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBEffectSubsystem* EffectSubsystem = GameInstance->GetSubsystem<UPBEffectSubsystem>();
	if (!EffectSubsystem)
	{
		return;
	}

	FPBEffectContext EffectContext;
	EffectContext.WorldContextObject = this;
	EffectContext.SourceActor = this;
	EffectContext.TargetActors.Reserve(PartyBalls.Num());
	for (AActor* Ball : GetValidPartyBalls())
	{
		EffectContext.TargetActors.Add(Ball);
	}

	EffectSubsystem->NotifyTrigger(GameplayTags::TriggerEvent_Battle_PartyBuilt, EffectContext);
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

	UPBBaseResourceComponent* ResourceComponent = Ball->GetResourceComponent();
	const float MaxMana = ResourceComponent
		? ResourceComponent->GetResourceMax(PBResourceNames::Mana)
		: 0.0f;
	if (MaxMana > 0.0f
		&& ResourceComponent->GetResourceCurrent(PBResourceNames::Mana) >= MaxMana
		&& Ball->TryActivateSkill())
	{
		ResourceComponent->ConsumeResource(PBResourceNames::Mana, MaxMana);
	}
}

void APBCombatPartyController::RequestDashToBoss()
{
	APBBallBase* Ball = GetLeaderBall();
	UPBBallDashComponent* DashComponent = IsValid(Ball)
		? Ball->GetDashComponent()
		: nullptr;
	if (DashComponent)
	{
		DashComponent->DashToBossWithDefaultSpeed();
	}
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

void APBCombatPartyController::BindRelicEvents()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBRelicSubsystem* RelicSubsystem =
		GameInstance->GetSubsystem<UPBRelicSubsystem>();

	if (!RelicSubsystem)
	{
		return;
	}

	RelicSubsystem->OnRelicsChanged.AddUObject(
		this,
		&APBCombatPartyController::HandleRelicsChanged);
}

void APBCombatPartyController::UnbindRelicEvents()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBRelicSubsystem* RelicSubsystem =
		GameInstance->GetSubsystem<UPBRelicSubsystem>();

	if (!RelicSubsystem)
	{
		return;
	}

	RelicSubsystem->OnRelicsChanged.RemoveAll(this);
}

void APBCombatPartyController::HandleRelicsChanged()
{
	RefreshPartyRelicStats();
}

void APBCombatPartyController::RefreshPartyRelicStats()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBRelicSubsystem* RelicSubsystem =
		GameInstance->GetSubsystem<UPBRelicSubsystem>();

	if (!RelicSubsystem)
	{
		return;
	}

	const UPBRelicCalculator* RelicCalculator =
		RelicSubsystem->GetCalculator();

	if (!IsValid(RelicCalculator))
	{
		return;
	}

	for (APBBallBase* Ball : GetValidPartyBalls())
	{
		if (!IsValid(Ball))
		{
			continue;
		}

		Ball->RefreshRelicStats(
			RelicCalculator);
	}
}
