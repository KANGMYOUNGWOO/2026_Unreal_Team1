// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCheatManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBoss.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossHand.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"

namespace
{
const FName LeftGolemHandName = TEXT("Left");
const FName RightGolemHandName = TEXT("Right");

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

void UPBCheatManager::DamageGolemHand(const FName HandName, const int32 DamageAmount)
{
	if (DamageAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. DamageAmount must be greater than 0."));
		return;
	}

	EPBGolemBossHandType HandType;
	if (HandName == LeftGolemHandName)
	{
		HandType = EPBGolemBossHandType::Left;
	}
	else if (HandName == RightGolemHandName)
	{
		HandType = EPBGolemBossHandType::Right;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. Unknown HandName=%s. Use Left or Right."),
			*HandName.ToString());
		return;
	}

	UWorld* World = GetWorld();
	APBGolemBoss* GolemBoss = World
		? Cast<APBGolemBoss>(UGameplayStatics::GetActorOfClass(World, APBGolemBoss::StaticClass()))
		: nullptr;
	if (!GolemBoss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. GolemBoss is missing."));
		return;
	}

	APBGolemBossHand* GolemHand = GolemBoss->GetGolemHand(HandType);
	if (!GolemHand)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageGolemHand failed. Hand=%s is missing."), *HandName.ToString());
		return;
	}

	GolemHand->ApplyHandDamage(DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] DamageGolemHand succeeded. Hand=%s Damage=%d HP=%d/%d Available=%s"),
		*HandName.ToString(),
		DamageAmount,
		GolemHand->GetCurrentHandHP(),
		GolemHand->GetMaxHandHP(),
		GolemHand->IsHandAvailable() ? TEXT("true") : TEXT("false"));
}

void UPBCheatManager::AddBossGroggy(const int32 GroggyAmount)
{
	if (GroggyAmount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] AddBossGroggy failed. GroggyAmount must be greater than 0."));
		return;
	}

	UWorld* World = GetWorld();
	APBBossBase* Boss = World
		? Cast<APBBossBase>(UGameplayStatics::GetActorOfClass(World, APBBossBase::StaticClass()))
		: nullptr;
	if (!Boss)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] AddBossGroggy failed. Boss is missing."));
		return;
	}

	UPBBossGroggyComponent* BossGroggyComponent = Boss->GetBossGroggyComponent();
	if (!BossGroggyComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] AddBossGroggy failed. BossGroggyComponent is invalid."));
		return;
	}

	BossGroggyComponent->ApplyGroggyDamage(GroggyAmount);
	UE_LOG(LogTemp, Log, TEXT("[Cheat] AddBossGroggy succeeded. Amount=%d Gauge=%d/%d"),
		GroggyAmount,
		BossGroggyComponent->GroggyGauge,
		BossGroggyComponent->MaxGroggyGauge);
}

void UPBCheatManager::DamageBall()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cheat] DamageBall failed. World is invalid."));
		return;
	}

	constexpr int32 DamageAmount = 10;
	int32 DamagedBallCount = 0;
	for (TActorIterator<APBBallBase> It(World); It; ++It)
	{
		APBBallBase* Ball = *It;
		UPBBaseResourceComponent* ResourceComponent =
			IsValid(Ball) ? Ball->GetResourceComponent() : nullptr;
		if (!ResourceComponent || ResourceComponent->IsDead())
		{
			continue;
		}

		ResourceComponent->TakeDamage(DamageAmount);
		++DamagedBallCount;
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat] DamageBall finished. Damage=%d DamagedBalls=%d"),
		DamageAmount,
		DamagedBallCount);
}

void UPBCheatManager::RegenMana()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	constexpr float ManaAmount = 500.0f;
	int32 RestoredBallCount = 0;
	for (TActorIterator<APBBallBase> It(World); It; ++It)
	{
		UPBBaseResourceComponent* ResourceComponent = It->GetResourceComponent();
		if (!ResourceComponent || !ResourceComponent->HasResource(PBResourceNames::Mana))
		{
			continue;
		}

		ResourceComponent->ApplyResourceDelta(PBResourceNames::Mana, ManaAmount);
		++RestoredBallCount;
	}

	UE_LOG(LogTemp, Log,
		TEXT("[Cheat] RestoreBallMana finished. Mana=%.0f RestoredBalls=%d"),
		ManaAmount,
		RestoredBallCount);
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
