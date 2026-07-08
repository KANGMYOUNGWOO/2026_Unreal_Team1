// Fill out your copyright notice in the Description page of Project Settings.


#include "PBCombatPartyActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Component/PBSnakeFormationComponent.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Struct/Ball/PBBallInstanceData.h"
#include "PinBallLike/Struct/Deck/PBDeckOwnedBallData.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckAssetLoadService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"

static void AppendResourceData(TArray<FPBResourceData>& OutResources, const TMap<FName, int32>& ResourceValues)
{
	for (const TPair<FName, int32>& ResourceValue : ResourceValues)
	{
		if (!ResourceValue.Key.IsNone())
		{
			OutResources.Add(FPBResourceData(
				ResourceValue.Key,
				static_cast<float>(ResourceValue.Value),
				static_cast<float>(ResourceValue.Value),
				0.0f));
		}
	}
}

static void AppendStatData(TArray<FPBStatData>& OutStats, const TMap<FName, int32>& StatValues)
{
	for (const TPair<FName, int32>& StatValue : StatValues)
	{
		if (!StatValue.Key.IsNone())
		{
			OutStats.Add(FPBStatData(StatValue.Key, StatValue.Value));
		}
	}
}

APBCombatPartyActor::APBCombatPartyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	LauncherVisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LauncherVisual"));
	SetRootComponent(LauncherVisualComponent);
	LauncherVisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LauncherVisualComponent->SetGenerateOverlapEvents(false);
	LauncherVisualComponent->SetSimulatePhysics(false);
	LauncherVisualComponent->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		LauncherVisualComponent->SetStaticMesh(CubeMesh.Object);
		LauncherVisualComponent->SetRelativeScale3D(FVector(0.5f));
	}

	SnakeFormationComponent = CreateDefaultSubobject<UPBSnakeFormationComponent>(TEXT("SnakeFormationComponent"));
}

void APBCombatPartyActor::InitializeFromDeck()
{
	RefreshFromDeck();
}

void APBCombatPartyActor::BindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.AddDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.AddDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsReordered);
	DeckSubsystem->OnDeploymentSlotsRotated.AddDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsRotated);
}

void APBCombatPartyActor::UnbindDeckEvents()
{
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.RemoveDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsReordered);
	DeckSubsystem->OnDeploymentSlotsRotated.RemoveDynamic(this, &APBCombatPartyActor::HandleDeploymentSlotsRotated);
}

void APBCombatPartyActor::HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId)
{
	(void)SlotIndex;
	(void)BallInstanceId;
	RefreshFromDeck();
}

void APBCombatPartyActor::HandleDeploymentSlotsReordered()
{
	RefreshFromDeck();
}

void APBCombatPartyActor::HandleDeploymentSlotsRotated()
{
	RefreshFromDeck();
}

void APBCombatPartyActor::BeginPlay()
{
	Super::BeginPlay();

	LauncherBaseLocation = GetActorLocation();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		DeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>();
	}

	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("PBCombatPartyActor could not find PBBallDeckSubsystem."));
		return;
	}

	InitializeFromDeck();
	BindDeckEvents();
	SetLauncherActive(bLauncherActive);
}

void APBCombatPartyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateLauncherMotion(DeltaTime);
}

void APBCombatPartyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearPartyRoles();
	UnbindDeckEvents();
	DestroyPartyBalls();
	Super::EndPlay(EndPlayReason);
}

void APBCombatPartyActor::RefreshFromDeck()
{
	ClearPartyRoles();
	DestroyPartyBalls();

	if (!DeckSubsystem)
	{
		PartyBalls.Reset();
		LeaderBall = nullptr;
		FollowerBalls.Reset();
		if (SnakeFormationComponent)
		{
			SnakeFormationComponent->ClearFormation();
		}
		return;
	}

	PartyBalls.Reset();
	for (const int32 BallInstanceId : DeckSubsystem->GetDeploymentBallInstanceIds())
	{
		if (APBBallBase* Ball = SpawnBallFromInstanceId(BallInstanceId))
		{
			PartyBalls.Add(Ball);
		}
	}

	LeaderBall = PartyBalls.IsValidIndex(0) ? PartyBalls[0] : nullptr;

	FollowerBalls.Reset();
	for (int32 BallIndex = 1; BallIndex < PartyBalls.Num(); ++BallIndex)
	{
		FollowerBalls.Add(PartyBalls[BallIndex]);
	}

	HandlePartyOrderChanged();
}

void APBCombatPartyActor::HandlePartyOrderChanged()
{
	TArray<APBBallBase*> OrderedBalls;
	OrderedBalls.Reserve(PartyBalls.Num());
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (IsValid(Ball.Get()))
		{
			OrderedBalls.Add(Ball.Get());
		}
	}

	if (SnakeFormationComponent)
	{
		SnakeFormationComponent->SetPartyBalls(OrderedBalls);
		SnakeFormationComponent->SetComponentTickEnabled(!bLauncherActive);
	}

	if (bLauncherActive)
	{
		HidePartyBallsForLaunchReady();
	}
	else
	{
		ApplyPartyRoles();
	}
}

void APBCombatPartyActor::UpdateLauncherMotion(const float DeltaTime)
{
	if (!bLauncherActive)
	{
		return;
	}

	LauncherElapsedTime += FMath::Max(DeltaTime, 0.0f);
	
	FVector MoveAxis = GetActorTransform().TransformVectorNoScale(LauncherMoveLocalAxis);
	MoveAxis.Z = 0.0f;
	if (!MoveAxis.Normalize())
	{
		MoveAxis = FVector::RightVector;
	}
	
	const float MoveOffset = FMath::Sin(LauncherElapsedTime * LauncherMoveSpeed * 2.0f * PI) * LauncherMoveHalfRange;
	SetActorLocation(LauncherBaseLocation + MoveAxis * MoveOffset);
}

void APBCombatPartyActor::HidePartyBallsForLaunchReady()
{
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (!IsValid(Ball.Get()))
		{
			continue;
		}

		Ball->SetCombatRole(EPBBallPartyRole::None);
		Ball->SetActorHiddenInGame(true);
	}
}

void APBCombatPartyActor::SpawnPartyBallsAtLauncher()
{
	const int32 BallCount = PartyBalls.Num();
	UE_LOG(LogTemp, Warning, TEXT("SpawnPartyBallsAtLauncher called. Party=%s BallCount=%d Location=%s"),
		*GetNameSafe(this),
		BallCount,
		*GetActorLocation().ToString());

	if (BallCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnPartyBallsAtLauncher skipped because PartyBalls is empty."));
		return;
	}

	FVector SpacingAxis = GetActorRightVector();
	SpacingAxis.Z = 0.0f;
	if (!SpacingAxis.Normalize())
	{
		SpacingAxis = FVector::RightVector;
	}

	const float CenterIndex = static_cast<float>(BallCount - 1) * 0.5f;
	for (int32 BallIndex = 0; BallIndex < BallCount; ++BallIndex)
	{
		APBBallBase* Ball = PartyBalls[BallIndex].Get();
		if (!IsValid(Ball))
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnPartyBallsAtLauncher skipped invalid ball at index %d."), BallIndex);
			continue;
		}

		const float Offset = (static_cast<float>(BallIndex) - CenterIndex) * ReadyBallSpacing;
		Ball->SetActorLocation(GetActorLocation() + SpacingAxis * Offset, false, nullptr, ETeleportType::TeleportPhysics);
		Ball->SetActorHiddenInGame(false);
		UE_LOG(LogTemp, Warning, TEXT("Spawned party ball at launcher. Index=%d Ball=%s Location=%s"),
			BallIndex,
			*GetNameSafe(Ball),
			*Ball->GetActorLocation().ToString());
	}
}

bool APBCombatPartyActor::LaunchPartyFromReadyPosition()
{
	if (bLaunchConsumed)
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("LaunchPartyFromReadyPosition called. Party=%s bLauncherActive=%s bLaunchConsumed=%s Leader=%s PartyBallCount=%d"),
		*GetNameSafe(this),
		bLauncherActive ? TEXT("true") : TEXT("false"),
		bLaunchConsumed ? TEXT("true") : TEXT("false"),
		*GetNameSafe(LeaderBall.Get()),
		PartyBalls.Num());

	if (!IsValid(LeaderBall.Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("LaunchPartyFromReadyPosition failed: LeaderBall is invalid."));
		return false;
	}

	bLaunchConsumed = true;
	SetLauncherActive(false);
	SpawnPartyBallsAtLauncher();
	HandlePartyOrderChanged();

	UE_LOG(LogTemp, Warning, TEXT("LaunchPartyFromReadyPosition succeeded. Party balls placed at launcher."));
	return true;
}

void APBCombatPartyActor::SetLauncherActive(const bool bNewLauncherActive)
{
	bLauncherActive = bNewLauncherActive;
	SetActorTickEnabled(bLauncherActive);
	UE_LOG(LogTemp, Warning, TEXT("SetLauncherActive called. Party=%s bLauncherActive=%s"),
		*GetNameSafe(this),
		bLauncherActive ? TEXT("true") : TEXT("false"));

	if (LauncherVisualComponent)
	{
		LauncherVisualComponent->SetHiddenInGame(!bLauncherActive);
		LauncherVisualComponent->SetVisibility(bLauncherActive);
	}

	if (SnakeFormationComponent)
	{
		SnakeFormationComponent->SetComponentTickEnabled(!bLauncherActive);
	}

	if (bLauncherActive)
	{
		LauncherBaseLocation = GetActorLocation();
		HidePartyBallsForLaunchReady();
	}
}

void APBCombatPartyActor::ApplyPartyRoles()
{
	if (LeaderBall)
	{
		LeaderBall->SetCombatRole(EPBBallPartyRole::Leader);
	}

	for (const TObjectPtr<APBBallBase>& FollowerBall : FollowerBalls)
	{
		if (FollowerBall)
		{
			FollowerBall->SetCombatRole(EPBBallPartyRole::Follower);
		}
	}
}

void APBCombatPartyActor::ClearPartyRoles()
{
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (Ball)
		{
			Ball->SetCombatRole(EPBBallPartyRole::None);
		}
	}
}

void APBCombatPartyActor::CompactPartyBalls()
{
	PartyBalls.RemoveAll([](const TObjectPtr<APBBallBase>& Ball)
	{
		return Ball == nullptr;
	});

	FollowerBalls.RemoveAll([](const TObjectPtr<APBBallBase>& Ball)
	{
		return Ball == nullptr;
	});
}

void APBCombatPartyActor::DestroyPartyBalls()
{
	for (const TObjectPtr<APBBallBase>& Ball : PartyBalls)
	{
		if (IsValid(Ball.Get()))
		{
			Ball->Destroy();
		}
	}

	PartyBalls.Reset();
	LeaderBall = nullptr;
	FollowerBalls.Reset();
}

APBBallBase* APBCombatPartyActor::SpawnBallFromInstanceId(int32 BallInstanceId)
{
	if (!DeckSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. DeckSubsystem is null. BallInstanceId=%d"),
			BallInstanceId);
		return nullptr;
	}

	const FPBDeckOwnedBallData* BallInstanceData = DeckSubsystem->GetOwnedBallData(BallInstanceId);
	if (!BallInstanceData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. Missing owned ball data. BallInstanceId=%d"),
			BallInstanceId);
		return nullptr;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
	const UPBBallDeckAssetLoadService* AssetLoadService = DeckSubsystem->GetAssetLoadService();
	if (!TableDataSubsystem || !AssetLoadService)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. Missing subsystem. BallInstanceId=%d BallId=%s TableData=%s AssetLoadService=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			TableDataSubsystem ? TEXT("valid") : TEXT("null"),
			AssetLoadService ? TEXT("valid") : TEXT("null"));
		return nullptr;
	}

	FPBBallTableRow BallRow;
	if (BallInstanceData->BallId.IsNone() || !TableDataSubsystem->FindBallRow(BallInstanceData->BallId, BallRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. Ball row not found. BallInstanceId=%d BallId=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString());
		return nullptr;
	}

	FName StarLevelRowName;
	FPBBallStarLevelRow StarLevelRow;
	if (!TableDataSubsystem->FindBallStarLevelRow(
		BallInstanceData->BallId,
		BallInstanceData->StarLevel,
		StarLevelRowName,
		StarLevelRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. Star level row not found. BallInstanceId=%d BallId=%s StarLevel=%d"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			BallInstanceData->StarLevel);
		return nullptr;
	}

	UClass* BallActorClass = AssetLoadService->GetLoadedBallActorClass(BallInstanceId);
	if (!IsValid(BallActorClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. Ball actor class is not loaded. BallInstanceId=%d BallId=%s StarLevel=%d"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			BallInstanceData->StarLevel);
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. World is null. BallInstanceId=%d BallId=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString());
		return nullptr;
	}

	const FTransform SpawnTransform(GetActorRotation(), GetActorLocation());
	APBBallBase* SpawnedBall = World->SpawnActorDeferred<APBBallBase>(
		BallActorClass,
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!SpawnedBall)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatParty] SpawnBall failed. SpawnActorDeferred returned null. BallInstanceId=%d BallId=%s ActorClass=%s"),
			BallInstanceId,
			*BallInstanceData->BallId.ToString(),
			*GetNameSafe(BallActorClass));
		return nullptr;
	}

	FPBBallInstanceData NewBallInstanceData;
	NewBallInstanceData.InstanceId = BallInstanceData->InstanceId;
	NewBallInstanceData.BallId = BallInstanceData->BallId;
	NewBallInstanceData.StarLevel = BallInstanceData->StarLevel;
	AppendResourceData(NewBallInstanceData.BaseResources, StarLevelRow.BaseResources);
	AppendStatData(NewBallInstanceData.BaseStats, StarLevelRow.BaseStats);

	SpawnedBall->InitializeFromBallInstanceData(NewBallInstanceData);
	SpawnedBall->FinishSpawning(SpawnTransform);
	SpawnedBall->SetActorHiddenInGame(true);

	return SpawnedBall;
}
