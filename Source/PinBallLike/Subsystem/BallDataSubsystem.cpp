// Fill out your copyright notice in the Description page of Project Settings.


#include "BallDataSubsystem.h"
#include "Algo/RandomShuffle.h"


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
