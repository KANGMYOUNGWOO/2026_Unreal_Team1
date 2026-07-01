// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BallDataStruct.h"
#include "BallDataSubsystem.generated.h"

class UPBBallDataAsset;

/**
 * 
 */
UCLASS()
class PINBALLLIKE_API UBallDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public :
void InitializeData();

private:
	UPROPERTY()
	TObjectPtr<UDataTable> BallDataTable;
	
	TMap<int32, const FBallDataStruct*> BallDataMap;
	TArray<const FBallDataStruct*> BallDataArray;
	
	
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	
public :
	TArray<int32> GetRandomBalls(int32 Count);
	
	FText GetBallName(int32 BallId);

    FText GetBallSynergeny(int32 BallId);	
	
	const FBallDataStruct* GetBallData(int32 BallId);
	
#pragma region Ball
	
public:
	void InitializeBallData();
	const UPBBallDataAsset* GetBallDataAsset(int32 BallId) const;
	TArray<const UPBBallDataAsset*> GetAllBallDataAssets() const;

private:
	UPROPERTY()
	TArray<TObjectPtr<UPBBallDataAsset>> BallDataAssets;

	TMap<int32, UPBBallDataAsset*> BallDataAssetMap;
	
#pragma endregion
};
