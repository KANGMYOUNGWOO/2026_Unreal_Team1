// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperAssetLoader.h"

#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Utils/PBSubsystemUtils.h"

void UPBBumperAssetLoader::Initialize(UPBGameDataLoadSubsystem* InOwnerSubsystem)
{
	OwnerSubsystem = InOwnerSubsystem;
}

void UPBBumperAssetLoader::LoadEquippedBumpersAsync()
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	if (!OwnerSubsystem->IsStartupGameDataReady())
	{
		OwnerSubsystem->OnStartupGameDataLoaded.AddUniqueDynamic(
			this,
			&UPBBumperAssetLoader::ResumeEquippedBumperLoadAfterStartupDataReady);
		return;
	}

	const UPBPlayerDataSubsystem* PlayerDataSubsystem =
		PBSubsystemUtils::GetGameInstanceSubsystem<UPBPlayerDataSubsystem>(OwnerSubsystem);
	if (!IsValid(PlayerDataSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperAssetLoad] Missing PlayerDataSubsystem."));
		OwnerSubsystem->CompletePrimaryAssetLoad(false);
		return;
	}

	LoadBumpersByRowIdsAsync(PlayerDataSubsystem->GetEquippedBumperRowIds());
}

void UPBBumperAssetLoader::ResumeEquippedBumperLoadAfterStartupDataReady()
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	OwnerSubsystem->OnStartupGameDataLoaded.RemoveDynamic(
		this,
		&UPBBumperAssetLoader::ResumeEquippedBumperLoadAfterStartupDataReady);

	LoadEquippedBumpersAsync();
}

void UPBBumperAssetLoader::LoadBumpersByRowIdsAsync(const TArray<FName>& BumperRowIds)
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	TArray<FPrimaryAssetId> BumperAssetIds;
	BumperAssetIds.Reserve(BumperRowIds.Num());
	for (const FName& BumperRowId : BumperRowIds)
	{
		if (!BumperRowId.IsNone())
		{
			BumperAssetIds.Add(FPrimaryAssetId(PBBumperAssetIds::Type::BumperData, BumperRowId));
		}
	}

	if (BumperAssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperAssetLoad] No equipped bumper row ids."));
		OwnerSubsystem->CompletePrimaryAssetLoad(false);
		return;
	}

	TArray<FName> BundleNames;
	BundleNames.Add(TEXT("Gameplay"));

	// 범퍼 PrimaryDataAsset을 먼저 로드하고 내부 class 참조를 이어서 로드한다.
	OwnerSubsystem->LoadPrimaryAssetsByIdsAsync(
		BumperAssetIds,
		BundleNames,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBBumperAssetLoader::HandlePrimaryAssetLoadCompleted,
			BumperAssetIds));
}

void UPBBumperAssetLoader::HandlePrimaryAssetLoadCompleted(TArray<FPrimaryAssetId> LoadedAssetIds)
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	OwnerSubsystem->CachePrimaryAssetsFromManager(LoadedAssetIds);
	LoadReferencedClassesAsync();
}

void UPBBumperAssetLoader::LoadReferencedClassesAsync()
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	TArray<FSoftObjectPath> SoftReferencePaths;
	for (const TPair<FPrimaryAssetId, TObjectPtr<UObject>>& LoadedPrimaryAsset : OwnerSubsystem->GetLoadedPrimaryAssets())
	{
		const UPBBumperDataAsset* BumperDataAsset = Cast<UPBBumperDataAsset>(LoadedPrimaryAsset.Value.Get());
		if (!IsValid(BumperDataAsset))
		{
			continue;
		}

		const FSoftObjectPath TriggerClassPath = BumperDataAsset->TriggerClass.ToSoftObjectPath();
		if (TriggerClassPath.IsValid())
		{
			SoftReferencePaths.AddUnique(TriggerClassPath);
		}

		const FSoftObjectPath EffectClassPath = BumperDataAsset->EffectClass.ToSoftObjectPath();
		if (EffectClassPath.IsValid())
		{
			SoftReferencePaths.AddUnique(EffectClassPath);
		}
	}

	OwnerSubsystem->LoadSoftReferencesAsync(
		SoftReferencePaths,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBBumperAssetLoader::HandleReferencedClassLoadCompleted));
}

void UPBBumperAssetLoader::HandleReferencedClassLoadCompleted()
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	OwnerSubsystem->CompletePrimaryAssetLoad(OwnerSubsystem->GetLoadedPrimaryAssets().Num() > 0);
}
