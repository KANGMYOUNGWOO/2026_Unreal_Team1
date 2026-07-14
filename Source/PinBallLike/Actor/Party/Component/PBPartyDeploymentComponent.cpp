// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPartyDeploymentComponent.h"

#include "Engine/EngineTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PinBallLike/Struct/Ball/PBBallInstanceData.h"
#include "PinBallLike/Struct/Deck/PBDeckOwnedBallData.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckAssetLoadService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"

UPBPartyDeploymentComponent::UPBPartyDeploymentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBPartyDeploymentComponent::InitializeDependencies(FPBPartyDeploymentDependencies InDependencies)
{
	Dependencies = MoveTemp(InDependencies);
}

void UPBPartyDeploymentComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheDeckSubsystem();
	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("PBPartyDeploymentComponent could not find PBBallDeckSubsystem. Owner=%s"),
			*GetNameSafe(GetOwner()));
		return;
	}

	BindDeckEvents();
	ScheduleInitialRebuild();
}

void UPBPartyDeploymentComponent::CacheDeckSubsystem()
{
	if (const UGameInstance* GameInstance = GetOwner() ? GetOwner()->GetGameInstance() : nullptr)
	{
		DeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>();
	}
}

void UPBPartyDeploymentComponent::ScheduleInitialRebuild()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UPBPartyDeploymentComponent::RebuildPartyFromDeployment));
	}
}

void UPBPartyDeploymentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindDeckEvents();
	Super::EndPlay(EndPlayReason);
}

void UPBPartyDeploymentComponent::BindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.AddUniqueDynamic(this, &UPBPartyDeploymentComponent::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.AddUniqueDynamic(this, &UPBPartyDeploymentComponent::HandleDeploymentChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.AddUniqueDynamic(this, &UPBPartyDeploymentComponent::HandleDeploymentChanged);
}

void UPBPartyDeploymentComponent::UnbindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(this, &UPBPartyDeploymentComponent::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.RemoveDynamic(this, &UPBPartyDeploymentComponent::HandleDeploymentChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.RemoveDynamic(this, &UPBPartyDeploymentComponent::HandleDeploymentChanged);
}

void UPBPartyDeploymentComponent::HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId)
{
	(void)SlotIndex;
	(void)BallInstanceId;
	RebuildPartyFromDeployment();
}

void UPBPartyDeploymentComponent::HandleDeploymentChanged()
{
	RebuildPartyFromDeployment();
}

void UPBPartyDeploymentComponent::PrepareForDeployment()
{
	if (Dependencies.UnbindPartyBalls)
	{
		Dependencies.UnbindPartyBalls();
	}
	if (Dependencies.ResetPartyDeathState)
	{
		Dependencies.ResetPartyDeathState();
	}
	if (Dependencies.SetLauncherActive)
	{
		Dependencies.SetLauncherActive(true);
	}

	RebuildPartyFromDeployment();
}

void UPBPartyDeploymentComponent::RebuildPartyFromDeployment()
{
	ResetCurrentParty();
	if (!DeckSubsystem)
	{
		if (Dependencies.ClearSnakeFormation)
		{
			Dependencies.ClearSnakeFormation();
		}
		return;
	}

	if (Dependencies.SetPartyBalls)
	{
		Dependencies.SetPartyBalls(SpawnDeploymentPartyBalls());
	}
	if (Dependencies.RefreshPartyOrder)
	{
		Dependencies.RefreshPartyOrder();
	}
}

void UPBPartyDeploymentComponent::ResetCurrentParty()
{
	if (Dependencies.StopLeaderPromotion)
	{
		Dependencies.StopLeaderPromotion();
	}
	if (Dependencies.UnbindPartyBalls)
	{
		Dependencies.UnbindPartyBalls();
	}
	if (Dependencies.ClearPartyRoles)
	{
		Dependencies.ClearPartyRoles();
	}
	if (Dependencies.DestroyPartyBalls)
	{
		Dependencies.DestroyPartyBalls();
	}
}

TArray<TObjectPtr<APBBallBase>> UPBPartyDeploymentComponent::SpawnDeploymentPartyBalls()
{
	TArray<TObjectPtr<APBBallBase>> SpawnedPartyBalls;
	if (!DeckSubsystem || !Dependencies.GetSpawnTransform)
	{
		return SpawnedPartyBalls;
	}

	const FTransform SpawnTransform = Dependencies.GetSpawnTransform();
	const TArray<int32> DeploymentBallInstanceIds = DeckSubsystem->GetDeploymentBallInstanceIds();
	for (const int32 BallInstanceId : DeploymentBallInstanceIds)
	{
		APBBallBase* Ball = SpawnBallFromDeckInstance(BallInstanceId, SpawnTransform);
		if (Ball)
		{
			SpawnedPartyBalls.Add(Ball);
		}
	}

	return SpawnedPartyBalls;
}

APBBallBase* UPBPartyDeploymentComponent::SpawnBallFromDeckInstance(
	const int32 BallInstanceId,
	const FTransform& SpawnTransform)
{
	if (!DeckSubsystem || !Dependencies.GetSpawnOwner)
	{
		return nullptr;
	}

	FPBBallInstanceData NewBallInstanceData;
	if (!BuildBallInstanceData(BallInstanceId, NewBallInstanceData))
	{
		return nullptr;
	}

	const FPBDeckOwnedBallData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyDeployment] SpawnBall failed. Missing owned ball data. BallInstanceId=%d"),
			BallInstanceId);
		return nullptr;
	}

	const UPBBallDeckAssetLoadService* AssetLoadService = DeckSubsystem->GetAssetLoadService();
	UClass* BallActorClass = AssetLoadService ? AssetLoadService->GetLoadedBallActorClass(BallInstanceId) : nullptr;
	if (!IsValid(BallActorClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyDeployment] SpawnBall failed. Ball actor class is not loaded. BallInstanceId=%d BallId=%s StarLevel=%d"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			BallInstanceData->StarLevel);
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APBBallBase* SpawnedBall = World->SpawnActorDeferred<APBBallBase>(
		BallActorClass,
		SpawnTransform,
		Dependencies.GetSpawnOwner(),
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!SpawnedBall)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyDeployment] SpawnBall failed. SpawnActorDeferred returned null. BallInstanceId=%d BallId=%s ActorClass=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			*GetNameSafe(BallActorClass));
		return nullptr;
	}

	SpawnedBall->InitializeFromBallInstanceData(NewBallInstanceData);
	const UPBBallDataAsset* BallDataAsset =
		AssetLoadService ? AssetLoadService->GetLoadedBallDataAsset(BallInstanceId) : nullptr;
	SpawnedBall->InitializeSkillActorClass(
		BallDataAsset ? BallDataAsset->SkillActorClass.Get() : nullptr);
	SpawnedBall->FinishSpawning(SpawnTransform);
	SpawnedBall->SetActorHiddenInGame(true);

	return SpawnedBall;
}

bool UPBPartyDeploymentComponent::BuildBallInstanceData(
	const int32 BallInstanceId,
	FPBBallInstanceData& OutBallInstanceData) const
{
	if (!DeckSubsystem)
	{
		return false;
	}

	const FPBDeckOwnedBallData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyDeployment] SpawnBall failed. Missing owned ball data. BallInstanceId=%d"),
			BallInstanceId);
		return false;
	}

	const UGameInstance* GameInstance = DeckSubsystem->GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	if (!TableDataSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyDeployment] SpawnBall failed. Missing table data subsystem. BallInstanceId=%d BallId=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString());
		return false;
	}

	FPBBallTableRow BallRow;
	if (BallInstanceData->BallId.IsNone() || !TableDataSubsystem->FindBallRow(BallInstanceData->BallId, BallRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyDeployment] SpawnBall failed. Ball row not found. BallInstanceId=%d BallId=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString());
		return false;
	}

	FName StarLevelRowName;
	FPBBallStarLevelRow StarLevelRow;
	if (!TableDataSubsystem->FindBallStarLevelRow(
		BallInstanceData->BallId,
		BallInstanceData->StarLevel,
		StarLevelRowName,
		StarLevelRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PartyDeployment] SpawnBall failed. Star level row not found. BallInstanceId=%d BallId=%s StarLevel=%d"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			BallInstanceData->StarLevel);
		return false;
	}

	OutBallInstanceData = FPBBallInstanceData();
	OutBallInstanceData.InstanceId = BallInstanceData->InstanceId;
	OutBallInstanceData.BallId = BallInstanceData->BallId;
	OutBallInstanceData.StarLevel = BallInstanceData->StarLevel;

	for (const TPair<FName, int32>& ResourceValue : StarLevelRow.BaseResources)
	{
		if (!ResourceValue.Key.IsNone())
		{
			const float MaxValue = static_cast<float>(ResourceValue.Value);
			const float CurrentValue = ResourceValue.Key == PBResourceNames::Mana
				? FMath::Clamp(DeckSubsystem->GetOwnedBallSavedMana(BallInstanceId), 0.0f, MaxValue)
				: MaxValue;
			OutBallInstanceData.BaseResources.Add(FPBResourceData(
				ResourceValue.Key,
				CurrentValue,
				MaxValue,
				0.0f));
		}
	}

	for (const TPair<FName, int32>& StatValue : StarLevelRow.BaseStats)
	{
		if (!StatValue.Key.IsNone())
		{
			OutBallInstanceData.BaseStats.Add(FPBStatData(StatValue.Key, StatValue.Value));
		}
	}

	return true;
}
