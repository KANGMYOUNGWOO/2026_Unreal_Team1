// Fill out your copyright notice in the Description page of Project Settings.


#include "BallDataSubsystem.h"
#include "Algo/RandomShuffle.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "Modules/ModuleManager.h"
#include "PinBallLike/DataAsset/Ball/BPBallDataAsset.h"


void UBallDataSubsystem::InitializeData()
{
	BallDataMap.Empty();
	
	TArray<FBallDataStruct*> Rows;
	
	BallDataTable = LoadObject<UDataTable>(nullptr,  TEXT("/Game/Blueprints/Shop/Data/DT_BallData.DT_BallData"));
	
	if (!BallDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Load BallDataTable"));
		return;
	}
	
	BallDataTable->GetAllRows<FBallDataStruct>(TEXT("GameDataSub"),Rows);
	
	if (BallDataTable == nullptr)
	{
		return;
	}
	
	for (const FBallDataStruct* Row : Rows)
	{
		BallDataMap.Add(Row->BallId,Row); 
		BallDataArray.Add(Row);
	}
	
	if (GEngine)
	{
		if (const FBallDataStruct* const* Found = BallDataMap.Find(11001))
		{
			const FBallDataStruct* BallData = *Found;
			
			UE_LOG(LogTemp, Warning, TEXT("BallData"));
		}
	}
	
	

	
}

const FBallDataStruct* UBallDataSubsystem::GetBallData(int32 BallId)
{
	if (const FBallDataStruct* const* Found = BallDataMap.Find(BallId))
	{
		return *Found;
	}
	
	else return nullptr;
}

void UBallDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeData();
	InitializeBallData();
}

FText UBallDataSubsystem::GetBallName(int32 BallId)
{
	const FBallDataStruct* Data = GetBallData(BallId);
	
	if (!Data)
	{
		return FText::GetEmpty();
	}
	
	
	
	return FText::FromStringTable(
	  TEXT("STBallText"),
	   Data->NameKey.ToString());
}

FText UBallDataSubsystem::GetBallSynergeny(int32 BallId)
{
	const FBallDataStruct* Data = GetBallData(BallId);
	
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to GetBallSynergeny"));
		return FText::GetEmpty();
	}
	 
	
	
	return FText::FromStringTable(
	  TEXT("STBallText"),
	   Data->SynergyKey.ToString());
	
	
	
}

TArray<int32> UBallDataSubsystem::GetRandomBalls(int32 Count)
{
	if (BallDataArray.IsEmpty())
	{
		return {};
	}
	
	TArray<const FBallDataStruct*> TempArray = BallDataArray;
	TArray<int32> ResultArray;
	Algo::RandomShuffle(TempArray);
	
	
	const int32 PickCount = FMath::Min(Count, TempArray.Num());
	ResultArray.Reserve(PickCount);

	
	for (int i=0;i<PickCount;i++)
	{
		ResultArray.Add(TempArray[i]->BallId);
	}
	
	return ResultArray;
}

#pragma region Ball

void UBallDataSubsystem::InitializeBallData()
{
	BallDataAssets.Empty();
	BallDataAssetMap.Empty();

	const UAssetManagerSettings* AssetManagerSettings = GetDefault<UAssetManagerSettings>();
	if (AssetManagerSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem AssetManagerSettings. PrimaryAssetTypesToScan=%d"),
			AssetManagerSettings->PrimaryAssetTypesToScan.Num());

		for (const FPrimaryAssetTypeInfo& TypeInfo : AssetManagerSettings->PrimaryAssetTypesToScan)
		{
			FString DirectoriesText;
			for (const FDirectoryPath& Directory : TypeInfo.GetDirectories())
			{
				if (!DirectoriesText.IsEmpty())
				{
					DirectoriesText += TEXT(",");
				}
				DirectoriesText += Directory.Path;
			}

			UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem AssetManager TypeInfo. Type=%s BaseClass=%s bHasBlueprintClasses=%s Directories=%s"),
				*TypeInfo.PrimaryAssetType.ToString(),
				*TypeInfo.GetAssetBaseClass().ToString(),
				TypeInfo.bHasBlueprintClasses ? TEXT("true") : TEXT("false"),
				*DirectoriesText);
		}
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	bool bRequestedBallDataScan = false;
	if (AssetManagerSettings)
	{
		for (const FPrimaryAssetTypeInfo& TypeInfo : AssetManagerSettings->PrimaryAssetTypesToScan)
		{
			if (TypeInfo.PrimaryAssetType != UPBBallDataAsset::BallDataAssetType)
			{
				continue;
			}

			TArray<FString> ScanPaths;
			ScanPaths.Reserve(TypeInfo.GetDirectories().Num());
			for (const FDirectoryPath& Directory : TypeInfo.GetDirectories())
			{
				if (!Directory.Path.IsEmpty())
				{
					ScanPaths.Add(Directory.Path);
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem requesting AssetManager scan. Type=%s PathCount=%d bHasBlueprintClasses=%s"),
				*TypeInfo.PrimaryAssetType.ToString(),
				ScanPaths.Num(),
				TypeInfo.bHasBlueprintClasses ? TEXT("true") : TEXT("false"));

			if (!ScanPaths.IsEmpty())
			{
				AssetManager.ScanPathsForPrimaryAssets(
					UPBBallDataAsset::BallDataAssetType,
					ScanPaths,
					UPBBallDataAsset::StaticClass(),
					TypeInfo.bHasBlueprintClasses,
					false,
					true);
				bRequestedBallDataScan = true;
			}
			break;
		}
	}

	if (!bRequestedBallDataScan)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem did not request BallData scan. BallData TypeInfo was not found or had no directories."));
	}

	TArray<FPrimaryAssetId> BallAssetIds;
	AssetManager.GetPrimaryAssetIdList(UPBBallDataAsset::BallDataAssetType, BallAssetIds);

	UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem InitializeBallData. PrimaryAssetType=%s AssetIdCount=%d"),
		*UPBBallDataAsset::BallDataAssetType.ToString(),
		BallAssetIds.Num());

	if (BallAssetIds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem found no BallData primary assets from AssetManager. Trying AssetRegistry fallback using AssetManager settings."));
	}

	for (const FPrimaryAssetId& BallAssetId : BallAssetIds)
	{
		const FSoftObjectPath BallAssetPath = AssetManager.GetPrimaryAssetPath(BallAssetId);
		UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem loading BallDataAsset. PrimaryAssetId=%s Path=%s"),
			*BallAssetId.ToString(),
			*BallAssetPath.ToString());

		UPBBallDataAsset* BallDataAsset = Cast<UPBBallDataAsset>(BallAssetPath.TryLoad());
		if (!BallDataAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem failed to load BallDataAsset. PrimaryAssetId=%s Path=%s"),
				*BallAssetId.ToString(),
				*BallAssetPath.ToString());
			continue;
		}

		if (BallDataAsset->BallId == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("BallDataAsset %s has invalid BallId 0."), *GetNameSafe(BallDataAsset));
			continue;
		}

		if (BallDataAssetMap.Contains(BallDataAsset->BallId))
		{
			UE_LOG(LogTemp, Warning, TEXT("Duplicate BallDataAsset BallId %d. Asset=%s"), BallDataAsset->BallId, *GetNameSafe(BallDataAsset));
			continue;
		}

		BallDataAssets.Add(BallDataAsset);
		BallDataAssetMap.Add(BallDataAsset->BallId, BallDataAsset);

		UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem registered BallDataAsset. BallId=%d Asset=%s Icon=%s"),
			BallDataAsset->BallId,
			*GetNameSafe(BallDataAsset),
			*GetNameSafe(BallDataAsset->Icon));
	}

	if (BallDataAssetMap.IsEmpty() && AssetManagerSettings)
	{
		const FPrimaryAssetTypeInfo* BallDataTypeInfo = nullptr;
		for (const FPrimaryAssetTypeInfo& TypeInfo : AssetManagerSettings->PrimaryAssetTypesToScan)
		{
			if (TypeInfo.PrimaryAssetType == UPBBallDataAsset::BallDataAssetType)
			{
				BallDataTypeInfo = &TypeInfo;
				break;
			}
		}

		if (BallDataTypeInfo)
		{
			TArray<FString> ScanPaths;
			TArray<FName> PackagePaths;
			for (const FDirectoryPath& Directory : BallDataTypeInfo->GetDirectories())
			{
				if (Directory.Path.IsEmpty())
				{
					continue;
				}

				ScanPaths.Add(Directory.Path);
				PackagePaths.Add(*Directory.Path);
			}

			UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem AssetRegistry fallback scan. PathCount=%d Class=%s"),
				ScanPaths.Num(),
				*GetNameSafe(UPBBallDataAsset::StaticClass()));

			if (!ScanPaths.IsEmpty())
			{
				FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
				IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
				AssetRegistry.ScanPathsSynchronous(ScanPaths, true);

				FARFilter BallDataFilter;
				BallDataFilter.PackagePaths = PackagePaths;
				BallDataFilter.ClassPaths.Add(UPBBallDataAsset::StaticClass()->GetClassPathName());
				BallDataFilter.bRecursivePaths = true;
				BallDataFilter.bRecursiveClasses = true;

				TArray<FAssetData> BallAssetDataList;
				AssetRegistry.GetAssets(BallDataFilter, BallAssetDataList);

				UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem AssetRegistry fallback result. AssetDataCount=%d"),
					BallAssetDataList.Num());

				for (const FAssetData& BallAssetData : BallAssetDataList)
				{
					UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem fallback loading asset. AssetName=%s ObjectPath=%s Class=%s"),
						*BallAssetData.AssetName.ToString(),
						*BallAssetData.GetObjectPathString(),
						*BallAssetData.AssetClassPath.ToString());

					UPBBallDataAsset* BallDataAsset = Cast<UPBBallDataAsset>(BallAssetData.GetAsset());
					if (!BallDataAsset)
					{
						UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem fallback failed to load/cast asset. ObjectPath=%s"),
							*BallAssetData.GetObjectPathString());
						continue;
					}

					if (BallDataAsset->BallId == 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem fallback skipped invalid BallId 0. Asset=%s"),
							*GetNameSafe(BallDataAsset));
						continue;
					}

					if (BallDataAssetMap.Contains(BallDataAsset->BallId))
					{
						UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem fallback skipped duplicate BallId. BallId=%d Asset=%s"),
							BallDataAsset->BallId,
							*GetNameSafe(BallDataAsset));
						continue;
					}

					BallDataAssets.Add(BallDataAsset);
					BallDataAssetMap.Add(BallDataAsset->BallId, BallDataAsset);

					UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem fallback registered BallDataAsset. BallId=%d Asset=%s Icon=%s"),
						BallDataAsset->BallId,
						*GetNameSafe(BallDataAsset),
						*GetNameSafe(BallDataAsset->Icon));
				}
			}
		}
	}

	if (BallDataAssetMap.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem failed to register any BallDataAsset from AssetManager or AssetRegistry fallback."));
	}

	UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem InitializeBallData finished. RegisteredCount=%d"),
		BallDataAssetMap.Num());
}

const UPBBallDataAsset* UBallDataSubsystem::GetBallDataAsset(int32 BallId) const
{
	if (UPBBallDataAsset* const* Found = BallDataAssetMap.Find(BallId))
	{
		UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem GetBallDataAsset succeeded. BallId=%d Asset=%s Icon=%s"),
			BallId,
			*GetNameSafe(*Found),
			*GetNameSafe((*Found)->Icon));
		return *Found;
	}

	UE_LOG(LogTemp, Warning, TEXT("BallDataSubsystem GetBallDataAsset failed. BallId=%d RegisteredCount=%d"),
		BallId,
		BallDataAssetMap.Num());
	return nullptr;
}

TArray<const UPBBallDataAsset*> UBallDataSubsystem::GetAllBallDataAssets() const
{
	TArray<const UPBBallDataAsset*> Result;
	Result.Reserve(BallDataAssets.Num());

	for (const TObjectPtr<UPBBallDataAsset>& BallDataAsset : BallDataAssets)
	{
		if (BallDataAsset)
		{
			Result.Add(BallDataAsset.Get());
		}
	}

	return Result;
}

#pragma endregion
