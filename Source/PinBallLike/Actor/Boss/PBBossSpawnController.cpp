// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBossSpawnController.h"

#include "Components/SceneComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"

APBBossSpawnController::APBBossSpawnController()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void APBBossSpawnController::BeginPlay()
{
	Super::BeginPlay();

	if (IsSpawnOnBeginPlay)
	{
		SpawnBossAsync();
	}
}

void APBBossSpawnController::SpawnBossAsync()
{
	ClearSpawnedBoss();

	UWorld* World = GetWorld();
	if (!IsValid(World) || !BossClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossSpawn] Cannot spawn boss. BossClass=%s"),
			*GetNameSafe(BossClass.Get()));
		CompleteBossPreparation(false);
		return;
	}

	const FTransform SpawnTransform(GetActorRotation(), GetActorLocation());

	SpawnedBoss = World->SpawnActorDeferred<APBBossBase>(
		BossClass,
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!IsValid(SpawnedBoss))
	{
		CompleteBossPreparation(false);
		return;
	}

	UGameplayStatics::FinishSpawningActor(SpawnedBoss, SpawnTransform);

	UE_LOG(LogTemp, Log, TEXT("[BossSpawn] Spawned boss. Boss=%s"),
		*GetNameSafe(SpawnedBoss.Get()));

	CompleteBossPreparation(true);
}

void APBBossSpawnController::ClearSpawnedBoss()
{
	if (IsValid(SpawnedBoss))
	{
		SpawnedBoss->Destroy();
		SpawnedBoss = nullptr;
	}
}

void APBBossSpawnController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearSpawnedBoss();

	Super::EndPlay(EndPlayReason);
}

void APBBossSpawnController::CompleteBossPreparation(const bool IsSuccess) const
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePreparationCompletedMessage Message;
	Message.PreparationType = EPBBattlePreparationType::Boss;
	Message.bSuccess = IsSuccess;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Phase_Prepare_Completed,
		Message);
}
