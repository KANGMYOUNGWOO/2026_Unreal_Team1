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
	
public :
	TArray<int32> GetRandomBalls(int32 Count);
	
	
};
