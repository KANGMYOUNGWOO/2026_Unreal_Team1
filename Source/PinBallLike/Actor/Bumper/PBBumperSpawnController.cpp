// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperSpawnController.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBBumperPositionAnchor.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Utils/PBSubsystemUtils.h"

APBBumperSpawnController::APBBumperSpawnController()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

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

bool APBBumperSpawnController::RequestEquippedBumperGameplayAssetsAsync()
{
	if (!CacheRequiredSubsystems())
	{
		return false;
	}

	PendingBumperRowIds = CachedPlayerDataSubsystem->GetEquippedBumperRowIds();
	if (PendingBumperRowIds.IsEmpty())
	{
		return false;
	}

	TArray<FName> BundleNames;
	BundleNames.Add(PBAssetBundleNames::Gameplay);
	bPendingBumperAssetLoadCompleted = false;
	const FGuid RequestId = CachedGameDataLoadSubsystem->LoadPrimaryAssetsByNamesAsync(
		PBBumperAssetIds::Type::BumperData,
		PendingBumperRowIds,
		BundleNames);

	if (!bPendingBumperAssetLoadCompleted)
	{
		PendingBumperAssetLoadRequestId = RequestId;
	}

	return RequestId.IsValid() || bPendingBumperAssetLoadCompleted;
}

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

void APBBumperSpawnController::GetSpawnedBumpers(TArray<APBModularBumperBase*>& OutBumpers) const
{
	OutBumpers.Reset();
	OutBumpers.Reserve(SpawnedBumpers.Num());

	for (APBModularBumperBase* Bumper : SpawnedBumpers)
	{
		if (IsValid(Bumper))
		{
			OutBumpers.Add(Bumper);
		}
	}
}

void APBBumperSpawnController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(CachedGameDataLoadSubsystem))
	{
		CachedGameDataLoadSubsystem->OnPrimaryAssetLoadCompleted.RemoveDynamic(
			this,
			&APBBumperSpawnController::HandleEquippedBumperAssetsLoaded);
	}

	ClearSpawnedBumpers();

	Super::EndPlay(EndPlayReason);
}

void APBBumperSpawnController::PrepareEquippedBumpersAsync()
{
	if (!CacheRequiredSubsystems())
	{
		CompleteBumperPreparation(false);
		return;
	}

	CachedGameDataLoadSubsystem->OnPrimaryAssetLoadCompleted.AddUniqueDynamic(
		this,
		&APBBumperSpawnController::HandleEquippedBumperAssetsLoaded);

	if (!RequestEquippedBumperGameplayAssetsAsync())
	{
		CachedGameDataLoadSubsystem->OnPrimaryAssetLoadCompleted.RemoveDynamic(
			this,
			&APBBumperSpawnController::HandleEquippedBumperAssetsLoaded);
		CompleteBumperPreparation(false);
	}
}

void APBBumperSpawnController::HandleEquippedBumperAssetsLoaded(const FPBPrimaryAssetLoadResult& Result)
{
	bool bMatchesPendingRequest = PendingBumperAssetLoadRequestId.IsValid()
		&& Result.RequestId == PendingBumperAssetLoadRequestId;

	if (!bMatchesPendingRequest && !PendingBumperRowIds.IsEmpty() && Result.BundleKey == PBAssetBundleNames::Gameplay)
	{
		bMatchesPendingRequest = Result.RequestedAssetIds.Num() == PendingBumperRowIds.Num();
		for (const FName& BumperRowId : PendingBumperRowIds)
		{
			if (!Result.RequestedAssetIds.Contains(FPrimaryAssetId(PBBumperAssetIds::Type::BumperData, BumperRowId)))
			{
				bMatchesPendingRequest = false;
				break;
			}
		}
	}

	if (!bMatchesPendingRequest)
	{
		return;
	}

	if (IsValid(CachedGameDataLoadSubsystem))
	{
		CachedGameDataLoadSubsystem->OnPrimaryAssetLoadCompleted.RemoveDynamic(
			this,
			&APBBumperSpawnController::HandleEquippedBumperAssetsLoaded);
	}

	PendingBumperAssetLoadRequestId = FGuid();
	bPendingBumperAssetLoadCompleted = true;
	
	if (!Result.bSuccess || !CacheRequiredSubsystems())
	{
		CompleteBumperPreparation(false);
		return;
	}

	if (PendingBumperRowIds.IsEmpty())
	{
		PendingBumperRowIds = CachedPlayerDataSubsystem->GetEquippedBumperRowIds();
	}

	ClearSpawnedBumpers();
	CollectBumperAnchors();

	if (!BumperClass || PendingBumperRowIds.IsEmpty())
	{
		CompleteBumperPreparation(false);
		return;
	}

	SpawnPreparedBumpers();
}

void APBBumperSpawnController::SpawnPreparedBumpers()
{
	const int32 PreviousSpawnedCount = SpawnedBumpers.Num();
	for (const FName& BumperRowId : PendingBumperRowIds)
	{
		SpawnSingleBumper(BumperRowId);
	}

	const int32 SpawnedCount = SpawnedBumpers.Num() - PreviousSpawnedCount;
	const bool bSuccess = SpawnedCount == PendingBumperRowIds.Num();

	CompleteBumperPreparation(bSuccess);
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

void APBBumperSpawnController::CompleteBumperPreparation(const bool bSuccess)
{
	TArray<APBModularBumperBase*> SpawnedBumperActors;
	GetSpawnedBumpers(SpawnedBumperActors);
	OnSpawnedBumpersReady.Broadcast(bSuccess, SpawnedBumperActors);

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePreparationCompletedMessage Message;
	Message.PreparationType = EPBBattlePreparationType::Bumper;
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

