#include "PBRelicSubsystem.h"

#include "PinBallLike/Relic/PBRelicCalculator.h"
#include "PinBallLike/Subsystem/PBEffectSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"

void UPBRelicSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameInstance* GameInstance =
		GetGameInstance();

	if (!IsValid(GameInstance))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[RelicSubsystem] GameInstance is invalid."));

		return;
	}

	UPBTableDataSubsystem* TableSubsystem =
		GameInstance->GetSubsystem<UPBTableDataSubsystem>();

	if (!IsValid(TableSubsystem))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[RelicSubsystem] TableDataSubsystem is invalid."));

		return;
	}

	RelicCalculator =
		NewObject<UPBRelicCalculator>(this);

	if (!IsValid(RelicCalculator))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[RelicSubsystem] Failed to create RelicCalculator."));

		return;
	}

	RelicCalculator->Initialize(
		this,
		TableSubsystem);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[RelicSubsystem] Initialized."));
}

void UPBRelicSubsystem::Deinitialize()
{
	OwnedRelicIds.Reset();

	RelicCalculator = nullptr;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[RelicSubsystem] Deinitialized."));

	Super::Deinitialize();
}

bool UPBRelicSubsystem::AcquireRelic(
	const FName RelicId)
{
	if (RelicId.IsNone())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicSubsystem] Acquire failed. RelicId is none."));

		return false;
	}

	if (HasRelic(RelicId))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicSubsystem] Acquire failed. Already owned. RelicId=%s"),
			*RelicId.ToString());

		return false;
	}

	UGameInstance* GameInstance =
		GetGameInstance();

	if (!IsValid(GameInstance))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicSubsystem] Acquire failed. GameInstance is invalid. RelicId=%s"),
			*RelicId.ToString());

		return false;
	}

	UPBTableDataSubsystem* TableSubsystem =
		GameInstance->GetSubsystem<UPBTableDataSubsystem>();

	if (!IsValid(TableSubsystem))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicSubsystem] Acquire failed. TableDataSubsystem is invalid. RelicId=%s"),
			*RelicId.ToString());

		return false;
	}

	FPBRelicTableRow RelicRow;

	if (!TableSubsystem->FindRelicRow(
		RelicId,
		RelicRow))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicSubsystem] Acquire failed. Relic row not found. RelicId=%s"),
			*RelicId.ToString());

		return false;
	}

	OwnedRelicIds.Add(RelicId);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[RelicSubsystem] Relic acquired. RelicId=%s OwnedCount=%d"),
		*RelicId.ToString(),
		OwnedRelicIds.Num());

	OnRelicAcquired.Broadcast(RelicId);
    OnRelicsChanged.Broadcast();
	return true;
}

bool UPBRelicSubsystem::RemoveRelic(
	const FName RelicId)
{
	if (RelicId.IsNone())
	{
		return false;
	}

	const int32 RemovedCount =
		OwnedRelicIds.Remove(RelicId);

	if (RemovedCount <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicSubsystem] Remove failed. Relic is not owned. RelicId=%s"),
			*RelicId.ToString());

		return false;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[RelicSubsystem] Relic removed. RelicId=%s OwnedCount=%d"),
		*RelicId.ToString(),
		OwnedRelicIds.Num());

	OnRelicRemoved.Broadcast(RelicId);

	return true;
}

bool UPBRelicSubsystem::HasRelic(
	const FName RelicId) const
{
	if (RelicId.IsNone())
	{
		return false;
	}

	return OwnedRelicIds.Contains(RelicId);
}

const TArray<FName>&
UPBRelicSubsystem::GetOwnedRelicIds() const
{
	return OwnedRelicIds;
}

UPBRelicCalculator*
UPBRelicSubsystem::GetCalculator() const
{
	return RelicCalculator;
}

bool UPBRelicSubsystem::GetRandomRelicIds(int32 Count, TArray<FName>& OutRelicIds) const
{
	OutRelicIds.Reset();

	if (Count <= 0)
	{
		return false;
	}

	UGameInstance* GameInstance =
		GetGameInstance();

	if (!IsValid(GameInstance))
	{
		return false;
	}

	const UPBTableDataSubsystem* TableSubsystem =
		GameInstance->GetSubsystem<UPBTableDataSubsystem>();

	if (!IsValid(TableSubsystem))
	{
		return false;
	}

	TArray<FName> RelicIds;

	TableSubsystem->GetAllRelicIds(
		RelicIds);

	RelicIds.RemoveAll(
		[this](const FName RelicId)
		{
			return RelicId.IsNone() ||
				HasRelic(RelicId);
		});

	if (RelicIds.IsEmpty())
	{
		return false;
	}

	for (int32 Index = RelicIds.Num() - 1;
		 Index > 0;
		 --Index)
	{
		const int32 SwapIndex =
			FMath::RandRange(
				0,
				Index);

		RelicIds.Swap(
			Index,
			SwapIndex);
	}

	const int32 ResultCount =
		FMath::Min(
			Count,
			RelicIds.Num());

	for (int32 Index = 0;
		 Index < ResultCount;
		 ++Index)
	{
		OutRelicIds.Add(
			RelicIds[Index]);
	}

	return !OutRelicIds.IsEmpty();
}

bool UPBRelicSubsystem::ApplyRelicEffect(const FName RelicId, const FPBRelicTableRow& RelicRow, AActor* TargetActor)
{
	if (RelicRow.EffectId.IsNone())
	{
		return false;
	}

	UGameInstance* GameInstance =
		GetGameInstance();

	if (!IsValid(GameInstance))
	{
		return false;
	}

	UPBEffectSubsystem* EffectSubsystem =
		GameInstance->GetSubsystem<
			UPBEffectSubsystem>();

	if (!IsValid(EffectSubsystem))
	{
		return false;
	}

	FPBEffectContext Context;
	Context.WorldContextObject = this;
	Context.SourceActor = TargetActor;

	return EffectSubsystem->ApplyEffect(
		RelicRow.EffectId,
		Context);
}
