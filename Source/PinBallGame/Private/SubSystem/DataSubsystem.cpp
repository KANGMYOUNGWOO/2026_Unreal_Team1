// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/DataSubsystem.h"

void UDataSubsystem::InitializeData()
{
	BallDataMap.Empty();
	
	TArray<FBallDataStruct*> Rows;
	
	BallDataTable = LoadObject<UDataTable>(nullptr,TEXT("/Game/UserAsset/Data/CardData/HeroCard/DT_HeroCard.DT_HeroCard"));
	
	if (!BallDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load HeroCardTable"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Red,
				TEXT("BallTable is nullptr")
			);
		}
		return;
	}
	
	BallDataTable->GetAllRows<FBallDataStruct>(TEXT("GameDataSub"), Rows);

	if (BallDataTable == nullptr) return;

	for (const FBallDataStruct* Row : Rows)
	{
		BallDataMap.Add(Row->BallId,Row);
	}
	
	if (GEngine)
	{
		if(const FBallDataStruct* const* Found = BallDataMap.Find(110001))
		{
			const FBallDataStruct* ballData = *Found;
		
			int32 power =  ballData->BallId;
		
			GEngine->AddOnScreenDebugMessage(
			-1,          // 고유 키 (새 메시지로 계속 띄우려면 -1)
			5.f,         // 출력 시간 (초)
			FColor::Red, // 글자 색상
			 FString ::Printf(TEXT("ID : %d"),power)
			);
		}
	
	
	
	
	}
	
	
	
}
