#include "PBCollectionTabWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ListView.h"
#include "Components/SizeBox.h"
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
	EnsureSortControl();
	ConfigureSortControl();

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
	if (SortComboBox)
	{
		SortComboBox->OnSelectionChanged.RemoveDynamic(this, &ThisClass::HandleSortSelectionChanged);
		SortComboBox->OnGenerateWidgetEvent.Unbind();
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
		TabController->Initialize(GetGameInstance());
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

bool UPBCollectionTabWidgetBase::IsCatalogDataReady(
	const EPBCollectionCategory Category,
	const FText& LoadingText) const
{
	if (!TabController || !TabController->IsCatalogDataReady(Category))
	{
		const FText StatusTextValue = TabController && TabController->HasDataLoadCompleted()
			? NSLOCTEXT("PBCollection", "CatalogLoadFailed", "도감 데이터 일부를 불러오지 못했습니다.")
			: LoadingText;
		SetStatus(StatusTextValue, true);
		return false;
	}
	return true;
}

bool UPBCollectionTabWidgetBase::PrepareCatalogRefresh(
	UListView* ListView,
	const EPBCollectionCategory Category,
	const FText& LoadingText) const
{
	if (!ListView)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Collection] Catalog refresh skipped. Missing ListView binding. Widget=%s"),
			*GetNameSafe(this));
		return false;
	}
	if (IsCatalogDataReady(Category, LoadingText))
	{
		return true;
	}

	ListView->ClearListItems();
	return false;
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

void UPBCollectionTabWidgetBase::HandleSortSelectionChanged(
	FString SelectedItem,
	const ESelectInfo::Type SelectionType)
{
	static_cast<void>(SelectedItem);
	static_cast<void>(SelectionType);
	if (!TabController || !SortComboBox)
	{
		return;
	}

	switch (SortComboBox->GetSelectedIndex())
	{
	case 1:
		TabController->SetSortMode(EPBCollectionSortMode::NameAsc);
		break;
	case 2:
		TabController->SetSortMode(EPBCollectionSortMode::NameDesc);
		break;
	case 0:
	default:
		TabController->SetSortMode(EPBCollectionSortMode::SortOrder);
		break;
	}
}

void UPBCollectionTabWidgetBase::EnsureSortControl()
{
	if (SortComboBox || !SearchTextBox || !WidgetTree)
	{
		return;
	}

	UWidget* SearchContainer = SearchTextBox;
	UHorizontalBox* HeaderRow = nullptr;
	while (SearchContainer && SearchContainer->GetParent())
	{
		if (UHorizontalBox* HorizontalParent = Cast<UHorizontalBox>(SearchContainer->GetParent()))
		{
			HeaderRow = HorizontalParent;
			break;
		}
		SearchContainer = SearchContainer->GetParent();
	}
	if (!HeaderRow || !SearchContainer)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Collection] Sort control fallback skipped. SearchTextBox is not inside a HorizontalBox. Widget=%s"),
			*GetNameSafe(this));
		return;
	}

	USizeBox* SortSizeBox = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(),
		TEXT("SortSize_RuntimeFallback"));
	SortComboBox = WidgetTree->ConstructWidget<UComboBoxString>(
		UComboBoxString::StaticClass(),
		TEXT("SortComboBox_RuntimeFallback"));
	if (!SortSizeBox || !SortComboBox)
	{
		SortComboBox = nullptr;
		return;
	}

	SortComboBox->SetContentPadding(FMargin(10.0f, 4.0f));
	SortSizeBox->SetWidthOverride(150.0f);
	SortSizeBox->SetHeightOverride(32.0f);
	SortSizeBox->AddChild(SortComboBox);

	UHorizontalBoxSlot* SortSlot = HeaderRow->AddChildToHorizontalBox(SortSizeBox);
	if (SortSlot)
	{
		SortSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		SortSlot->SetHorizontalAlignment(HAlign_Right);
		SortSlot->SetVerticalAlignment(VAlign_Center);
	}
}

void UPBCollectionTabWidgetBase::ConfigureSortControl()
{
	if (!SortComboBox || !TabController)
	{
		return;
	}

	SortComboBox->OnSelectionChanged.RemoveDynamic(this, &ThisClass::HandleSortSelectionChanged);
	SortComboBox->OnGenerateWidgetEvent.Unbind();
	SortComboBox->OnGenerateWidgetEvent.BindDynamic(this, &ThisClass::GenerateSortOptionWidget);
	SortComboBox->ClearOptions();
	SortComboBox->AddOption(NSLOCTEXT("PBCollection", "SortDefault", "기본 순서").ToString());
	SortComboBox->AddOption(NSLOCTEXT("PBCollection", "SortNameAscending", "이름 가나다순").ToString());
	SortComboBox->AddOption(NSLOCTEXT("PBCollection", "SortNameDescending", "이름 역순").ToString());

	int32 SelectedIndex = 0;
	if (TabController->GetSortMode() == EPBCollectionSortMode::NameAsc)
	{
		SelectedIndex = 1;
	}
	else if (TabController->GetSortMode() == EPBCollectionSortMode::NameDesc)
	{
		SelectedIndex = 2;
	}
	SortComboBox->SetSelectedIndex(SelectedIndex);
	SortComboBox->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::HandleSortSelectionChanged);
}

UWidget* UPBCollectionTabWidgetBase::GenerateSortOptionWidget(FString Item)
{
	if (!WidgetTree)
	{
		return nullptr;
	}

	UTextBlock* OptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (!OptionText)
	{
		return nullptr;
	}

	OptionText->SetText(FText::FromString(Item));
	OptionText->SetAutoWrapText(false);
	OptionText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	OptionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	if (SearchTextBox)
	{
		FSlateFontInfo SortFont = SearchTextBox->GetWidgetStyle().TextStyle.Font;
		SortFont.Size = FMath::Min(SortFont.Size, 12);
		OptionText->SetFont(SortFont);
	}
	return OptionText;
}

void UPBCollectionTabWidgetBase::HandleRefreshRequested()
{
	RefreshTab();
}
