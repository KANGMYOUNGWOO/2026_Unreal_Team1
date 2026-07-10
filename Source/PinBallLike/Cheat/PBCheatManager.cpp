// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCheatManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"

namespace
{
FName GetCheatSceneMapPath(const int32 SceneIndex)
{
	switch (SceneIndex)
	{
	case 1:
		return FName(TEXT("/Game/Map/Lv_MainMenu"));
	case 2:
		return FName(TEXT("/Game/Map/Lv_Battle"));
	case 3:
		return FName(TEXT("/Game/Map/Lv_Shop"));
	default:
		return NAME_None;
	}
}
}


void UPBCheatManager::AddDefaultBall()
{
	//AddBenchBall(PBBallAssetIds::Ball::Test01);
	AddBenchBall("Ball_Test01");
}

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

void UPBCheatManager::GoScene(const int32 SceneIndex)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] GoScene failed. World is invalid."));
		return;
	}

	const FName MapPath = GetCheatSceneMapPath(SceneIndex);
	if (MapPath.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] GoScene failed. Unknown scene index=%d. Use 1=MainMenu, 2=Battle, 3=Shop."),
			SceneIndex);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Open scene. Index=%d Map=%s"),
		SceneIndex,
		*MapPath.ToString());
	UGameplayStatics::OpenLevel(World, MapPath);
}

void UPBCheatManager::GoMainMenu()
{
	GoScene(1);
}

void UPBCheatManager::GoBattle()
{
	GoScene(2);
}

void UPBCheatManager::GoShop()
{
	GoScene(3);
}

void UPBCheatManager::PrintAsyncLoadState() const
{
	UPBGameDataLoadSubsystem* GameDataLoadSubsystem = GetGameDataLoadSubsystem();
	if (!GameDataLoadSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] PrintAsyncLoadState failed. GameDataLoadSubsystem is invalid."));
		return;
	}

	const TMap<FPrimaryAssetId, TObjectPtr<UObject>>& LoadedPrimaryAssets = GameDataLoadSubsystem->GetLoadedPrimaryAssets();
	UE_LOG(LogTemp, Log, TEXT("[Cheat] AsyncLoadState. PrimaryAssetsReady=%s CachedPrimaryAssets=%d"),
		GameDataLoadSubsystem->IsPrimaryAssetsReady() ? TEXT("true") : TEXT("false"),
		LoadedPrimaryAssets.Num());

	for (const TPair<FPrimaryAssetId, TObjectPtr<UObject>>& LoadedAssetPair : LoadedPrimaryAssets)
	{
		UE_LOG(LogTemp, Log, TEXT("[Cheat] CachedPrimaryAsset. AssetId=%s Asset=%s"),
			*LoadedAssetPair.Key.ToString(),
			*GetNameSafe(LoadedAssetPair.Value.Get()));
	}
}

void UPBCheatManager::DamageBoss(const int32 DamageAmount)
{
	if (DamageAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. DamageAmount must be greater than 0."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. World is invalid."));
		return;
	}

	APBBossBase* Boss = Cast<APBBossBase>(UGameplayStatics::GetActorOfClass(World, APBBossBase::StaticClass()));
	if (!Boss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. Boss is missing."));
		return;
	}

	UPBBossStatComponent* BossStatComponent = Boss->GetBossStatComponent();
	if (!BossStatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBoss failed. BossStatComponent is invalid."));
		return;
	}

	BossStatComponent->ApplyBossDamage(NAME_None, DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] DamageBoss succeeded. Damage=%d"), DamageAmount);
}

UGameInstance* UPBCheatManager::GetCheatGameInstance() const
{
	UWorld* World = GetWorld();
	return World ? World->GetGameInstance() : nullptr;
}

UPBBallDeckSubsystem* UPBCheatManager::GetBallDeckSubsystem() const
{
	UGameInstance* GameInstance = GetCheatGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
}

UPBGameDataLoadSubsystem* UPBCheatManager::GetGameDataLoadSubsystem() const
{
	UGameInstance* GameInstance = GetCheatGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>() : nullptr;
}

void UPBCheatManager::RequestPlacedBallAssetReload()
{
	UPBBallDeckSubsystem* DeckSubsystem = GetBallDeckSubsystem();
	if (!DeckSubsystem)
	{
		return;
	}

	// 덱 변경 후 UI/전투용 볼 에셋을 다시 준비한다.
	const FGuid UIRequestId = DeckSubsystem->LoadPlacedBallUIAssetsAsync(FStreamableDelegate());
	const FGuid GameplayRequestId = DeckSubsystem->LoadPlacedBallGameplayAssetsAsync(FStreamableDelegate::CreateUObject(
		this,
		&UPBCheatManager::HandleCheatPlacedBallGameplayAssetsLoaded));

	UE_LOG(LogTemp, Log, TEXT("[Cheat] Reload placed ball assets. UIRequestId=%s GameplayRequestId=%s"),
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

	APBCombatPartyController* CombatPartyActor = Cast<APBCombatPartyController>(
		UGameplayStatics::GetActorOfClass(World, APBCombatPartyController::StaticClass()));
	if (!CombatPartyActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBenchBall asset reload finished but CombatPartyActor is missing."));
		return;
	}

	CombatPartyActor->InitializeFromDeck();
	UE_LOG(LogTemp, Log, TEXT("AddBenchBall refreshed CombatPartyActor after asset reload."));
}
