#include "PBCollectionGameEventSubsystem.h"

#include "PBCollectionSubsystem.h"
#include "PinBallLike/Struct/Deck/PBDeckOwnedBallData.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"

namespace
{
const FName BallSourceTableName(TEXT("DT_Ball"));
}

void UPBCollectionGameEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UPBCollectionSubsystem::StaticClass());
	Collection.InitializeDependency(UPBBallDeckSubsystem::StaticClass());

	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return;
	}

	CollectionSubsystem = GameInstance->GetSubsystem<UPBCollectionSubsystem>();
	BallDeckSubsystem = GameInstance->GetSubsystem<UPBBallDeckSubsystem>();

	if (IsValid(CollectionSubsystem))
	{
		CollectionSubsystem->OnCollectionDataReady.AddUniqueDynamic(
			this,
			&UPBCollectionGameEventSubsystem::HandleCollectionDataReady);
	}

	if (IsValid(BallDeckSubsystem))
	{
		BallDeckSubsystem->OnDeploymentSlotChanged.AddUniqueDynamic(
			this,
			&UPBCollectionGameEventSubsystem::HandleDeploymentSlotChanged);
		BallDeckSubsystem->OnBenchSlotChanged.AddUniqueDynamic(
			this,
			&UPBCollectionGameEventSubsystem::HandleBenchSlotChanged);
	}

	SynchronizeCurrentBallOwnership();
}

void UPBCollectionGameEventSubsystem::Deinitialize()
{
	if (IsValid(CollectionSubsystem))
	{
		CollectionSubsystem->OnCollectionDataReady.RemoveDynamic(
			this,
			&UPBCollectionGameEventSubsystem::HandleCollectionDataReady);
	}

	if (IsValid(BallDeckSubsystem))
	{
		BallDeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(
			this,
			&UPBCollectionGameEventSubsystem::HandleDeploymentSlotChanged);
		BallDeckSubsystem->OnBenchSlotChanged.RemoveDynamic(
			this,
			&UPBCollectionGameEventSubsystem::HandleBenchSlotChanged);
	}

	BallDeckSubsystem = nullptr;
	CollectionSubsystem = nullptr;
	PendingBallIds.Reset();

	Super::Deinitialize();
}

void UPBCollectionGameEventSubsystem::SynchronizeCurrentBallOwnership()
{
	if (!IsValid(CollectionSubsystem)
		|| !CollectionSubsystem->IsDataReady()
		|| !IsValid(BallDeckSubsystem))
	{
		return;
	}

	for (const int32 BallInstanceId : BallDeckSubsystem->GetAllPlacedBallInstanceIds())
	{
		UnlockBallByInstanceId(BallInstanceId);
	}
}

void UPBCollectionGameEventSubsystem::HandleDeploymentSlotChanged(
	const int32 SlotIndex,
	const int32 BallInstanceId)
{
	(void)SlotIndex;
	UnlockBallByInstanceId(BallInstanceId);
}

void UPBCollectionGameEventSubsystem::HandleBenchSlotChanged(
	const int32 SlotIndex,
	const int32 BallInstanceId)
{
	(void)SlotIndex;
	UnlockBallByInstanceId(BallInstanceId);
}

void UPBCollectionGameEventSubsystem::HandleCollectionDataReady(const bool bIsReady)
{
	if (!bIsReady)
	{
		return;
	}

	const TArray<FName> BallIdsToProcess = PendingBallIds.Array();
	PendingBallIds.Reset();
	for (const FName BallId : BallIdsToProcess)
	{
		UnlockBallBySourceId(BallId);
	}

	SynchronizeCurrentBallOwnership();
}

void UPBCollectionGameEventSubsystem::UnlockBallByInstanceId(const int32 BallInstanceId)
{
	if (BallInstanceId == INDEX_NONE || !IsValid(BallDeckSubsystem))
	{
		return;
	}

	const FPBDeckOwnedBallData* OwnedBallData = BallDeckSubsystem->GetOwnedBallData(BallInstanceId);
	if (!OwnedBallData || !OwnedBallData->IsValid())
	{
		return;
	}

	UnlockBallBySourceId(OwnedBallData->BallId);
}

void UPBCollectionGameEventSubsystem::UnlockBallBySourceId(const FName BallId)
{
	if (BallId.IsNone() || !IsValid(CollectionSubsystem))
	{
		return;
	}
	if (!CollectionSubsystem->IsDataReady())
	{
		PendingBallIds.Add(BallId);
		return;
	}

	PendingBallIds.Remove(BallId);

	TSet<FName> CollectionIds;
	CollectionIds.Append(CollectionSubsystem->FindCollectionIdsBySourceId(BallId));
	CollectionIds.Append(CollectionSubsystem->FindCollectionIdsBySourceRow(BallSourceTableName, BallId));

	for (const FName CollectionId : CollectionIds)
	{
		FPBCollectionDisplayData PreviousDisplayData;
		const bool bWasBeforeUnlocked = CollectionSubsystem->GetDisplayEntry(
			CollectionId,
			PreviousDisplayData)
			&& static_cast<uint8>(PreviousDisplayData.State)
				>= static_cast<uint8>(EPBCollectionState::Unlocked);

		if (CollectionSubsystem->UnlockEntry(CollectionId) && !bWasBeforeUnlocked)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[CollectionIntegration] Ball ownership unlocked collection entry. BallId=%s CollectionId=%s"),
				*BallId.ToString(),
				*CollectionId.ToString());
		}
	}
}
