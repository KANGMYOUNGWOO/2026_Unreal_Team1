// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperSpawnController.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBBumperPositionAnchor.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/GamePlayMessage/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Utils/PBSubsystemUtils.h"

APBBumperSpawnController::APBBumperSpawnController()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

// Lifecycle.

void APBBumperSpawnController::BeginPlay()
{
	Super::BeginPlay();

	CacheRequiredSubsystems();
}

void APBBumperSpawnController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CollectBumperAnchors();
}

void APBBumperSpawnController::CollectBumperAnchors()
{
	AnchorTransforms.Reset();

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (TActorIterator<APBBumperPositionAnchor> It(World); It; ++It)
	{
		const APBBumperPositionAnchor* Anchor = *It;
		if (!IsValid(Anchor) || Anchor->GetPositionId() == EPBBumperPositionId::None)
		{
			continue;
		}

		AnchorTransforms.Add(Anchor->GetPositionId(), Anchor->GetActorTransform());
	}
}

void APBBumperSpawnController::SpawnEquippedBumpers()
{
	if (!CacheRequiredSubsystems())
	{
		CompleteBumperPreparation(0, false);
		return;
	}

	PendingBumperRowIds = CachedPlayerDataSubsystem->GetEquippedBumperRowIds();
	ClearSpawnedBumpers();
	CollectBumperAnchors();

	if (!BumperClass || PendingBumperRowIds.IsEmpty())
	{
		CompleteBumperPreparation(0, false);
		return;
	}

	SpawnPreparedBumpers();
}
// Cleanup.

void APBBumperSpawnController::ClearSpawnedBumpers()
{
	for (APBModularBumperBase* Bumper : SpawnedBumpers)
	{
		if (IsValid(Bumper))
		{
			Bumper->Destroy();
		}
	}

	SpawnedBumpers.Reset();
}

void APBBumperSpawnController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearSpawnedBumpers();

	Super::EndPlay(EndPlayReason);
}
// Spawn steps.

void APBBumperSpawnController::SpawnPreparedBumpers()
{
	const int32 PreviousSpawnedCount = SpawnedBumpers.Num();
	for (const FName& BumperRowId : PendingBumperRowIds)
	{
		SpawnSingleBumper(BumperRowId);
	}

	const int32 SpawnedCount = SpawnedBumpers.Num() - PreviousSpawnedCount;
	const bool bSuccess = SpawnedCount == PendingBumperRowIds.Num();

	CompleteBumperPreparation(SpawnedCount, bSuccess);
}

APBModularBumperBase* APBBumperSpawnController::SpawnSingleBumper(const FName BumperRowId)
{
	if (BumperRowId.IsNone() || !CacheRequiredSubsystems())
	{
		return nullptr;
	}

	FPBBumperTableRow BumperRow;
	FPBBumperTriggerRow TriggerRow;
	const UPBBumperDataAsset* BumperDataAsset = nullptr;
	if (!TryMakeBumperSpawnData(BumperRowId, BumperRow, TriggerRow, BumperDataAsset))
	{
		return nullptr;
	}

	const TArray<FPBBumperTriggerSpawnInfo> TriggerSpawnInfos =
		MakeTriggerSpawnInfos(TriggerRow, BumperDataAsset);
	if (TriggerSpawnInfos.IsEmpty())
	{
		return nullptr;
	}

	APBModularBumperBase* Bumper = SpawnInitializedBumper(BumperRow, TriggerSpawnInfos);
	if (!IsValid(Bumper))
	{
		return nullptr;
	}

	SpawnedBumpers.Add(Bumper);

	return Bumper;
}

bool APBBumperSpawnController::TryMakeBumperSpawnData(
	const FName BumperRowId,
	FPBBumperTableRow& OutBumperRow,
	FPBBumperTriggerRow& OutTriggerRow,
	const UPBBumperDataAsset*& OutBumperDataAsset) const
{
	if (!CachedTableDataSubsystem || !CachedGameDataLoadSubsystem)
	{
		return false;
	}

	if (!CachedTableDataSubsystem->FindBumperRow(BumperRowId, OutBumperRow))
	{
		return false;
	}

	if (!CachedTableDataSubsystem->FindLinkedBumperTriggerRow(BumperRowId, OutTriggerRow))
	{
		return false;
	}

	// 범퍼 row id와 같은 이름의 BumperData primary asset을 사용한다.
	const FPrimaryAssetId BumperAssetId(PBBumperAssetIds::Type::BumperData, BumperRowId);
	OutBumperDataAsset =
		Cast<UPBBumperDataAsset>(CachedGameDataLoadSubsystem->GetLoadedPrimaryAsset(BumperAssetId));

	return IsValid(OutBumperDataAsset);
}

TArray<FPBBumperTriggerSpawnInfo> APBBumperSpawnController::MakeTriggerSpawnInfos(
	const FPBBumperTriggerRow& TriggerRow,
	const UPBBumperDataAsset* BumperDataAsset) const
{
	TArray<FPBBumperTriggerSpawnInfo> TriggerSpawnInfos;
	if (!IsValid(BumperDataAsset))
	{
		return TriggerSpawnInfos;
	}

	UClass* TriggerClass = BumperDataAsset->TriggerClass.Get();
	if (!IsValid(TriggerClass))
	{
		return TriggerSpawnInfos;
	}

	// Modular bumper는 BeginPlay에서 이 정보로 trigger actor를 생성한다.
	FPBBumperTriggerSpawnInfo TriggerSpawnInfo;
	TriggerSpawnInfo.TriggerClass = TSubclassOf<APBBumperTriggerActorBase>(TriggerClass);
	TriggerSpawnInfo.PositionIds = TriggerRow.PositionIds;
	TriggerSpawnInfos.Add(TriggerSpawnInfo);

	return TriggerSpawnInfos;
}

APBModularBumperBase* APBBumperSpawnController::SpawnInitializedBumper(
	const FPBBumperTableRow& BumperRow,
	const TArray<FPBBumperTriggerSpawnInfo>& TriggerSpawnInfos)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !BumperClass)
	{
		return nullptr;
	}

	APBModularBumperBase* Bumper = World->SpawnActorDeferred<APBModularBumperBase>(
		BumperClass,
		FTransform::Identity);
	if (!IsValid(Bumper))
	{
		return nullptr;
	}

	Bumper->InitializeBumper(BumperRow, TriggerSpawnInfos, nullptr, AnchorTransforms);
	UGameplayStatics::FinishSpawningActor(Bumper, FTransform::Identity);

	return Bumper;
}

void APBBumperSpawnController::CompleteBumperPreparation(
	const int32 CompletedCount,
	const bool bSuccess) const
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePreparationCompletedMessage Message;
	Message.PreparationType = EPBBattlePreparationType::Bumper;
	Message.RequestedCount = PendingBumperRowIds.Num();
	Message.CompletedCount = CompletedCount;
	Message.bSuccess = bSuccess;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Phase_Prepare_Completed,
		Message);
}

bool APBBumperSpawnController::CacheRequiredSubsystems()
{
	if (!CachedGameDataLoadSubsystem)
	{
		CachedGameDataLoadSubsystem =
			PBSubsystemUtils::GetGameInstanceSubsystem<UPBGameDataLoadSubsystem>(this);
	}

	if (!CachedTableDataSubsystem)
	{
		CachedTableDataSubsystem =
			PBSubsystemUtils::GetGameInstanceSubsystem<UPBTableDataSubsystem>(this);
	}

	if (!CachedPlayerDataSubsystem)
	{
		CachedPlayerDataSubsystem =
			PBSubsystemUtils::GetGameInstanceSubsystem<UPBPlayerDataSubsystem>(this);
	}

	return IsValid(CachedGameDataLoadSubsystem)
		&& IsValid(CachedTableDataSubsystem)
		&& IsValid(CachedPlayerDataSubsystem);
}
