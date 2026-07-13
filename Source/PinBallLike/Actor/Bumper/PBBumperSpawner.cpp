// Fill out your copyright notice in the Description page of Project Settings.

#include "PBBumperSpawner.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/Modular/PBBumperPositionAnchor.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
#include "PinBallLike/Utils/PBSubsystemUtils.h"

APBBumperSpawner::APBBumperSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBBumperSpawner::BeginPlay()
{
	Super::BeginPlay();

	CacheRequiredSubsystems();
}

void APBBumperSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CollectBumperAnchors();
}

void APBBumperSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearSpawnedBumpers();

	Super::EndPlay(EndPlayReason);
}

void APBBumperSpawner::CollectBumperAnchors()
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
		if (IsValid(Anchor) && Anchor->GetPositionId() != EPBBumperPositionId::None)
		{
			AnchorTransforms.Add(Anchor->GetPositionId(), Anchor->GetActorTransform());
		}
	}
}

FGuid APBBumperSpawner::LoadEquippedBumperDataAssetAsync(const FStreamableDelegate& OnLoaded)
{
	if (!CacheRequiredSubsystems())
	{
		return FGuid();
	}

	PendingBumperRowIds = CachedPlayerDataSubsystem->GetEquippedBumperRowIds();

	// 장착된 RowId를 PrimaryAssetId로 바꿔 Gameplay 번들을 로드한다.
	TArray<FPrimaryAssetId> BumperAssetIds;
	BuildPendingBumperAssetIds(BumperAssetIds);
	if (BumperAssetIds.IsEmpty())
	{
		return FGuid();
	}

	TArray<FName> BundleNames;
	BundleNames.Add(PBAssetBundleNames::Gameplay);

	return CachedGameDataLoadSubsystem->LoadPrimaryAssetsByIdsAsync(
		BumperAssetIds,
		BundleNames,
		OnLoaded);
}

void APBBumperSpawner::SpawnLoadedBumpers()
{
	if (!CacheRequiredSubsystems())
	{
		CompleteBumperPreparation(false);
		return;
	}

	ClearSpawnedBumpers();
	CollectBumperAnchors();

	// 로드된 DataAsset과 테이블 row를 실제 스폰용 데이터로 변환한다.
	if (!BuildPreparedBumperSpawnData())
	{
		CompleteBumperPreparation(false);
		return;
	}

	PlacePreparedBumperActors();
}

void APBBumperSpawner::ClearSpawnedBumpers()
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

void APBBumperSpawner::GetSpawnedBumpers(TArray<APBModularBumperBase*>& OutBumpers) const
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

void APBBumperSpawner::BuildPendingBumperAssetIds(TArray<FPrimaryAssetId>& OutAssetIds) const
{
	OutAssetIds.Reset();
	OutAssetIds.Reserve(PendingBumperRowIds.Num());

	for (const FName& BumperRowId : PendingBumperRowIds)
	{
		if (!BumperRowId.IsNone())
		{
			OutAssetIds.Add(FPrimaryAssetId(PBBumperAssetIds::Type::BumperData, BumperRowId));
		}
	}
}

bool APBBumperSpawner::BuildPreparedBumperSpawnData()
{
	PreparedBumperSpawnDataList.Reset();
	if (PendingBumperRowIds.IsEmpty())
	{
		return false;
	}

	for (const FName& BumperRowId : PendingBumperRowIds)
	{
		FPBPreparedBumperSpawnData SpawnData;
		if (TryBuildBumperSpawnData(BumperRowId, SpawnData))
		{
			PreparedBumperSpawnDataList.Add(SpawnData);
		}
	}

	return PreparedBumperSpawnDataList.Num() == PendingBumperRowIds.Num();
}

bool APBBumperSpawner::TryBuildBumperSpawnData(
	const FName BumperRowId,
	FPBPreparedBumperSpawnData& OutSpawnData) const
{
	if (BumperRowId.IsNone()
		|| !CachedTableDataSubsystem
		|| !CachedGameDataLoadSubsystem
		|| !CachedTableDataSubsystem->FindBumperRow(BumperRowId, OutSpawnData.BumperRow))
	{
		return false;
	}

	const FPrimaryAssetId BumperAssetId(PBBumperAssetIds::Type::BumperData, BumperRowId);
	const UPBBumperDataAsset* BumperDataAsset =
		Cast<UPBBumperDataAsset>(CachedGameDataLoadSubsystem->GetLoadedPrimaryAsset(BumperAssetId));
	if (!IsValid(BumperDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Missing loaded data asset. RowName=%s"), *BumperRowId.ToString());
		return false;
	}

	if (!TryBuildTriggerSpawnInfos(BumperRowId, BumperDataAsset, OutSpawnData.TriggerSpawnInfos))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Failed to prepare trigger data. RowName=%s TriggerId=%s"),
			*BumperRowId.ToString(),
			*OutSpawnData.BumperRow.TriggerID.ToString());
		return false;
	}

	if (!CachedTableDataSubsystem->FindLinkedBumperEffectRow(BumperRowId, OutSpawnData.EffectRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Missing linked effect row. RowName=%s EffectId=%s"),
			*BumperRowId.ToString(),
			*OutSpawnData.BumperRow.EffectID.ToString());
		return false;
	}

	UClass* LoadedEffectClass = BumperDataAsset->EffectClass.Get();
	if (!IsValid(LoadedEffectClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Bumper] Missing loaded effect class. RowName=%s EffectId=%s"),
			*BumperRowId.ToString(),
			*OutSpawnData.BumperRow.EffectID.ToString());
		return false;
	}

	OutSpawnData.EffectClass = TSubclassOf<UPBBumperEffectBase>(LoadedEffectClass);
	return true;
}

bool APBBumperSpawner::TryBuildTriggerSpawnInfos(
	const FName BumperRowId,
	const UPBBumperDataAsset* BumperDataAsset,
	TArray<FPBBumperTriggerSpawnInfo>& OutTriggerSpawnInfos) const
{
	if (!IsValid(BumperDataAsset) || !IsValid(BumperDataAsset->TriggerClass.Get()))
	{
		return false;
	}

	FPBBumperTriggerRow TriggerRow;
	if (!CachedTableDataSubsystem->FindLinkedBumperTriggerRow(BumperRowId, TriggerRow))
	{
		return false;
	}

	// Modular Bumper가 BeginPlay에서 이 정보로 trigger actor를 생성한다.
	FPBBumperTriggerSpawnInfo TriggerSpawnInfo;
	TriggerSpawnInfo.TriggerClass =
		TSubclassOf<APBBumperTriggerActorBase>(BumperDataAsset->TriggerClass.Get());
	TriggerSpawnInfo.PositionIds = TriggerRow.PositionIds;

	OutTriggerSpawnInfos.Reset();
	OutTriggerSpawnInfos.Add(TriggerSpawnInfo);
	return true;
}

void APBBumperSpawner::PlacePreparedBumperActors()
{
	if (!BumperClass || PreparedBumperSpawnDataList.IsEmpty())
	{
		CompleteBumperPreparation(false);
		return;
	}

	// 준비된 데이터만 사용해서 actor 배치 단계는 단순하게 유지한다.
	for (const FPBPreparedBumperSpawnData& SpawnData : PreparedBumperSpawnDataList)
	{
		if (APBModularBumperBase* Bumper = PlaceBumperActor(SpawnData))
		{
			SpawnedBumpers.Add(Bumper);
		}
	}

	CompleteBumperPreparation(SpawnedBumpers.Num() == PreparedBumperSpawnDataList.Num());
}

APBModularBumperBase* APBBumperSpawner::PlaceBumperActor(const FPBPreparedBumperSpawnData& SpawnData)
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

	Bumper->InitializeBumper(
		SpawnData.BumperRow,
		SpawnData.TriggerSpawnInfos,
		SpawnData.EffectRow,
		SpawnData.EffectClass,
		AnchorTransforms);
	UGameplayStatics::FinishSpawningActor(Bumper, FTransform::Identity);

	return Bumper;
}

void APBBumperSpawner::CompleteBumperPreparation(const bool bSuccess)
{
	PreparedBumperSpawnDataList.Reset();

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

bool APBBumperSpawner::CacheRequiredSubsystems()
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
