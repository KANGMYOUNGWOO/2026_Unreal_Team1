// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataStruct/BallDataStruct.h"
#include "DataSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PINBALLGAME_API UDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public :
	void InitializeData();

private:
	UPROPERTY()
	TObjectPtr<UDataTable> BallDataTable;
	
	TMap<int32, const FBallDataStruct*> BallDataMap;

};
