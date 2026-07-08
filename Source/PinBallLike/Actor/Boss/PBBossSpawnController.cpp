// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBossSpawnController.h"

#include "Components/SceneComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Boss/DataAsset/PBBossDataAsset.h"
#include "PinBallLike/Table/Boss/PBBossAssetIds.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"

APBBossSpawnController::APBBossSpawnController()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBBossSpawnController::BeginPlay()
{
	Super::BeginPlay();

	if (IsSpawnOnBeginPlay)
	{
		SpawnBossAsync();
	}
}

void APBBossSpawnController::SpawnBossAsync()
{
	ClearSpawnedBoss();

	if (RequestBossDataAsync())
	{
		return;
	}

	if (BossRowName.IsNone() && SpawnBossWithClass(BossClass))
	{
		CompleteBossPreparation(true);
		return;
	}

	CompleteBossPreparation(false);
}

bool APBBossSpawnController::RequestBossDataAsync()
{
	if (BossRowName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load skipped. BossRowName is None."));
		return false;
	}

	if (!CachedGameDataLoadSubsystem)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			CachedGameDataLoadSubsystem = GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>();
		}
	}

	if (!IsValid(CachedGameDataLoadSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Cannot load boss data. BossRowName=%s"),
			*BossRowName.ToString());
		return false;
	}

	UnbindBossDataLoadEvent();
	CachedGameDataLoadSubsystem->OnPrimaryAssetLoadCompleted.AddUniqueDynamic(
		this,
		&APBBossSpawnController::HandleBossDataLoaded);

	TArray<FName> BossAssetNames;
	BossAssetNames.Add(BossRowName);

	TArray<FName> BundleNames;
	BundleNames.Add(PBAssetBundleNames::Gameplay);

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load started. BossRowName=%s Bundle=%s"),
		*BossRowName.ToString(),
		*PBAssetBundleNames::Gameplay.ToString());

	PendingBossLoadRequestId = CachedGameDataLoadSubsystem->LoadPrimaryAssetsByNamesAsync(
		PBBossAssetIds::Type::BossData,
		BossAssetNames,
		BundleNames);

	if (!PendingBossLoadRequestId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load failed to start. BossRowName=%s"),
			*BossRowName.ToString());
		UnbindBossDataLoadEvent();
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load request created. BossRowName=%s RequestId=%s"),
		*BossRowName.ToString(),
		*PendingBossLoadRequestId.ToString());

	return true;
}

void APBBossSpawnController::HandleBossDataLoaded(const FPBPrimaryAssetLoadResult& Result)
{
	if (Result.RequestId != PendingBossLoadRequestId)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BossSpawn] Ignore boss async load result. ExpectedRequestId=%s ActualRequestId=%s"),
			*PendingBossLoadRequestId.ToString(),
			*Result.RequestId.ToString());
		return;
	}

	UnbindBossDataLoadEvent();
	PendingBossLoadRequestId.Invalidate();

	if (!Result.bSuccess || !IsValid(CachedGameDataLoadSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load completed with failure. BossRowName=%s RequestId=%s Success=%s Loaded=%d Requested=%d HasSubsystem=%s"),
			*BossRowName.ToString(),
			*Result.RequestId.ToString(),
			Result.bSuccess ? TEXT("true") : TEXT("false"),
			Result.LoadedAssetIds.Num(),
			Result.RequestedAssetIds.Num(),
			IsValid(CachedGameDataLoadSubsystem) ? TEXT("true") : TEXT("false"));
		CompleteBossPreparation(false);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load completed. BossRowName=%s RequestId=%s Loaded=%d Requested=%d"),
		*BossRowName.ToString(),
		*Result.RequestId.ToString(),
		Result.LoadedAssetIds.Num(),
		Result.RequestedAssetIds.Num());

	const FPrimaryAssetId BossAssetId(PBBossAssetIds::Type::BossData, BossRowName);
	const UPBBossDataAsset* BossDataAsset =
		Cast<UPBBossDataAsset>(CachedGameDataLoadSubsystem->GetLoadedPrimaryAsset(BossAssetId));
	if (!IsValid(BossDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load failed. Loaded asset is invalid. BossRowName=%s AssetId=%s"),
			*BossRowName.ToString(),
			*BossAssetId.ToString());
		CompleteBossPreparation(false);
		return;
	}

	UClass* LoadedBossClass = BossDataAsset->BossClass.Get();
	if (!IsValid(LoadedBossClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load failed. BossClass is invalid. BossRowName=%s DataAsset=%s"),
			*BossRowName.ToString(),
			*GetNameSafe(BossDataAsset));
		CompleteBossPreparation(false);
		return;
	}

	const bool IsSpawnSuccess = SpawnBossWithClass(LoadedBossClass, BossDataAsset);
	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async spawn completed. BossRowName=%s BossClass=%s Success=%s"),
		*BossRowName.ToString(),
		*GetNameSafe(LoadedBossClass),
		IsSpawnSuccess ? TEXT("true") : TEXT("false"));

	CompleteBossPreparation(IsSpawnSuccess);
}

bool APBBossSpawnController::SpawnBossWithClass(const TSubclassOf<APBBossBase> BossClassToSpawn, const UPBBossDataAsset* BossDataAsset)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !BossClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Cannot spawn boss. BossClass=%s"),
			*GetNameSafe(BossClassToSpawn.Get()));
		return false;
	}

	const FTransform SpawnTransform(GetActorRotation(), GetActorLocation());

	SpawnedBoss = World->SpawnActorDeferred<APBBossBase>(
		BossClassToSpawn,
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!IsValid(SpawnedBoss))
	{
		return false;
	}

	if (IsValid(BossDataAsset))
	{
		SpawnedBoss->InitializeFromBossDataAsset(BossDataAsset);
	}

	UGameplayStatics::FinishSpawningActor(SpawnedBoss, SpawnTransform);

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Spawned boss. Boss=%s"),
		*GetNameSafe(SpawnedBoss.Get()));

	return true;
}

void APBBossSpawnController::ClearSpawnedBoss()
{
	if (IsValid(SpawnedBoss))
	{
		SpawnedBoss->Destroy();
		SpawnedBoss = nullptr;
	}
}

void APBBossSpawnController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindBossDataLoadEvent();
	ClearSpawnedBoss();

	Super::EndPlay(EndPlayReason);
}

void APBBossSpawnController::UnbindBossDataLoadEvent()
{
	if (!IsValid(CachedGameDataLoadSubsystem))
	{
		return;
	}

	CachedGameDataLoadSubsystem->OnPrimaryAssetLoadCompleted.RemoveDynamic(
		this,
		&APBBossSpawnController::HandleBossDataLoaded);
}

void APBBossSpawnController::CompleteBossPreparation(const bool IsSuccess) const
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePreparationCompletedMessage Message;
	Message.PreparationType = EPBBattlePreparationType::Boss;
	Message.bSuccess = IsSuccess;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Phase_Prepare_Completed,
		Message);
}
