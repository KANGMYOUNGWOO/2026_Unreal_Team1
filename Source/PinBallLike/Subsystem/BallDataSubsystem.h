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
	
	TMap<FName, const FBallDataStruct*> BallDataMap;
	TArray<const FBallDataStruct*> BallDataArray;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	
public :
	TArray<FName> GetRandomBalls(int32 Count);
	
	FText GetBallName(FName BallId);

    FText GetBallSynergeny(FName BallId);	
	
	const FBallDataStruct* GetBallData(FName BallId);
	
};
