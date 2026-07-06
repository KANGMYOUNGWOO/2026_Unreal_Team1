// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBumperDataLoadTestActor.generated.h"

class UPBGameDataLoadSubsystem;
class UPBTableDataSubsystem;
struct FPBBumperTableRow;

UCLASS()
class PINBALLLIKE_API APBBumperDataLoadTestActor : public AActor
{
	GENERATED_BODY()

public:
	APBBumperDataLoadTestActor();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Test")
	void LoadBumperAssetsForTest();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Test")
	void UnloadBumperAssetsForTest();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleStartupGameDataLoaded();

	UFUNCTION()
	void HandleBumperAssetsLoaded();

	bool FindTestBumperRow(FPBBumperTableRow& OutRow) const;
	void PrintCurrentTableState() const;
	void PrintCurrentBumperAssetState() const;

	UPROPERTY(EditAnywhere, Category = "Bumper|Test")
	FName TestBumperRowName = FName(TEXT("BasicCombo"));

	UPROPERTY(EditAnywhere, Category = "Bumper|Test")
	bool bLoadBumperAssetsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Bumper|Test")
	TArray<FName> BumperAssetBundleNames;

	UPROPERTY()
	TObjectPtr<UPBGameDataLoadSubsystem> GameDataLoadSubsystem;

	UPROPERTY()
	TObjectPtr<UPBTableDataSubsystem> TableDataSubsystem;
};
