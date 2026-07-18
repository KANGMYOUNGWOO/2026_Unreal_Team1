#include "PBCollectionTabWidgetBase.h"

#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "PinBallLike/Collection/PBCollectionSubsystem.h"
#include "PBCollectionCatalogItemObject.h"
#include "PBCollectionTabController.h"

void UPBCollectionTabWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TabController = NewObject<UPBCollectionTabController>(this);
}

void UPBCollectionTabWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!TabController)
	{
		TabController = NewObject<UPBCollectionTabController>(this);
	}
	if (!IsValid(TabController))
	{
		return;
	}
	TabController->OnRefreshRequested.RemoveAll(this);
	TabController->OnRefreshRequested.AddUObject(this, &ThisClass::HandleRefreshRequested);
	TabController->Initialize(GetGameInstance());

	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.AddUniqueDynamic(this, &ThisClass::HandleSearchTextChanged);
		TabController->SetSearchText(SearchTextBox->GetText());
	}
}

void UPBCollectionTabWidgetBase::NativeDestruct()
{
	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.RemoveDynamic(this, &ThisClass::HandleSearchTextChanged);
	}
	if (TabController)
	{
		TabController->OnRefreshRequested.RemoveAll(this);
		TabController->Shutdown();
	}

	Super::NativeDestruct();
}

void UPBCollectionTabWidgetBase::ActivateTab()
{
	if (TabController)
	{
		TabController->Activate();
	}
	RefreshTab();
	BP_OnTabActivated();
}

void UPBCollectionTabWidgetBase::DeactivateTab()
{
	if (TabController)
	{
		TabController->Deactivate();
	}
	BP_OnTabDeactivated();
}

void UPBCollectionTabWidgetBase::RefreshTab()
{
}

bool UPBCollectionTabWidgetBase::IsCatalogDataReady(const FText& LoadingText) const
{
	if (!TabController || !TabController->IsCatalogDataReady())
	{
		const FText StatusTextValue = TabController && TabController->HasDataLoadCompleted()
			? NSLOCTEXT("PBCollection", "CatalogLoadFailed", "도감 데이터 일부를 불러오지 못했습니다.")
			: LoadingText;
		SetStatus(StatusTextValue, true);
		return false;
	}
	return true;
}

bool UPBCollectionTabWidgetBase::MatchesSearch(
	const FPBCollectionItemSummary& Summary,
	const TArray<FText>& AdditionalTexts) const
{
	return TabController && TabController->MatchesSearch(Summary, AdditionalTexts);
}

int32 UPBCollectionTabWidgetBase::PopulateCatalogItems(
	UListView* ListView,
	const EPBCollectionCategory Category,
	const TArray<FPBCollectionItemSummary>& Summaries,
	const TArray<int32>& DataIndexes)
{
	if (!ListView || !TabController)
	{
		return INDEX_NONE;
	}

	TArray<UPBCollectionCatalogItemObject*> CatalogItems;
	UPBCollectionCatalogItemObject* SelectedItem = nullptr;
	const int32 SelectedDataIndex = TabController->BuildCatalogItems(
		Category,
		Summaries,
		DataIndexes,
		CatalogItems,
		SelectedItem);

	ListView->ClearListItems();
	for (UPBCollectionCatalogItemObject* Item : CatalogItems)
	{
		ListView->AddItem(Item);
	}
	if (SelectedItem)
	{
		ListView->SetSelectedItem(SelectedItem);
	}
	return SelectedDataIndex;
}

const UPBCollectionCatalogItemObject* UPBCollectionTabWidgetBase::ResolveCatalogItem(UObject* ItemObject)
{
	return TabController ? TabController->ResolveCatalogItem(ItemObject) : nullptr;
}

UPBCollectionSubsystem* UPBCollectionTabWidgetBase::GetCollectionSubsystem() const
{
	return TabController ? TabController->GetCollectionSubsystem() : nullptr;
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
	if (TabController)
	{
		TabController->SetSearchText(Text);
	}
}

void UPBCollectionTabWidgetBase::HandleRefreshRequested()
{
	RefreshTab();
}
