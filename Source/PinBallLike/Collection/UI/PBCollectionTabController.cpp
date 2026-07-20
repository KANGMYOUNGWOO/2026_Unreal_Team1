#include "PBCollectionTabController.h"

#include "PBCollectionCatalogItemObject.h"
#include "Engine/GameInstance.h"
#include "PinBallLike/Collection/PBCollectionSubsystem.h"

void UPBCollectionTabController::Initialize(UGameInstance* InGameInstance)
{
	if (bInitialized || !IsValid(InGameInstance))
	{
		return;
	}

	CollectionSubsystem = InGameInstance->GetSubsystem<UPBCollectionSubsystem>();
	if (!IsValid(CollectionSubsystem))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Collection] Tab controller initialization deferred. Missing CollectionSubsystem."));
		return;
	}

	CollectionSubsystem->OnCollectionDataReady.AddUniqueDynamic(
		this,
		&ThisClass::HandleCollectionDataReady);
	CollectionSubsystem->OnCollectionEntryChanged.AddUniqueDynamic(
		this,
		&ThisClass::HandleCollectionEntryChanged);
	bInitialized = true;
}

void UPBCollectionTabController::Shutdown()
{
	if (!bInitialized)
	{
		return;
	}

	if (IsValid(CollectionSubsystem))
	{
		CollectionSubsystem->OnCollectionDataReady.RemoveDynamic(
			this,
			&ThisClass::HandleCollectionDataReady);
		CollectionSubsystem->OnCollectionEntryChanged.RemoveDynamic(
			this,
			&ThisClass::HandleCollectionEntryChanged);
	}
	bIsActive = false;
	bInitialized = false;
	CatalogItems.Reset();
	CollectionSubsystem = nullptr;
}

void UPBCollectionTabController::Activate()
{
	bIsActive = true;
}

void UPBCollectionTabController::Deactivate()
{
	bIsActive = false;
}

void UPBCollectionTabController::SetSearchText(const FText& Text)
{
	const FString NewSearchText = Text.ToString().TrimStartAndEnd();
	if (NormalizedSearchText.Equals(NewSearchText, ESearchCase::CaseSensitive))
	{
		return;
	}

	NormalizedSearchText = NewSearchText;
	if (bIsActive)
	{
		OnRefreshRequested.Broadcast();
	}
}

void UPBCollectionTabController::SetSortMode(const EPBCollectionSortMode InSortMode)
{
	if (SortMode == InSortMode)
	{
		return;
	}

	SortMode = InSortMode;
	if (bIsActive)
	{
		OnRefreshRequested.Broadcast();
	}
}

bool UPBCollectionTabController::IsCatalogDataReady() const
{
	return IsValid(CollectionSubsystem) && CollectionSubsystem->IsDataReady();
}

bool UPBCollectionTabController::HasDataLoadCompleted() const
{
	return IsValid(CollectionSubsystem) && CollectionSubsystem->HasDataLoadCompleted();
}

bool UPBCollectionTabController::MatchesSearch(
	const FPBCollectionItemSummary& Summary,
	const TArray<FText>& AdditionalTexts) const
{
	if (NormalizedSearchText.IsEmpty())
	{
		return true;
	}

	const auto ContainsSearchText = [this](const FText& Source)
	{
		return Source.ToString().Contains(NormalizedSearchText, ESearchCase::IgnoreCase);
	};
	if (ContainsSearchText(Summary.DisplayName)
		|| ContainsSearchText(Summary.Subtitle)
		|| ContainsSearchText(Summary.Description))
	{
		return true;
	}

	return AdditionalTexts.ContainsByPredicate(ContainsSearchText);
}

int32 UPBCollectionTabController::BuildCatalogItems(
	const EPBCollectionCategory Category,
	const TArray<FPBCollectionItemSummary>& Summaries,
	const TArray<int32>& DataIndexes,
	TArray<UPBCollectionCatalogItemObject*>& OutItems,
	UPBCollectionCatalogItemObject*& OutSelectedItem)
{
	CatalogItems.Reset();
	OutItems.Reset();
	OutSelectedItem = nullptr;
	if (Summaries.Num() != DataIndexes.Num())
	{
		return INDEX_NONE;
	}

	TArray<int32> SortedSourceIndexes;
	SortedSourceIndexes.Reserve(Summaries.Num());
	for (int32 Index = 0; Index < Summaries.Num(); ++Index)
	{
		SortedSourceIndexes.Add(Index);
	}

	SortedSourceIndexes.StableSort([this, &Summaries](const int32 LeftIndex, const int32 RightIndex)
	{
		const FPBCollectionItemSummary& Left = Summaries[LeftIndex];
		const FPBCollectionItemSummary& Right = Summaries[RightIndex];
		const auto CompareDefault = [&Left, &Right]()
		{
			return Left.SortOrder != Right.SortOrder
				? Left.SortOrder < Right.SortOrder
				: Left.SourceRowName.ToString() < Right.SourceRowName.ToString();
		};

		const int32 NameComparison = Left.DisplayName.ToString().Compare(
			Right.DisplayName.ToString(),
			ESearchCase::IgnoreCase);
		switch (SortMode)
		{
		case EPBCollectionSortMode::NameAsc:
			return NameComparison != 0 ? NameComparison < 0 : CompareDefault();
		case EPBCollectionSortMode::NameDesc:
			return NameComparison != 0 ? NameComparison > 0 : CompareDefault();
		case EPBCollectionSortMode::SortOrder:
		default:
			return CompareDefault();
		}
	});

	int32 SelectedItemIndex = INDEX_NONE;
	for (const int32 SourceIndex : SortedSourceIndexes)
	{
		UPBCollectionCatalogItemObject* Item = NewObject<UPBCollectionCatalogItemObject>(this);
		Item->Category = Category;
		Item->Summary = Summaries[SourceIndex];
		Item->DataIndex = DataIndexes[SourceIndex];
		CatalogItems.Add(Item);
		OutItems.Add(Item);

		if (Item->Summary.SourceRowName == SelectedSourceRowName)
		{
			SelectedItemIndex = OutItems.Num() - 1;
		}
	}

	if (CatalogItems.IsEmpty())
	{
		SelectedSourceRowName = NAME_None;
		return INDEX_NONE;
	}

	if (!CatalogItems.IsValidIndex(SelectedItemIndex))
	{
		SelectedItemIndex = 0;
	}

	OutSelectedItem = CatalogItems[SelectedItemIndex];
	SelectedSourceRowName = OutSelectedItem->Summary.SourceRowName;
	return OutSelectedItem->DataIndex;
}

const UPBCollectionCatalogItemObject* UPBCollectionTabController::ResolveCatalogItem(UObject* ItemObject)
{
	const UPBCollectionCatalogItemObject* Item = Cast<UPBCollectionCatalogItemObject>(ItemObject);
	if (Item)
	{
		SelectedSourceRowName = Item->Summary.SourceRowName;
	}
	return Item;
}

void UPBCollectionTabController::HandleCollectionDataReady(const bool bReady)
{
	static_cast<void>(bReady);
	if (bIsActive)
	{
		OnRefreshRequested.Broadcast();
	}
}

void UPBCollectionTabController::HandleCollectionEntryChanged(FName CollectionId)
{
	static_cast<void>(CollectionId);
	if (bIsActive)
	{
		OnRefreshRequested.Broadcast();
	}
}
