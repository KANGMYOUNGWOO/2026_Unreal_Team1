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

namespace
{
	const FName BossGameplayBundleKey(TEXT("BossGameplay"));
	const FName BossUIBundleKey(TEXT("BossUI"));

	FName MakeBundleKey(const TArray<FName>& BundleNames)
	{
		if (BundleNames.IsEmpty())
		{
			return NAME_None;
		}

		if (BundleNames.Num() == 1)
		{
			return BundleNames[0];
		}

		FString BundleKey;
		for (const FName& BundleName : BundleNames)
		{
			if (!BundleKey.IsEmpty())
			{
				BundleKey.AppendChar(TEXT('+'));
			}

			BundleKey.Append(BundleName.ToString());
		}

		return FName(*BundleKey);
	}

	FName MakeBossGameplayBundleKey()
	{
		return MakeBundleKey({ PBAssetBundleNames::Gameplay, BossGameplayBundleKey });
	}

	FName MakeBossUIBundleKey()
	{
		return MakeBundleKey({ PBAssetBundleNames::UI, BossUIBundleKey });
	}
}

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
	UnregisterBossDeadEvent();

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

	TArray<FName> GameplayBundleNames;
	GameplayBundleNames.Add(PBAssetBundleNames::Gameplay);
	GameplayBundleNames.Add(BossGameplayBundleKey);

	TArray<FName> UIBundleNames;
	UIBundleNames.Add(PBAssetBundleNames::UI);
	UIBundleNames.Add(BossUIBundleKey);

	TArray<FName> BossBundleNames;
	BossBundleNames.Append(GameplayBundleNames);
	BossBundleNames.Append(UIBundleNames);

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load started. BossRowName=%s Bundle=%s"),
		*BossRowName.ToString(),
		*MakeBossGameplayBundleKey().ToString());

	IsBossGameplayLoadCompleted = false;
	IsBossUILoadCompleted = false;
	IsBossGameplayLoadSuccess = false;
	IsBossUILoadSuccess = false;
	IsBossAssetsUnloaded = false;

	PendingBossGameplayLoadRequestId = CachedGameDataLoadSubsystem->LoadPrimaryAssetsByNamesAsync(
		PBBossAssetIds::Type::BossData,
		BossAssetNames,
		BossBundleNames);
	PendingBossUILoadRequestId = PendingBossGameplayLoadRequestId;

	if (!PendingBossGameplayLoadRequestId.IsValid() || !PendingBossUILoadRequestId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load failed to start. BossRowName=%s"),
			*BossRowName.ToString());
		UnbindBossDataLoadEvent();
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load request created. BossRowName=%s RequestId=%s"),
		*BossRowName.ToString(),
		*PendingBossGameplayLoadRequestId.ToString());

	return true;
}

void APBBossSpawnController::HandleBossDataLoaded(const FPBPrimaryAssetLoadResult& Result)
{
	const bool IsGameplayLoadResult = Result.RequestId == PendingBossGameplayLoadRequestId;
	const bool IsUILoadResult = Result.RequestId == PendingBossUILoadRequestId;
	if (!IsGameplayLoadResult && !IsUILoadResult)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BossSpawn] Ignore boss async load result. ExpectedRequestId=%s ActualRequestId=%s"),
			*PendingBossGameplayLoadRequestId.ToString(),
			*Result.RequestId.ToString());
		return;
	}

	if (IsGameplayLoadResult)
	{
		IsBossGameplayLoadCompleted = true;
		IsBossGameplayLoadSuccess = Result.bSuccess;
	}

	if (IsUILoadResult)
	{
		IsBossUILoadCompleted = true;
		IsBossUILoadSuccess = Result.bSuccess;
	}

	if (!IsBossGameplayLoadCompleted || !IsBossUILoadCompleted)
	{
		return;
	}

	UnbindBossDataLoadEvent();
	PendingBossGameplayLoadRequestId.Invalidate();
	PendingBossUILoadRequestId.Invalidate();

	if (!IsBossGameplayLoadSuccess || !IsBossUILoadSuccess || !IsValid(CachedGameDataLoadSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load completed with failure. BossRowName=%s GameplaySuccess=%s UISuccess=%s HasSubsystem=%s"),
			*BossRowName.ToString(),
			IsBossGameplayLoadSuccess ? TEXT("true") : TEXT("false"),
			IsBossUILoadSuccess ? TEXT("true") : TEXT("false"),
			IsValid(CachedGameDataLoadSubsystem) ? TEXT("true") : TEXT("false"));
		CompleteBossPreparation(false);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load completed. BossRowName=%s GameplayBundle=%s UIBundle=%s"),
		*BossRowName.ToString(),
		*MakeBossGameplayBundleKey().ToString(),
		*MakeBossUIBundleKey().ToString());

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

	if (IsSpawnSuccess)
	{
		RegisterBossDeadEvent();
	}

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
	UnregisterBossDeadEvent();
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

void APBBossSpawnController::RegisterBossDeadEvent()
{
	if (!UGameplayMessageSubsystem::HasInstance(this) || BossDeadListenerHandle.IsValid())
	{
		return;
	}

	BossDeadListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattleBossDeadMessage>(
		GameplayTags::Event_Battle_Boss_Dead,
		this,
		&APBBossSpawnController::HandleBossDeadMessage);
}

void APBBossSpawnController::UnregisterBossDeadEvent()
{
	if (BossDeadListenerHandle.IsValid())
	{
		BossDeadListenerHandle.Unregister();
	}
}

void APBBossSpawnController::HandleBossDeadMessage(FGameplayTag Channel, const FPBBattleBossDeadMessage& Message)
{
	static_cast<void>(Channel);

	if (Message.BossActor != SpawnedBoss)
	{
		return;
	}

	UnloadBossAssets();
	UnregisterBossDeadEvent();
}

void APBBossSpawnController::UnloadBossAssets()
{
	if (IsBossAssetsUnloaded || !IsValid(CachedGameDataLoadSubsystem))
	{
		return;
	}

	CachedGameDataLoadSubsystem->UnloadPrimaryAssetBundle(MakeBossGameplayBundleKey());
	CachedGameDataLoadSubsystem->UnloadPrimaryAssetBundle(MakeBossUIBundleKey());
	IsBossAssetsUnloaded = true;
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
