// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCheatManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Party/PBCombatPartyActor.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"

void UPBCheatManager::AddBenchBall(FName BallId)
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
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall failed: BallId=%s. Check BallId and empty deck slots."), *BallId.ToString());
		return;
	}

	const FGuid UIRequestId = DeckSubsystem->LoadPlacedBallUIAssetsAsync(FStreamableDelegate());
	const FGuid GameplayRequestId = DeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate::CreateUObject(
		this,
		&UPBCheatManager::HandleCheatPlacedBallGameplayAssetsLoaded));

	UE_LOG(LogTemp, Log, TEXT("AddBenchBall succeeded: BallId=%s UIRequestId=%s GameplayRequestId=%s."),
		*BallId.ToString(),
		*UIRequestId.ToString(),
		*GameplayRequestId.ToString());
}

void UPBCheatManager::HandleCheatPlacedBallGameplayAssetsLoaded()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall asset reload finished but World is invalid."));
		return;
	}

	APBCombatPartyActor* CombatPartyActor = Cast<APBCombatPartyActor>(
		UGameplayStatics::GetActorOfClass(World, APBCombatPartyActor::StaticClass()));
	if (!CombatPartyActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall asset reload finished but CombatPartyActor is missing."));
		return;
	}

	CombatPartyActor->InitializeFromDeck();
	UE_LOG(LogTemp, Log, TEXT("AddBenchBall refreshed CombatPartyActor after asset reload."));
}
