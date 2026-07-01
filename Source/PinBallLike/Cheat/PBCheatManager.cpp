// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCheatManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/Subsystem/PBBallDeckSubsystem.h"

void UPBCheatManager::AddBenchBall(int32 BallId)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: World is invalid."));
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: GameInstance is invalid."));
		return;
	}

	UPBBallDeckSubsystem* DeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>();
	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: BallDeckSubsystem is invalid."));
		return;
	}

	if (!DeckSubsystem->AddNewBallToDeck(BallId))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: BallId=%d. Check BallId and empty deck slots."), BallId);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AddBenchBall succeeded: BallId=%d."), BallId);
}
