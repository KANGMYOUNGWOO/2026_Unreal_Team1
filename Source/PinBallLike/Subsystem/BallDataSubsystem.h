// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BallDataStruct.h"
#include "BallDataSubsystem.generated.h"

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
};
