#include "PBCollectionTabWidgetBase.h"

#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "PinBallLike/Collection/PBCollectionSubsystem.h"
#include "PBCollectionCatalogItemObject.h"

void UPBCollectionTabWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		CollectionSubsystem = GameInstance->GetSubsystem<UPBCollectionSubsystem>();
	}

	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.AddUniqueDynamic(this, &ThisClass::HandleSearchTextChanged);
	}
	if (CollectionSubsystem)
	{
		CollectionSubsystem->OnCollectionDataReady.AddUniqueDynamic(this, &ThisClass::HandleCollectionDataReady);
		CollectionSubsystem->OnCollectionEntryChanged.AddUniqueDynamic(this, &ThisClass::HandleCollectionEntryChanged);
	}
}

void UPBCollectionTabWidgetBase::NativeDestruct()
{
	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.RemoveDynamic(this, &ThisClass::HandleSearchTextChanged);
	}
	if (CollectionSubsystem)
	{
		CollectionSubsystem->OnCollectionDataReady.RemoveDynamic(this, &ThisClass::HandleCollectionDataReady);
		CollectionSubsystem->OnCollectionEntryChanged.RemoveDynamic(this, &ThisClass::HandleCollectionEntryChanged);
	}

	Super::NativeDestruct();
}

void UPBCollectionTabWidgetBase::ActivateTab()
{
	bIsActiveTab = true;
	RefreshTab();
	BP_OnTabActivated();
}

void UPBCollectionTabWidgetBase::DeactivateTab()
{
	bIsActiveTab = false;
	BP_OnTabDeactivated();
}

void UPBCollectionTabWidgetBase::RefreshTab()
{
}

bool UPBCollectionTabWidgetBase::IsCatalogDataReady(const FText& LoadingText) const
{
	if (!CollectionSubsystem || !CollectionSubsystem->IsDataReady())
	{
		SetStatus(LoadingText, true);
		return false;
	}
	return true;
}

FString UPBCollectionTabWidgetBase::GetNormalizedSearchText() const
{
	return SearchTextBox ? SearchTextBox->GetText().ToString().TrimStartAndEnd() : FString();
}

bool UPBCollectionTabWidgetBase::MatchesSearch(
	const FPBCollectionItemSummary& Summary,
	const TArray<FText>& AdditionalTexts) const
{
	const FString SearchText = GetNormalizedSearchText();
	if (SearchText.IsEmpty())
	{
		return true;
	}

	const auto Contains = [&SearchText](const FText& Source)
	{
		return Source.ToString().Contains(SearchText, ESearchCase::IgnoreCase);
	};

	if (Contains(Summary.DisplayName) || Contains(Summary.Subtitle) || Contains(Summary.Description))
	{
		return true;
	}

	return AdditionalTexts.ContainsByPredicate([&Contains](const FText& Text)
	{
		return Contains(Text);
	});
}

int32 UPBCollectionTabWidgetBase::PopulateCatalogItems(
	UListView* ListView,
	const EPBCollectionCategory Category,
	const TArray<FPBCollectionItemSummary>& Summaries,
	const TArray<int32>& DataIndexes)
{
	CatalogItems.Reset();
	if (!ListView || Summaries.Num() != DataIndexes.Num())
	{
		return INDEX_NONE;
	}

	ListView->ClearListItems();
	int32 SelectedItemIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Summaries.Num(); ++Index)
	{
		UPBCollectionCatalogItemObject* Item = NewObject<UPBCollectionCatalogItemObject>(this);
		Item->Category = Category;
		Item->Summary = Summaries[Index];
		Item->DataIndex = DataIndexes[Index];
		CatalogItems.Add(Item);
		ListView->AddItem(Item);

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

	UPBCollectionCatalogItemObject* SelectedItem = CatalogItems[SelectedItemIndex];
	SelectedSourceRowName = SelectedItem->Summary.SourceRowName;
	ListView->SetSelectedItem(SelectedItem);
	return SelectedItem->DataIndex;
}

const UPBCollectionCatalogItemObject* UPBCollectionTabWidgetBase::ResolveCatalogItem(UObject* ItemObject)
{
	const UPBCollectionCatalogItemObject* Item = Cast<UPBCollectionCatalogItemObject>(ItemObject);
	if (Item)
	{
		SelectedSourceRowName = Item->Summary.SourceRowName;
	}
	return Item;
}

void UPBCollectionTabWidgetBase::SetStatus(const FText& Text, const bool bShow) const
{
	if (!StatusText)
	{
		return;
	}

	StatusText->SetText(Text);
	StatusText->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UPBCollectionTabWidgetBase::HandleSearchTextChanged(const FText& Text)
{
	if (bIsActiveTab)
	{
		RefreshTab();
	}
}

void UPBCollectionTabWidgetBase::HandleCollectionDataReady(const bool bReady)
{
	if (bReady && bIsActiveTab)
	{
		RefreshTab();
	}
}

void UPBCollectionTabWidgetBase::HandleCollectionEntryChanged(FName CollectionId)
{
	if (bIsActiveTab)
	{
		RefreshTab();
	}
}
