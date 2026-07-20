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

	int32 SelectedItemIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Summaries.Num(); ++Index)
	{
		UPBCollectionCatalogItemObject* Item = NewObject<UPBCollectionCatalogItemObject>(this);
		Item->Category = Category;
		Item->Summary = Summaries[Index];
		Item->DataIndex = DataIndexes[Index];
		CatalogItems.Add(Item);
		OutItems.Add(Item);

		if (Item->Summary.SourceRowName == SelectedSourceRowName)
		{
			SelectedItemIndex = Index;
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
