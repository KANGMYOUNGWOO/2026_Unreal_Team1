// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBumperDataLoadTestActor.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"

APBBumperDataLoadTestActor::APBBumperDataLoadTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BumperAssetBundleNames.Add(TEXT("Gameplay"));
	BumperAssetBundleNames.Add(TEXT("UI"));
}

void APBBumperDataLoadTestActor::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperDataLoadTest] GameInstance is invalid."));
		return;
	}

	GameDataLoadSubsystem = GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>();
	TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>();

	if (!IsValid(GameDataLoadSubsystem) || !IsValid(TableDataSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperDataLoadTest] Required subsystem is invalid. LoadSubsystem=%s TableSubsystem=%s"),
			*GetNameSafe(GameDataLoadSubsystem),
			*GetNameSafe(TableDataSubsystem));
		return;
	}

	GameDataLoadSubsystem->OnStartupGameDataLoaded.AddUniqueDynamic(
		this,
		&APBBumperDataLoadTestActor::HandleStartupGameDataLoaded);
	GameDataLoadSubsystem->OnPrimaryAssetsLoaded.AddUniqueDynamic(
		this,
		&APBBumperDataLoadTestActor::HandleBumperAssetsLoaded);

	PrintCurrentTableState();

	if (bLoadBumperAssetsOnBeginPlay)
	{
		LoadBumperAssetsForTest();
	}
}

void APBBumperDataLoadTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GameDataLoadSubsystem))
	{
		GameDataLoadSubsystem->OnStartupGameDataLoaded.RemoveDynamic(
			this,
			&APBBumperDataLoadTestActor::HandleStartupGameDataLoaded);
		GameDataLoadSubsystem->OnPrimaryAssetsLoaded.RemoveDynamic(
			this,
			&APBBumperDataLoadTestActor::HandleBumperAssetsLoaded);
	}

	Super::EndPlay(EndPlayReason);
}

void APBBumperDataLoadTestActor::HandleStartupGameDataLoaded()
{
	PrintCurrentTableState();
}

void APBBumperDataLoadTestActor::HandleBumperAssetsLoaded()
{
	PrintCurrentBumperAssetState();
}

void APBBumperDataLoadTestActor::LoadBumperAssetsForTest()
{
	if (!IsValid(GameDataLoadSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperDataLoadTest] Load failed. GameDataLoadSubsystem is invalid."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BumperDataLoadTest] Request BumperData primary asset load. BundleCount=%d"),
		BumperAssetBundleNames.Num());

	GameDataLoadSubsystem->LoadPrimaryAssetTypeAsync(PBBumperAssetIds::Type::BumperData, BumperAssetBundleNames);
}

void APBBumperDataLoadTestActor::UnloadBumperAssetsForTest()
{
	if (!IsValid(GameDataLoadSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperDataLoadTest] Unload failed. GameDataLoadSubsystem is invalid."));
		return;
	}

	GameDataLoadSubsystem->UnloadPrimaryAssets();

	UE_LOG(LogTemp, Log, TEXT("[BumperDataLoadTest] Bumper assets unloaded."));
	PrintCurrentBumperAssetState();
}

bool APBBumperDataLoadTestActor::FindTestBumperRow(FPBBumperTableRow& OutRow) const
{
	return IsValid(TableDataSubsystem)
		&& TableDataSubsystem->FindBumperRow(TestBumperRowName, OutRow);
}

void APBBumperDataLoadTestActor::PrintCurrentTableState() const
{
	if (!IsValid(GameDataLoadSubsystem) || !IsValid(TableDataSubsystem))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BumperDataLoadTest] StartupReady=%s TableReady=%s TestRow=%s"),
		GameDataLoadSubsystem->IsStartupGameDataReady() ? TEXT("true") : TEXT("false"),
		TableDataSubsystem->IsTableDataReady() ? TEXT("true") : TEXT("false"),
		*TestBumperRowName.ToString());

	FPBBumperTableRow BumperRow;
	if (!GameDataLoadSubsystem->IsStartupGameDataReady() || !TableDataSubsystem->IsTableDataReady())
	{
		UE_LOG(LogTemp, Log, TEXT("[BumperDataLoadTest] Waiting for startup table load."));
		return;
	}

	if (!FindTestBumperRow(BumperRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BumperDataLoadTest] Bumper row not found. RowName=%s"),
			*TestBumperRowName.ToString());
		return;
	}

	FPBBumperTriggerRow TriggerRow;
	const bool bFoundTriggerRow = TableDataSubsystem->FindLinkedBumperTriggerRow(TestBumperRowName, TriggerRow);

	FPBBumperEffectRow EffectRow;
	const bool bFoundEffectRow = TableDataSubsystem->FindLinkedBumperEffectRow(TestBumperRowName, EffectRow);

	UE_LOG(LogTemp, Log, TEXT("[BumperDataLoadTest] BumperRow found. TriggerID=%s TriggerFound=%s EffectID=%s EffectFound=%s BumperDataAsset=%s"),
		*BumperRow.TriggerID.ToString(),
		bFoundTriggerRow ? TEXT("true") : TEXT("false"),
		*BumperRow.EffectID.ToString(),
		bFoundEffectRow ? TEXT("true") : TEXT("false"),
		*BumperRow.BumperDataAsset.ToSoftObjectPath().ToString());
}

void APBBumperDataLoadTestActor::PrintCurrentBumperAssetState() const
{
	if (!IsValid(GameDataLoadSubsystem))
	{
		return;
	}

	FPBBumperTableRow BumperRow;
	const bool bFoundBumperRow = FindTestBumperRow(BumperRow);

	const FPrimaryAssetId TestAssetId(PBBumperAssetIds::Type::BumperData, TestBumperRowName);
	UObject* LoadedAsset = GameDataLoadSubsystem->GetLoadedPrimaryAsset(TestAssetId);
	const UPBBumperDataAsset* BumperDataAsset = Cast<UPBBumperDataAsset>(LoadedAsset);
	const UClass* TriggerClass = BumperDataAsset ? BumperDataAsset->TriggerClass.Get() : nullptr;
	const UClass* EffectClass = BumperDataAsset ? BumperDataAsset->EffectClass.Get() : nullptr;
	const FString RowAssetPath = bFoundBumperRow
		? BumperRow.BumperDataAsset.ToSoftObjectPath().ToString()
		: FString(TEXT("None"));
	const FString IconPath = BumperDataAsset
		? BumperDataAsset->Icon.ToSoftObjectPath().ToString()
		: FString(TEXT("None"));

	UE_LOG(LogTemp, Log, TEXT("[BumperDataLoadTest] BumperDataReady=%s PrimaryAsset=%s RowAsset=%s LoadedAsset=%s Icon=%s TriggerClass=%s EffectClass=%s"),
		GameDataLoadSubsystem->ArePrimaryAssetsReady() ? TEXT("true") : TEXT("false"),
		*TestAssetId.ToString(),
		*RowAssetPath,
		*GetNameSafe(LoadedAsset),
		*IconPath,
		*GetNameSafe(TriggerClass),
		*GetNameSafe(EffectClass));
}
