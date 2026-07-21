// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBallDeckAssetLoadService.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "PBBallDeckSubsystem.h"
#include "PinBallLike/Struct/Deck/PBDeckOwnedBallData.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"

namespace
{
	const FName BallGameplayBundleKey(TEXT("BallGameplay"));
	const FName BallUIBundleKey(TEXT("BallUI"));
}

void UPBBallDeckAssetLoadService::Initialize(UPBBallDeckSubsystem* InDeckSubsystem)
{
	DeckSubsystem = InDeckSubsystem;
}

FGuid UPBBallDeckAssetLoadService::LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate OnLoaded)
{
	return LoadPlacedBallAssetsAsync({ PBAssetBundleNames::Gameplay, BallGameplayBundleKey }, OnLoaded);
}

FGuid UPBBallDeckAssetLoadService::LoadPlacedBallUIAssetsAsync(FStreamableDelegate OnLoaded)
{
	return LoadPlacedBallAssetsAsync({ PBAssetBundleNames::UI, BallUIBundleKey }, OnLoaded);
}

FGuid UPBBallDeckAssetLoadService::LoadOwnedBallIconAsync(const FName BallId, FStreamableDelegate OnLoaded)
{
	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Owned icon load skipped. DeckSubsystem is null. BallId=%s"),
			*BallId.ToString());
		OnLoaded.ExecuteIfBound();
		return FGuid();
	}

	if (BallId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Owned icon load skipped. BallId is none."));
		OnLoaded.ExecuteIfBound();
		return FGuid();
	}

	UGameInstance* GameInstance = DeckSubsystem->GetGameInstance();
	UPBGameDataLoadSubsystem* GameDataLoadSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>() : nullptr;
	if (!GameDataLoadSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Owned icon load skipped. GameDataLoadSubsystem is null. BallId=%s"),
			*BallId.ToString());
		OnLoaded.ExecuteIfBound();
		return FGuid();
	}

	FPrimaryAssetId BallAssetId;
	if (!ResolveBallDataAssetId(BallId, BallAssetId))
	{
		BallAssetId = FPrimaryAssetId(PBBallAssetIds::Type::BallData, BallId);
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Owned icon asset id is not registered. BallId=%s ExpectedAssetId=%s"),
			*BallId.ToString(),
			*BallAssetId.ToString());
	}

	const FName OwnedIconBundleKey = MakeOwnedBallIconBundleKey(BallId);
	const FGuid RequestId = GameDataLoadSubsystem->LoadPrimaryAssetsByIdsAsync(
		{ BallAssetId },
		{ PBAssetBundleNames::UI, OwnedIconBundleKey },
		OnLoaded);

	UE_LOG(LogTemp, Log, TEXT("[BallDeckAssetLoad] Owned icon load requested. RequestId=%s BallId=%s AssetId=%s Bundle=%s"),
		*RequestId.ToString(),
		*BallId.ToString(),
		*BallAssetId.ToString(),
		*OwnedIconBundleKey.ToString());
	return RequestId;
}

void UPBBallDeckAssetLoadService::UnloadGameplayAssets()
{
	UnloadPlacedBallAssets({ PBAssetBundleNames::Gameplay, BallGameplayBundleKey });
}

void UPBBallDeckAssetLoadService::UnloadUIAssets()
{
	UnloadPlacedBallAssets({ PBAssetBundleNames::UI, BallUIBundleKey });
}

void UPBBallDeckAssetLoadService::UnloadOwnedBallIcon(const FName BallId)
{
	if (BallId.IsNone())
	{
		return;
	}

	if (DeckSubsystem)
	{
		if (UGameInstance* GameInstance = DeckSubsystem->GetGameInstance())
		{
			if (UPBGameDataLoadSubsystem* GameDataLoadSubsystem = GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>())
			{
				GameDataLoadSubsystem->UnloadPrimaryAssetBundle(MakeOwnedBallIconBundleKey(BallId));
			}
		}
	}
}

const UPBBallDataAsset* UPBBallDeckAssetLoadService::GetLoadedBallDataAsset(const int32 BallInstanceId) const
{
	FName BallId;
	if (!FindBallId(BallInstanceId, BallId))
	{
		return nullptr;
	}

	return GetLoadedBallDataAssetById(BallId);
}

UTexture2D* UPBBallDeckAssetLoadService::GetLoadedBallIcon(const int32 BallInstanceId) const
{
	const UPBBallDataAsset* BallDataAsset = GetLoadedBallDataAsset(BallInstanceId);
	return BallDataAsset ? BallDataAsset->BallIcon.Get() : nullptr;
}

FGuid UPBBallDeckAssetLoadService::LoadPlacedBallAssetsAsync(const TArray<FName>& BundleNames, FStreamableDelegate OnLoaded)
{
	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Load skipped. DeckSubsystem is null. Bundle=%s"),
			*MakeBundleKey(BundleNames).ToString());
		OnLoaded.ExecuteIfBound();
		return FGuid();
	}

	UGameInstance* GameInstance = DeckSubsystem->GetGameInstance();
	UPBGameDataLoadSubsystem* GameDataLoadSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>() : nullptr;
	if (!GameDataLoadSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Load skipped. GameDataLoadSubsystem is null. Bundle=%s"),
			*MakeBundleKey(BundleNames).ToString());
		OnLoaded.ExecuteIfBound();
		return FGuid();
	}

	TArray<FPrimaryAssetId> AssetIds;
	if (!BuildPlacedBallAssetIds(AssetIds))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Load skipped. No placed ball asset ids. Bundle=%s PlacedCount=%d"),
			*MakeBundleKey(BundleNames).ToString(),
			DeckSubsystem->GetAllPlacedBallInstanceIds().Num());
		OnLoaded.ExecuteIfBound();
		return FGuid();
	}

	const FGuid RequestId = GameDataLoadSubsystem->LoadPrimaryAssetsByIdsAsync(AssetIds, BundleNames, OnLoaded);
	FString AssetIdText;
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (!AssetIdText.IsEmpty())
		{
			AssetIdText.Append(TEXT(", "));
		}
		AssetIdText.Append(AssetId.ToString());
	}

	UE_LOG(LogTemp, Log, TEXT("[BallDeckAssetLoad] Load requested. RequestId=%s Bundle=%s AssetIds=[%s]"),
		*RequestId.ToString(),
		*MakeBundleKey(BundleNames).ToString(),
		*AssetIdText);
	return RequestId;
}

void UPBBallDeckAssetLoadService::UnloadPlacedBallAssets(const TArray<FName>& BundleNames)
{
	if (!DeckSubsystem)
	{
		return;
	}

	if (UGameInstance* GameInstance = DeckSubsystem->GetGameInstance())
	{
		if (UPBGameDataLoadSubsystem* GameDataLoadSubsystem = GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>())
		{
			GameDataLoadSubsystem->UnloadPrimaryAssetBundle(MakeBundleKey(BundleNames));
		}
	}
}

bool UPBBallDeckAssetLoadService::BuildPlacedBallAssetIds(TArray<FPrimaryAssetId>& OutAssetIds) const
{
	OutAssetIds.Reset();
	if (!DeckSubsystem)
	{
		return false;
	}

	for (const int32 BallInstanceId : DeckSubsystem->GetAllPlacedBallInstanceIds())
	{
		FName BallId;
		if (FindBallId(BallInstanceId, BallId))
		{
			FPrimaryAssetId BallAssetId;
			if (ResolveBallDataAssetId(BallId, BallAssetId))
			{
				OutAssetIds.AddUnique(BallAssetId);
				UE_LOG(LogTemp, Log, TEXT("[BallDeckAssetLoad] Resolved placed ball asset. BallInstanceId=%d BallId=%s AssetId=%s"),
					BallInstanceId,
					*BallId.ToString(),
					*BallAssetId.ToString());
			}
			else
			{
				const FPrimaryAssetId ExpectedAssetId(PBBallAssetIds::Type::BallData, BallId);
				OutAssetIds.AddUnique(ExpectedAssetId);
				UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] BallData asset id is not registered. BallInstanceId=%d BallId=%s ExpectedAssetId=%s"),
					BallInstanceId,
					*BallId.ToString(),
					*ExpectedAssetId.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Failed to find ball row. BallInstanceId=%d"),
				BallInstanceId);
		}
	}

	return !OutAssetIds.IsEmpty();
}

FName UPBBallDeckAssetLoadService::MakeBundleKey(const TArray<FName>& BundleNames) const
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

FName UPBBallDeckAssetLoadService::MakeOwnedBallIconBundleKey(const FName BallId) const
{
	return BallId.IsNone()
		? NAME_None
		: FName(*FString::Printf(TEXT("OwnedBallIcon_%s"), *BallId.ToString()));
}

bool UPBBallDeckAssetLoadService::ResolveBallDataAssetId(const FName BallId, FPrimaryAssetId& OutAssetId) const
{
	OutAssetId = FPrimaryAssetId();
	if (BallId.IsNone())
	{
		return false;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	const FPrimaryAssetId ExactAssetId(PBBallAssetIds::Type::BallData, BallId);
	FAssetData ExactAssetData;
	if (AssetManager.GetPrimaryAssetData(ExactAssetId, ExactAssetData))
	{
		OutAssetId = ExactAssetId;
		return true;
	}

	return false;
}

const UPBBallDataAsset* UPBBallDeckAssetLoadService::GetLoadedBallDataAssetById(const FName BallId) const
{
	if (!DeckSubsystem || BallId.IsNone())
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = DeckSubsystem->GetGameInstance();
	const UPBGameDataLoadSubsystem* GameDataLoadSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>() : nullptr;
	if (!GameDataLoadSubsystem)
	{
		return nullptr;
	}

	FPrimaryAssetId BallAssetId;
	if (!ResolveBallDataAssetId(BallId, BallAssetId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Failed to resolve loaded BallData asset id. BallId=%s"),
			*BallId.ToString());
		return nullptr;
	}

	const UPBBallDataAsset* BallDataAsset = Cast<UPBBallDataAsset>(GameDataLoadSubsystem->GetLoadedPrimaryAsset(BallAssetId));
	if (!BallDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] BallData asset is not loaded yet. BallId=%s AssetId=%s"),
			*BallId.ToString(),
			*BallAssetId.ToString());
	}

	return BallDataAsset;
}

bool UPBBallDeckAssetLoadService::FindBallId(const int32 BallInstanceId, FName& OutBallId) const
{
	OutBallId = NAME_None;
	if (!DeckSubsystem)
	{
		return false;
	}

	const FPBDeckOwnedBallData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData || !BallInstanceData->IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Invalid owned ball data. BallInstanceId=%d HasData=%s"),
			BallInstanceId,
			BallInstanceData ? TEXT("true") : TEXT("false"));
		return false;
	}

	OutBallId = BallInstanceData->BallId;
	if (OutBallId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Missing BallId. BallInstanceId=%d"),
			BallInstanceId);
		return false;
	}

	const UGameInstance* GameInstance = DeckSubsystem->GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	if (!TableDataSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Missing TableDataSubsystem. BallInstanceId=%d BallId=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString());
		return false;
	}

	FPBBallTableRow BallRow;
	const bool bFoundBallRow = TableDataSubsystem->FindBallRow(OutBallId, BallRow);
	if (!bFoundBallRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallDeckAssetLoad] Ball table row not found. BallInstanceId=%d BallId=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString());
	}
	return bFoundBallRow;
}
