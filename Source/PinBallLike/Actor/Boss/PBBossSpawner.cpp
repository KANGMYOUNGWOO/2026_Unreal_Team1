// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBossSpawner.h"

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

	FName MakeBossDataBundleKey()
	{
		return MakeBundleKey({ PBAssetBundleNames::Gameplay, BossGameplayBundleKey, PBAssetBundleNames::UI, BossUIBundleKey });
	}

	bool ArePatternClassesLoaded(const TArray<FPBBossPatternData>& PatternDatas)
	{
		for (const FPBBossPatternData& PatternData : PatternDatas)
		{
			if (PatternData.IsEnabled && !PatternData.PatternClass.IsNull() && !PatternData.PatternClass.Get())
			{
				return false;
			}
		}

		return true;
	}

	bool AreBossPatternClassesLoaded(const UPBBossDataAsset* BossDataAsset)
	{
		return BossDataAsset
			&& ArePatternClassesLoaded(BossDataAsset->PatternDatas)
			&& ArePatternClassesLoaded(BossDataAsset->EnragedPatternDatas)
			&& ArePatternClassesLoaded(BossDataAsset->EnragedEntryPatternDatas);
	}
}

APBBossSpawner::APBBossSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBBossSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (IsSpawnOnBeginPlay)
	{
		SpawnBossAsync();
	}
}

void APBBossSpawner::SpawnBossAsync()
{
	if (BossRowName.IsNone())
	{
		SpawnLoadedBoss();
		return;
	}

	const FGuid RequestId = LoadBossDataAssetAsync(
		FStreamableDelegate::CreateUObject(this, &APBBossSpawner::SpawnLoadedBoss));
	if (!RequestId.IsValid())
	{
		CompleteBossPreparation(false);
	}
}

FGuid APBBossSpawner::LoadBossDataAssetAsync(FStreamableDelegate OnLoaded)
{
	if (RequestBossDataAsync(OnLoaded))
	{
		return PendingBossDataLoadRequestId;
	}

	return FGuid();
}

void APBBossSpawner::SpawnLoadedBoss()
{
	ClearSpawnedBoss();
	UnregisterBossDeadEvent();

	if (BossRowName.IsNone() && SpawnBossWithClass(BossClass))
	{
		CompleteBossPreparation(true);
		return;
	}

	if (BossRowName.IsNone())
	{
		CompleteBossPreparation(false);
		return;
	}

	if (!IsLoadedBossDataReady() || !IsValid(CachedGameDataLoadSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss spawn failed. Boss data is not ready. BossRowName=%s"),
			*BossRowName.ToString());
		CompleteBossPreparation(false);
		return;
	}

	const FPrimaryAssetId BossAssetId(PBBossAssetIds::Type::BossData, BossRowName);
	const UPBBossDataAsset* BossDataAsset =
		Cast<UPBBossDataAsset>(CachedGameDataLoadSubsystem->GetLoadedPrimaryAsset(BossAssetId));
	if (!IsValid(BossDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss spawn failed. Loaded asset is invalid. BossRowName=%s AssetId=%s"),
			*BossRowName.ToString(),
			*BossAssetId.ToString());
		CompleteBossPreparation(false);
		return;
	}

	UClass* LoadedBossClass = BossDataAsset->BossClass.Get();
	if (!IsValid(LoadedBossClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss spawn failed. BossClass is invalid. BossRowName=%s DataAsset=%s"),
			*BossRowName.ToString(),
			*GetNameSafe(BossDataAsset));
		CompleteBossPreparation(false);
		return;
	}

	const bool IsSpawnSuccess = SpawnBossWithClass(LoadedBossClass, BossDataAsset);
	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss spawn completed. BossRowName=%s BossClass=%s Success=%s"),
		*BossRowName.ToString(),
		*GetNameSafe(LoadedBossClass),
		IsSpawnSuccess ? TEXT("true") : TEXT("false"));

	if (IsSpawnSuccess)
	{
		RegisterBossDeadEvent();
	}

	CompleteBossPreparation(IsSpawnSuccess);
}

bool APBBossSpawner::IsLoadedBossDataReady() const
{
	return IsBossDataLoaded;
}

APBBossBase* APBBossSpawner::GetSpawnedBoss() const
{
	return SpawnedBoss;
}

void APBBossSpawner::SetBossRowName(const FName NewBossRowName)
{
	BossRowName = NewBossRowName;
}

bool APBBossSpawner::RequestBossDataAsync(FStreamableDelegate OnLoaded)
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

	TArray<FPrimaryAssetId> BossAssetIds;
	BossAssetIds.Add(FPrimaryAssetId(PBBossAssetIds::Type::BossData, BossRowName));

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
		*MakeBossDataBundleKey().ToString());

	IsBossDataLoaded = false;
	IsBossAssetsUnloaded = false;
	PendingBossDataLoadedDelegate = OnLoaded;

	PendingBossDataLoadRequestId = CachedGameDataLoadSubsystem->LoadPrimaryAssetsByIdsAsync(
		BossAssetIds,
		BossBundleNames,
		FStreamableDelegate::CreateUObject(this, &APBBossSpawner::HandleBossDataLoadCompleted));

	if (!PendingBossDataLoadRequestId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load failed to start. BossRowName=%s"),
			*BossRowName.ToString());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load request created. BossRowName=%s RequestId=%s"),
		*BossRowName.ToString(),
		*PendingBossDataLoadRequestId.ToString());

	return true;
}

void APBBossSpawner::HandleBossDataLoadCompleted()
{
	PendingBossDataLoadRequestId.Invalidate();

	const FPrimaryAssetId BossAssetId(PBBossAssetIds::Type::BossData, BossRowName);
	const UPBBossDataAsset* BossDataAsset = IsValid(CachedGameDataLoadSubsystem)
		? Cast<UPBBossDataAsset>(CachedGameDataLoadSubsystem->GetLoadedPrimaryAsset(BossAssetId))
		: nullptr;

	if (!IsValid(BossDataAsset)
		|| !IsValid(BossDataAsset->BossClass.Get())
		|| !AreBossPatternClassesLoaded(BossDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Boss async load completed with failure. BossRowName=%s Asset=%s BossClass=%s HasPatterns=%s HasSubsystem=%s"),
			*BossRowName.ToString(),
			*GetNameSafe(BossDataAsset),
			IsValid(BossDataAsset) ? *GetNameSafe(BossDataAsset->BossClass.Get()) : TEXT("None"),
			AreBossPatternClassesLoaded(BossDataAsset) ? TEXT("true") : TEXT("false"),
			IsValid(CachedGameDataLoadSubsystem) ? TEXT("true") : TEXT("false"));
		IsBossDataLoaded = false;
		PendingBossDataLoadedDelegate.ExecuteIfBound();
		PendingBossDataLoadedDelegate.Unbind();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Boss async load completed. BossRowName=%s Bundle=%s"),
		*BossRowName.ToString(),
		*MakeBossDataBundleKey().ToString());

	IsBossDataLoaded = true;
	PendingBossDataLoadedDelegate.ExecuteIfBound();
	PendingBossDataLoadedDelegate.Unbind();
}

bool APBBossSpawner::SpawnBossWithClass(const TSubclassOf<APBBossBase> BossClassToSpawn, const UPBBossDataAsset* BossDataAsset)
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

void APBBossSpawner::ClearSpawnedBoss()
{
	if (IsValid(SpawnedBoss))
	{
		SpawnedBoss->Destroy();
		SpawnedBoss = nullptr;
	}
}

void APBBossSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterBossDeadEvent();
	ClearSpawnedBoss();

	Super::EndPlay(EndPlayReason);
}

void APBBossSpawner::RegisterBossDeadEvent()
{
	if (!UGameplayMessageSubsystem::HasInstance(this) || BossDeadListenerHandle.IsValid())
	{
		return;
	}

	BossDeadListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattleBossDeadMessage>(
		GameplayTags::Event_Battle_Boss_Dead,
		this,
		&APBBossSpawner::HandleBossDeadMessage);
}

void APBBossSpawner::UnregisterBossDeadEvent()
{
	if (BossDeadListenerHandle.IsValid())
	{
		BossDeadListenerHandle.Unregister();
	}
}

void APBBossSpawner::HandleBossDeadMessage(FGameplayTag Channel, const FPBBattleBossDeadMessage& Message)
{
	static_cast<void>(Channel);

	if (Message.BossActor != SpawnedBoss)
	{
		return;
	}

	UnloadBossAssets();
	UnregisterBossDeadEvent();
}

void APBBossSpawner::UnloadBossAssets()
{
	if (IsBossAssetsUnloaded || !IsValid(CachedGameDataLoadSubsystem))
	{
		return;
	}

	CachedGameDataLoadSubsystem->UnloadPrimaryAssetBundle(MakeBossDataBundleKey());
	IsBossAssetsUnloaded = true;
}

void APBBossSpawner::CompleteBossPreparation(const bool IsSuccess) const
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
