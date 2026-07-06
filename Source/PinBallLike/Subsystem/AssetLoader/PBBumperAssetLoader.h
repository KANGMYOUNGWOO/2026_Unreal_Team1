// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/PrimaryAssetId.h"
#include "PBBumperAssetLoader.generated.h"

class UPBGameDataLoadSubsystem;

UCLASS()
class PINBALLLIKE_API UPBBumperAssetLoader : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UPBGameDataLoadSubsystem* InOwnerSubsystem);

	// 장착된 범퍼를 PlayerData에서 읽어 로드한다.
	void LoadEquippedBumpersAsync();

	// 지정한 RowId 목록으로 범퍼를 로드한다.
	void LoadBumpersByRowIdsAsync(const TArray<FName>& BumperRowIds);

private:
	UFUNCTION()
	void ResumeEquippedBumperLoadAfterStartupDataReady();

	void HandlePrimaryAssetLoadCompleted(TArray<FPrimaryAssetId> LoadedAssetIds);
	void LoadReferencedClassesAsync();
	void HandleReferencedClassLoadCompleted();

	UPROPERTY()
	TObjectPtr<UPBGameDataLoadSubsystem> OwnerSubsystem;
};
