// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperAssetLoader.h"

#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"

void UPBBumperAssetLoader::Initialize(UPBGameDataLoadSubsystem* InOwnerSubsystem)
{
	OwnerSubsystem = InOwnerSubsystem;
}

void UPBBumperAssetLoader::LoadBumperAssetsAsync(
	const TArray<FName>& BumperRowIds,
	const TArray<FName>& BundleNames)
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	PendingBumperRowIds = BumperRowIds;
	PendingBundleNames = BundleNames;

	if (!OwnerSubsystem->IsStartupGameDataReady())
	{
		OwnerSubsystem->OnStartupGameDataLoaded.AddUniqueDynamic(
			this,
			&UPBBumperAssetLoader::ResumeBumperAssetLoadAfterStartupDataReady);
		return;
	}

	TArray<FPrimaryAssetId> BumperAssetIds;
	BumperAssetIds.Reserve(PendingBumperRowIds.Num());
	for (const FName& BumperRowId : PendingBumperRowIds)
	{
		if (!BumperRowId.IsNone())
		{
			BumperAssetIds.Add(FPrimaryAssetId(PBBumperAssetIds::Type::BumperData, BumperRowId));
		}
	}

	if (BumperAssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperAssetLoad] No bumper row ids."));
		OwnerSubsystem->CompletePrimaryAssetLoad(false);
		return;
	}

	OwnerSubsystem->LoadPrimaryAssetsByIdsAsync(
		BumperAssetIds,
		PendingBundleNames,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBBumperAssetLoader::HandleBumperPrimaryAssetLoadCompleted,
			BumperAssetIds));
}

void UPBBumperAssetLoader::ResumeBumperAssetLoadAfterStartupDataReady()
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	OwnerSubsystem->OnStartupGameDataLoaded.RemoveDynamic(
		this,
		&UPBBumperAssetLoader::ResumeBumperAssetLoadAfterStartupDataReady);

	LoadBumperAssetsAsync(PendingBumperRowIds, PendingBundleNames);
}

void UPBBumperAssetLoader::HandleBumperPrimaryAssetLoadCompleted(TArray<FPrimaryAssetId> LoadedAssetIds)
{
	if (!IsValid(OwnerSubsystem))
	{
		return;
	}

	OwnerSubsystem->CachePrimaryAssetsFromManager(LoadedAssetIds);
	OwnerSubsystem->CompletePrimaryAssetLoad(OwnerSubsystem->GetLoadedPrimaryAssets().Num() > 0);
}
