// Fill out your copyright notice in the Description page of Project Settings.


#include "BallDataSubsystem.h"
#include "Algo/RandomShuffle.h"

void UBallDataSubsystem::InitializeData()
{
	BallDataMap.Empty();
	
	TArray<FBallDataStruct*> Rows;
	
	BallDataTable = LoadObject<UDataTable>(nullptr, TEXT(""));
	
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
	/*
	if (GEngine)
	{
		if (const FBallDataStruct* const* Found = BallDataMap.Find(110001))
		{
			const FBallDataStruct* BallData = *Found;
			
			
		}
	}
	*/
	
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
