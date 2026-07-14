#include "PBBallSkillComponent.h"

#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Skill/PBBallSkillActorBase.h"
#include "PinBallLike/Struct/Ball/PBBallInstanceData.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckAssetLoadService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Utils/PBSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UPBBallSkillComponent::UPBBallSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPBBallSkillComponent::InitializeSkill(const FPBBallInstanceData& BallInstanceData)
{
	SkillActorClass = nullptr;
	SkillData = FPBBallSkillTableRow();

	if (!BallInstanceData.IsValid())
	{
		return false;
	}

	const UPBBallDeckSubsystem* DeckSubsystem =
		PBSubsystemUtils::GetGameInstanceSubsystem<UPBBallDeckSubsystem>(this);
	const UPBTableDataSubsystem* TableDataSubsystem =
		PBSubsystemUtils::GetGameInstanceSubsystem<UPBTableDataSubsystem>(this);
	const UPBBallDeckAssetLoadService* AssetLoadService =
		DeckSubsystem ? DeckSubsystem->GetAssetLoadService() : nullptr;
	const UPBBallDataAsset* BallDataAsset = AssetLoadService
		? AssetLoadService->GetLoadedBallDataAsset(BallInstanceData.InstanceId)
		: nullptr;
	const TSubclassOf<APBBallSkillActorBase> FoundSkillActorClass = BallDataAsset
		? BallDataAsset->SkillActorClass.Get()
		: nullptr;

	FPBBallSkillTableRow FoundSkillData;
	if (!FoundSkillActorClass
		|| !TableDataSubsystem
		|| !TableDataSubsystem->FindDefaultSkillRowForBall(BallInstanceData.BallId, FoundSkillData))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[BallSkill] Initialization failed. BallInstanceId=%d BallId=%s"),
			BallInstanceData.InstanceId,
			*BallInstanceData.BallId.ToString());
		return false;
	}

	SkillActorClass = FoundSkillActorClass;
	SkillData = MoveTemp(FoundSkillData);
	return true;
}

bool UPBBallSkillComponent::TryActivateSkill()
{
	return ActivateSkillActor() != nullptr;
}

APBBallSkillActorBase* UPBBallSkillComponent::ActivateSkillActor()
{
	if (IsValid(ActiveSkillActor))
	{
		return ActiveSkillActor;
	}

	APBBallBase* OwnerBall = Cast<APBBallBase>(GetOwner());
	if (!OwnerBall || !OwnerBall->GetWorld() || !SkillActorClass)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(
		OwnerBall->GetActorRotation(),
		OwnerBall->GetActorLocation());
	ActiveSkillActor = OwnerBall->GetWorld()->SpawnActorDeferred<APBBallSkillActorBase>(
		SkillActorClass,
		SpawnTransform,
		OwnerBall,
		OwnerBall->GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (ActiveSkillActor)
	{
		ActiveSkillActor->InitializeSkill(
			OwnerBall,
			SkillData);
		ActiveSkillActor->OnDestroyed.AddUniqueDynamic(
			this,
			&UPBBallSkillComponent::HandleActiveSkillActorDestroyed);
		UGameplayStatics::FinishSpawningActor(ActiveSkillActor, SpawnTransform);
	}

	return ActiveSkillActor;
}

void UPBBallSkillComponent::HandleActiveSkillActorDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == ActiveSkillActor)
	{
		ActiveSkillActor = nullptr;
	}
}
