#include "PBBumperEquipUI.h"

#include "PBBumperDragDropOperation.h"
#include "PBBumperEquipUIBuilder.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperInfoPanelViewModel.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperListItemObject.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Utils/PBTextFormatUtils.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/WidgetTree.h"

namespace
{
	constexpr int32 TopCatalogIndex = 0;
	constexpr int32 SideCatalogIndex = 1;
	constexpr int32 ReboundCatalogIndex = 2;
	constexpr int32 SpecialCatalogIndex = 3;
	constexpr int32 BumperEquipSlotCount = 7;
}

void UPBBumperEquipUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindBoardSlotButtons();
	BindRedesignedControls();
	CacheRequiredSubsystems();
	EnsureInfoPanelViewModel();
	BindDataLoadEvents();

	if (LoadBumperRowsOnce())
	{
		RequestBumperUIAssetsAsync();
	}
}

void UPBBumperEquipUI::NativeConstruct()
{
	Super::NativeConstruct();
	bWidgetConstructed = true;
	RefreshRedesignedPresentation();

	// Cached assets can finish during NativeOnInitialized, before Blueprint Construct binds this event.
	if (bBumperListItemObjectsBuilt)
	{
		OnBumperListItemsReady.Broadcast();

		// Recreate the Blueprint list entries first, then restore the visible selection and details.
		if (!SelectedBumperRowName.IsNone())
		{
			SelectBumperRow(SelectedBumperRowName);
		}
		else
		{
			SelectBumperEquipSlot(SelectedBumperEquipSlot);
		}
	}
}

void UPBBumperEquipUI::NativeDestruct()
{
	bWidgetConstructed = false;
	HoveredDropSlot.Reset();
	UnbindDataLoadEvents();

	Super::NativeDestruct();
}

void UPBBumperEquipUI::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

bool UPBBumperEquipUI::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UPBBumperDragDropOperation* DragOperation = Cast<UPBBumperDragDropOperation>(InOperation);
	if (!IsValid(DragOperation) || !DragOperation->IsValidBumperDrag())
	{
		SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}

	EPBBumperEquipSlot TargetSlot;
	const bool bCanDrop = FindBoardSlotAtScreenPosition(
		InDragDropEvent.GetScreenSpacePosition(),
		TargetSlot)
		&& CanEquipBumperRowAtSlot(DragOperation->BumperRowName, TargetSlot);

	SetHoveredDropSlot(bCanDrop
		? TOptional<EPBBumperEquipSlot>(TargetSlot)
		: TOptional<EPBBumperEquipSlot>());
	return bCanDrop;
}

bool UPBBumperEquipUI::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UPBBumperDragDropOperation* DragOperation = Cast<UPBBumperDragDropOperation>(InOperation);
	if (!IsValid(DragOperation) || !DragOperation->IsValidBumperDrag())
	{
		SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	EPBBumperEquipSlot TargetSlot;
	const bool bCanDrop = FindBoardSlotAtScreenPosition(
		InDragDropEvent.GetScreenSpacePosition(),
		TargetSlot)
		&& CanEquipBumperRowAtSlot(DragOperation->BumperRowName, TargetSlot);
	SetHoveredDropSlot(TOptional<EPBBumperEquipSlot>());

	return bCanDrop && EquipBumperRowAtSlot(DragOperation->BumperRowName, TargetSlot);
}

void UPBBumperEquipUI::GetBumperListItemObjects(
	TArray<UPBBumperListItemObject*>& OutTopItems,
	TArray<UPBBumperListItemObject*>& OutSideItems,
	TArray<UPBBumperListItemObject*>& OutReboundItems,
	TArray<UPBBumperListItemObject*>& OutSpecialItems) const
{
	OutTopItems.Reset();
	OutSideItems.Reset();
	OutReboundItems.Reset();
	OutSpecialItems.Reset();

	PBBumperEquipUIBuilder::AppendListItemObjects(TopItems, OutTopItems);
	PBBumperEquipUIBuilder::AppendListItemObjects(SideItems, OutSideItems);
	PBBumperEquipUIBuilder::AppendListItemObjects(ReboundItems, OutReboundItems);
	PBBumperEquipUIBuilder::AppendListItemObjects(SpecialItems, OutSpecialItems);
}

bool UPBBumperEquipUI::SelectBumperSlot(const EPBBumperSlotType SlotType)
{
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
	case EPBBumperSlotType::Side:
	case EPBBumperSlotType::Rebound:
	case EPBBumperSlotType::Special:
		break;
	default:
		return false;
	}

	return SelectBumperEquipSlot(PBBumperEquipSlotUtils::GetDefaultEquipSlot(SlotType));
}

bool UPBBumperEquipUI::SelectBumperEquipSlot(const EPBBumperEquipSlot EquipSlot)
{
	EPBBumperSlotType SlotType;
	if (!PBBumperEquipSlotUtils::TryGetSlotType(EquipSlot, SlotType))
	{
		return false;
	}

	SelectedBumperEquipSlot = EquipSlot;
	SelectedBumperRowName = NAME_None;

	FName EquippedRowName = NAME_None;
	if (GetEquippedBumperForEquipSlot(EquipSlot, EquippedRowName))
	{
		SelectedBumperRowName = EquippedRowName;
		UpdateInfoPanelByRowName(EquippedRowName);
	}
	else if (IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel->ClearBumperInfoPanelData();
	}

	BroadcastSelectedSlotChanged();
	RefreshBoardSlotSelection();
	RefreshRedesignedPresentation();
	return true;
}

void UPBBumperEquipUI::SelectBumperRow(const FName RowName)
{
	if (RowName.IsNone())
	{
		return;
	}

	SelectedBumperRowName = RowName;

	EPBBumperSlotType SlotType;
	if (PBBumperEquipUIBuilder::TryGetBumperSlotTypeByRowName(BumperRowNames, BumperRows, RowName, SlotType))
	{
		EPBBumperSlotType CurrentSlotType;
		if (!PBBumperEquipSlotUtils::TryGetSlotType(SelectedBumperEquipSlot, CurrentSlotType)
			|| CurrentSlotType != SlotType)
		{
			SelectedBumperEquipSlot = PBBumperEquipSlotUtils::GetDefaultEquipSlot(SlotType);
		}

		BroadcastSelectedSlotChanged();
		RefreshBoardSlotSelection();
	}

	UpdateInfoPanelByRowName(RowName);
	RefreshRedesignedPresentation();
}

bool UPBBumperEquipUI::GetEquippedBumperForSlot(
	const EPBBumperSlotType SlotType,
	FName& OutBumperRowId) const
{
	EPBBumperSlotType SelectedSlotType;
	const EPBBumperEquipSlot EquipSlot =
		PBBumperEquipSlotUtils::TryGetSlotType(SelectedBumperEquipSlot, SelectedSlotType)
		&& SelectedSlotType == SlotType
			? SelectedBumperEquipSlot
			: PBBumperEquipSlotUtils::GetDefaultEquipSlot(SlotType);

	return GetEquippedBumperForEquipSlot(EquipSlot, OutBumperRowId);
}

bool UPBBumperEquipUI::GetEquippedBumperForEquipSlot(
	const EPBBumperEquipSlot EquipSlot,
	FName& OutBumperRowId) const
{
	if (!IsValid(PlayerDataSubsystem))
	{
		OutBumperRowId = NAME_None;
		return false;
	}

	return PlayerDataSubsystem->GetEquippedBumperAtSlot(EquipSlot, OutBumperRowId);
}

EPBBumperSlotType UPBBumperEquipUI::GetSelectedBumperSlotType() const
{
	EPBBumperSlotType SlotType = EPBBumperSlotType::Top;
	PBBumperEquipSlotUtils::TryGetSlotType(SelectedBumperEquipSlot, SlotType);
	return SlotType;
}

bool UPBBumperEquipUI::EquipBumperRow(const FName RowName)
{
	SelectBumperRow(RowName);
	return EquipBumperRowAtSlot(RowName, SelectedBumperEquipSlot);
}

bool UPBBumperEquipUI::EquipBumperRowAtSlot(
	const FName RowName,
	const EPBBumperEquipSlot EquipSlot)
{
	CacheRequiredSubsystems();
	if (!IsValid(PlayerDataSubsystem) || !CanEquipBumperRowAtSlot(RowName, EquipSlot))
	{
		return false;
	}

	SelectedBumperEquipSlot = EquipSlot;
	SelectedBumperRowName = RowName;
	if (!PlayerDataSubsystem->EquipBumperAtSlot(EquipSlot, RowName))
	{
		return false;
	}

	RefreshBumperEquipState(RowName);
	return true;
}

bool UPBBumperEquipUI::UnequipBumperRow(const FName RowName)
{
	CacheRequiredSubsystems();

	if (RowName.IsNone() || !IsValid(PlayerDataSubsystem))
	{
		return false;
	}

	EPBBumperSlotType SlotType;
	if (!PBBumperEquipUIBuilder::TryGetBumperSlotTypeByRowName(BumperRowNames, BumperRows, RowName, SlotType))
	{
		return false;
	}

	SelectBumperRow(RowName);
	FName EquippedRowName = NAME_None;
	if (GetSelectedBumperSlotType() != SlotType
		|| !PlayerDataSubsystem->GetEquippedBumperAtSlot(SelectedBumperEquipSlot, EquippedRowName)
		|| EquippedRowName != RowName)
	{
		return false;
	}

	if (!PlayerDataSubsystem->UnequipBumperAtSlot(SelectedBumperEquipSlot))
	{
		return false;
	}

	RefreshBumperEquipState(RowName);

	return true;
}

bool UPBBumperEquipUI::EquipSelectedBumper()
{
	return EquipBumperRow(SelectedBumperRowName);
}

bool UPBBumperEquipUI::UnequipSelectedBumper()
{
	return UnequipBumperRow(SelectedBumperRowName);
}

void UPBBumperEquipUI::HandleStartupGameDataLoaded()
{
	if (LoadBumperRowsOnce())
	{
		RequestBumperUIAssetsAsync();
	}
}

void UPBBumperEquipUI::HandleBumperUIAssetsLoaded()
{
	if (bBumperListItemObjectsBuilt || !bBumperUIAssetLoadPending)
	{
		return;
	}

	bBumperUIAssetLoadPending = false;
	BuildBumperListItemObjects();
	if (bWidgetConstructed)
	{
		OnBumperListItemsReady.Broadcast();
	}
	SelectBumperEquipSlot(EPBBumperEquipSlot::TopLeft);
}

void UPBBumperEquipUI::CacheRequiredSubsystems()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return;
	}

	if (!IsValid(TableDataSubsystem))
	{
		TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>();
	}

	if (!IsValid(PlayerDataSubsystem))
	{
		PlayerDataSubsystem = GameInstance->GetSubsystem<UPBPlayerDataSubsystem>();
	}

	if (!IsValid(GameDataLoadSubsystem))
	{
		GameDataLoadSubsystem = GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>();
	}
}

void UPBBumperEquipUI::BindDataLoadEvents()
{
	if (IsValid(TableDataSubsystem))
	{
		TableDataSubsystem->OnStartupGameDataLoaded.AddUniqueDynamic(
			this,
			&UPBBumperEquipUI::HandleStartupGameDataLoaded);
	}
}

void UPBBumperEquipUI::UnbindDataLoadEvents()
{
	if (IsValid(TableDataSubsystem))
	{
		TableDataSubsystem->OnStartupGameDataLoaded.RemoveDynamic(
			this,
			&UPBBumperEquipUI::HandleStartupGameDataLoaded);
	}
}

void UPBBumperEquipUI::EnsureInfoPanelViewModel()
{
	if (!IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel = NewObject<UPBBumperInfoPanelViewModel>(this);
	}
}

bool UPBBumperEquipUI::LoadBumperRowsOnce()
{
	if (bBumperRowsLoaded)
	{
		return true;
	}

	CacheRequiredSubsystems();
	if (!IsValid(TableDataSubsystem) || !TableDataSubsystem->IsTableDataReady())
	{
		return false;
	}

	bBumperRowsLoaded = TableDataSubsystem->GetAllBumperRows(BumperRowNames, BumperRows)
		&& BumperRowNames.Num() == BumperRows.Num();

	return bBumperRowsLoaded;
}

void UPBBumperEquipUI::RequestBumperUIAssetsAsync()
{
	if (bBumperListItemObjectsBuilt
		|| bBumperUIAssetLoadPending
		|| !IsValid(GameDataLoadSubsystem)
		|| BumperRowNames.IsEmpty())
	{
		return;
	}

	TArray<FName> BundleNames;
	BundleNames.Add(PBAssetBundleNames::UI);

	TArray<FPrimaryAssetId> BumperAssetIds;
	BumperAssetIds.Reserve(BumperRowNames.Num());
	for (const FName BumperRowName : BumperRowNames)
	{
		if (!BumperRowName.IsNone())
		{
			BumperAssetIds.Emplace(PBBumperAssetIds::Type::BumperData, BumperRowName);
		}
	}
	if (BumperAssetIds.IsEmpty())
	{
		return;
	}

	bBumperUIAssetLoadPending = true;
	GameDataLoadSubsystem->LoadPrimaryAssetsByIdsAsync(
		BumperAssetIds,
		BundleNames,
		FStreamableDelegate::CreateUObject(
			this,
			&UPBBumperEquipUI::HandleBumperUIAssetsLoaded));
}

void UPBBumperEquipUI::BuildBumperListItemObjects()
{
	if (bBumperListItemObjectsBuilt)
	{
		return;
	}

	TopItems.Reset();
	SideItems.Reset();
	ReboundItems.Reset();
	SpecialItems.Reset();

	const TSet<FName> EquippedRowIds = PBBumperEquipUIBuilder::MakeEquippedBumperRowIdSet(PlayerDataSubsystem.Get());

	PBBumperEquipUIBuilder::BuildBumperListItemObjects(
		this, this, BumperRowNames, BumperRows, EquippedRowIds, GameDataLoadSubsystem.Get(),
		TopItems, SideItems, ReboundItems, SpecialItems);

	bBumperListItemObjectsBuilt = true;

	if (IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel->ClearBumperInfoPanelData();
	}
}

void UPBBumperEquipUI::UpdateInfoPanelByRowName(const FName RowName)
{
	const FPBBumperTableRow* Row = PBBumperEquipUIBuilder::FindBumperRow(BumperRowNames, BumperRows, RowName);
	if (!Row)
	{
		if (IsValid(InfoPanelViewModel))
		{
			InfoPanelViewModel->ClearBumperInfoPanelData();
		}
		ClearDetailPresentation();
		return;
	}

	FName EquippedRowName = NAME_None;
	const bool bIsEquippedInSelectedSlot = IsValid(PlayerDataSubsystem)
		&& PlayerDataSubsystem->GetEquippedBumperAtSlot(SelectedBumperEquipSlot, EquippedRowName)
		&& EquippedRowName == RowName;

	EnsureInfoPanelViewModel();
	if (IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel->SetBumperInfoPanelData(
			Row->DisplayName,
			PBBumperEquipUIBuilder::ResolveBumperIconTexture(GameDataLoadSubsystem.Get(), RowName, *Row),
			PBBumperEquipUIBuilder::ResolveBumperDescription(TableDataSubsystem.Get(), *Row),
			bIsEquippedInSelectedSlot);
	}

	UpdateDetailPresentation(RowName);
}

void UPBBumperEquipUI::UpdateBumperListEquipStates()
{
	const TSet<FName> EquippedRowIds = PBBumperEquipUIBuilder::MakeEquippedBumperRowIdSet(PlayerDataSubsystem.Get());
	PBBumperEquipUIBuilder::UpdateBumperListEquipStates(
		EquippedRowIds,
		TopItems,
		SideItems,
		ReboundItems,
		SpecialItems);
}

void UPBBumperEquipUI::RefreshBumperEquipState(const FName RowName)
{
	UpdateBumperListEquipStates();
	SelectBumperRow(RowName);
}

void UPBBumperEquipUI::BindBoardSlotButtons()
{
	BuildBoardSlotPresentations();

	if (IsValid(TopLeftMarker))
	{
		TopLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopLeftSlotClicked);
	}
	if (IsValid(TopRightMarker))
	{
		TopRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopRightSlotClicked);
	}
	if (IsValid(SideLeftMarker))
	{
		SideLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideLeftSlotClicked);
	}
	if (IsValid(SideRightMarker))
	{
		SideRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideRightSlotClicked);
	}
	if (IsValid(ReboundLeftMarker))
	{
		ReboundLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundLeftSlotClicked);
	}
	if (IsValid(ReboundRightMarker))
	{
		ReboundRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundRightSlotClicked);
	}
	if (IsValid(SpecialCenterMarker))
	{
		SpecialCenterMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSpecialSlotClicked);
	}

	RefreshBoardSlotSelection();
}

void UPBBumperEquipUI::BuildBoardSlotPresentations()
{
	BoardSlotIconImages.Reset();
	BoardSlotLabels.Reset();

	BuildBoardSlotPresentation(TopLeftMarker, EPBBumperEquipSlot::TopLeft);
	BuildBoardSlotPresentation(TopRightMarker, EPBBumperEquipSlot::TopRight);
	BuildBoardSlotPresentation(SideLeftMarker, EPBBumperEquipSlot::SideLeft);
	BuildBoardSlotPresentation(SideRightMarker, EPBBumperEquipSlot::SideRight);
	BuildBoardSlotPresentation(ReboundLeftMarker, EPBBumperEquipSlot::ReboundLeft);
	BuildBoardSlotPresentation(ReboundRightMarker, EPBBumperEquipSlot::ReboundRight);
	BuildBoardSlotPresentation(SpecialCenterMarker, EPBBumperEquipSlot::Special);
}

void UPBBumperEquipUI::BuildBoardSlotPresentation(
	UButton* Button,
	const EPBBumperEquipSlot EquipSlot)
{
	if (!IsValid(Button) || !IsValid(WidgetTree))
	{
		return;
	}

	// 기존 Blueprint 라벨을 재사용하여 폰트 설정과 변수 참조를 보존합니다.
	UTextBlock* Label = Cast<UTextBlock>(Button->GetContent());
	if (!IsValid(Label) || !Button->RemoveChild(Label))
	{
		return;
	}

	const FString SlotSuffix = FString::FromInt(static_cast<int32>(EquipSlot));
	UVerticalBox* CardContent = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(),
		FName(*FString::Printf(TEXT("BumperSlotCard_%s"), *SlotSuffix)));
	USizeBox* IconSizeBox = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(),
		FName(*FString::Printf(TEXT("BumperSlotIconSize_%s"), *SlotSuffix)));
	UImage* IconImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(),
		FName(*FString::Printf(TEXT("BumperSlotIcon_%s"), *SlotSuffix)));
	if (!IsValid(CardContent) || !IsValid(IconSizeBox) || !IsValid(IconImage))
	{
		Button->AddChild(Label);
		return;
	}

	IconSizeBox->SetHeightOverride(98.0f);
	IconSizeBox->AddChild(IconImage);
	IconImage->SetVisibility(ESlateVisibility::Hidden);
	IconImage->SetColorAndOpacity(FLinearColor::White);

	if (UVerticalBoxSlot* IconSlot = CardContent->AddChildToVerticalBox(IconSizeBox))
	{
		IconSlot->SetPadding(FMargin(6.0f, 5.0f, 6.0f, 2.0f));
		IconSlot->SetHorizontalAlignment(HAlign_Fill);
		IconSlot->SetVerticalAlignment(VAlign_Fill);
	}

	Label->SetAutoWrapText(true);
	Label->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* LabelSlot = CardContent->AddChildToVerticalBox(Label))
	{
		LabelSlot->SetPadding(FMargin(5.0f, 2.0f, 5.0f, 5.0f));
		LabelSlot->SetHorizontalAlignment(HAlign_Fill);
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Button->AddChild(CardContent)))
	{
		ButtonSlot->SetPadding(FMargin(2.0f));
		ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
		ButtonSlot->SetVerticalAlignment(VAlign_Fill);
	}

	BoardSlotIconImages.Add(EquipSlot, IconImage);
	BoardSlotLabels.Add(EquipSlot, Label);
}

void UPBBumperEquipUI::BindRedesignedControls()
{
	if (IsValid(TopCategoryButton))
	{
		TopCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopCategoryClicked);
	}
	if (IsValid(SideCategoryButton))
	{
		SideCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideCategoryClicked);
	}
	if (IsValid(ReboundCategoryButton))
	{
		ReboundCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundCategoryClicked);
	}
	if (IsValid(SpecialCategoryButton))
	{
		SpecialCategoryButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSpecialCategoryClicked);
	}
	if (IsValid(EquipActionButton))
	{
		EquipActionButton->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleEquipActionClicked);
	}

	ClearDetailPresentation();
}

void UPBBumperEquipUI::RefreshBoardSlotSelection()
{
	SetBoardSlotButtonState(TopLeftMarker, EPBBumperEquipSlot::TopLeft);
	SetBoardSlotButtonState(TopRightMarker, EPBBumperEquipSlot::TopRight);
	SetBoardSlotButtonState(SideLeftMarker, EPBBumperEquipSlot::SideLeft);
	SetBoardSlotButtonState(SideRightMarker, EPBBumperEquipSlot::SideRight);
	SetBoardSlotButtonState(ReboundLeftMarker, EPBBumperEquipSlot::ReboundLeft);
	SetBoardSlotButtonState(ReboundRightMarker, EPBBumperEquipSlot::ReboundRight);
	SetBoardSlotButtonState(SpecialCenterMarker, EPBBumperEquipSlot::Special);
}

void UPBBumperEquipUI::RefreshRedesignedPresentation()
{
	RefreshCategoryPresentation();
	RefreshLoadoutPresentation();

	if (SelectedBumperRowName.IsNone())
	{
		ClearDetailPresentation();
	}
	else
	{
		UpdateDetailPresentation(SelectedBumperRowName);
	}
}

void UPBBumperEquipUI::RefreshCategoryPresentation()
{
	const EPBBumperSlotType SelectedSlotType = GetSelectedBumperSlotType();
	int32 CatalogIndex = TopCatalogIndex;
	switch (SelectedSlotType)
	{
	case EPBBumperSlotType::Top:
		CatalogIndex = TopCatalogIndex;
		break;
	case EPBBumperSlotType::Side:
		CatalogIndex = SideCatalogIndex;
		break;
	case EPBBumperSlotType::Rebound:
		CatalogIndex = ReboundCatalogIndex;
		break;
	case EPBBumperSlotType::Special:
		CatalogIndex = SpecialCatalogIndex;
		break;
	default:
		break;
	}

	if (IsValid(CatalogSwitcher) && CatalogSwitcher->GetNumWidgets() > CatalogIndex)
	{
		CatalogSwitcher->SetActiveWidgetIndex(CatalogIndex);
	}

	SetCategoryButtonState(TopCategoryButton, EPBBumperSlotType::Top);
	SetCategoryButtonState(SideCategoryButton, EPBBumperSlotType::Side);
	SetCategoryButtonState(ReboundCategoryButton, EPBBumperSlotType::Rebound);
	SetCategoryButtonState(SpecialCategoryButton, EPBBumperSlotType::Special);

	if (IsValid(CatalogTitleText))
	{
		CatalogTitleText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "CatalogTitleFormat", "{0} 범퍼"),
			PBBumperEquipUIBuilder::GetBumperSlotTypeDisplayName(SelectedSlotType)));
	}
	if (IsValid(SelectedSlotText))
	{
		SelectedSlotText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "SelectedSlotFormat", "선택 위치 · {0}"),
			PBBumperEquipUIBuilder::GetBumperEquipSlotDisplayName(SelectedBumperEquipSlot)));
	}
	if (IsValid(DetailAccentBorder))
	{
		DetailAccentBorder->SetBrushColor(GetSlotColor(SelectedSlotType));
	}
}

void UPBBumperEquipUI::RefreshLoadoutPresentation()
{
	static constexpr EPBBumperEquipSlot EquipSlots[] =
	{
		EPBBumperEquipSlot::TopLeft,
		EPBBumperEquipSlot::TopRight,
		EPBBumperEquipSlot::SideLeft,
		EPBBumperEquipSlot::SideRight,
		EPBBumperEquipSlot::ReboundLeft,
		EPBBumperEquipSlot::ReboundRight,
		EPBBumperEquipSlot::Special
	};

	int32 EquippedCount = 0;
	for (const EPBBumperEquipSlot EquipSlot : EquipSlots)
	{
		FName EquippedRowName = NAME_None;
		if (GetEquippedBumperForEquipSlot(EquipSlot, EquippedRowName) && !EquippedRowName.IsNone())
		{
			++EquippedCount;
		}
	}

	if (IsValid(LoadoutStatusText))
	{
		LoadoutStatusText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "LoadoutStatusFormat", "{0}개 중 {1}개 장착"),
			FText::AsNumber(BumperEquipSlotCount),
			FText::AsNumber(EquippedCount)));
	}

	if (!IsValid(SlotEquipStatusText))
	{
		return;
	}

	FName EquippedRowName = NAME_None;
	const FPBBumperTableRow* EquippedRow = nullptr;
	if (GetEquippedBumperForEquipSlot(SelectedBumperEquipSlot, EquippedRowName))
	{
		EquippedRow = PBBumperEquipUIBuilder::FindBumperRow(BumperRowNames, BumperRows, EquippedRowName);
	}

	SlotEquipStatusText->SetText(EquippedRow
		? FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "EquippedBumperFormat", "장착됨 · {0}"),
			EquippedRow->DisplayName)
		: NSLOCTEXT("PBBumperEquipUI", "EmptyEquipSlot", "비어 있음"));
}

void UPBBumperEquipUI::ClearDetailPresentation()
{
	if (IsValid(DetailIconImage))
	{
		DetailIconImage->SetBrushFromTexture(nullptr);
		DetailIconImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.16f));
	}
	if (IsValid(DetailNameText))
	{
		DetailNameText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyDetailName", "범퍼를 선택해 주세요"));
	}
	if (IsValid(DetailMetaText))
	{
		DetailMetaText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyDetailMeta", "카탈로그에서 장착할 범퍼를 선택합니다."));
	}
	if (IsValid(DetailDescriptionText))
	{
		DetailDescriptionText->SetText(FText::GetEmpty());
	}
	if (IsValid(DetailTriggerText))
	{
		DetailTriggerText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyTrigger", "발동 조건\n범퍼를 선택하면 조건을 확인할 수 있습니다."));
	}
	if (IsValid(DetailEffectText))
	{
		DetailEffectText->SetText(NSLOCTEXT("PBBumperEquipUI", "EmptyEffect", "효과\n범퍼를 선택하면 효과를 확인할 수 있습니다."));
	}
	if (IsValid(EquipActionLabel))
	{
		EquipActionLabel->SetText(NSLOCTEXT("PBBumperEquipUI", "EquipAction", "장착"));
	}
	if (IsValid(EquipActionButton))
	{
		EquipActionButton->SetIsEnabled(false);
		EquipActionButton->SetBackgroundColor(GetSlotColor(GetSelectedBumperSlotType()));
	}
}

void UPBBumperEquipUI::UpdateDetailPresentation(const FName RowName)
{
	const FPBBumperTableRow* Row = PBBumperEquipUIBuilder::FindBumperRow(BumperRowNames, BumperRows, RowName);
	if (!Row)
	{
		ClearDetailPresentation();
		return;
	}

	if (IsValid(DetailIconImage))
	{
		DetailIconImage->SetBrushFromTexture(
			PBBumperEquipUIBuilder::ResolveBumperIconTexture(GameDataLoadSubsystem.Get(), RowName, *Row));
		DetailIconImage->SetColorAndOpacity(FLinearColor::White);
	}
	if (IsValid(DetailNameText))
	{
		DetailNameText->SetText(Row->DisplayName);
	}
	if (IsValid(DetailMetaText))
	{
		DetailMetaText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "DetailMetaFormat", "{0} · {1}"),
			PBBumperEquipUIBuilder::GetBumperRoleDisplayName(Row->RoleType),
			PBBumperEquipUIBuilder::GetBumperEffectTypeDisplayName(Row->EffectType)));
	}
	if (IsValid(DetailDescriptionText))
	{
		DetailDescriptionText->SetText(PBTextFormatUtils::FormatSingleValueTemplate(
			Row->Description,
			FText::AsNumber(Row->RequiredTriggerCount)));
	}

	const FText TriggerDescription = PBBumperEquipUIBuilder::ResolveBumperTriggerDescription(TableDataSubsystem.Get(), *Row);
	if (IsValid(DetailTriggerText))
	{
		DetailTriggerText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "DetailTriggerFormat", "발동 조건\n{0}"),
			TriggerDescription.IsEmptyOrWhitespace()
				? NSLOCTEXT("PBBumperEquipUI", "MissingTrigger", "조건 정보가 없습니다.")
				: TriggerDescription));
	}

	const FText EffectDescription = PBBumperEquipUIBuilder::ResolveBumperEffectDescription(TableDataSubsystem.Get(), *Row);
	if (IsValid(DetailEffectText))
	{
		DetailEffectText->SetText(FText::Format(
			NSLOCTEXT("PBBumperEquipUI", "DetailEffectFormat", "효과\n{0}"),
			EffectDescription.IsEmptyOrWhitespace()
				? NSLOCTEXT("PBBumperEquipUI", "MissingEffect", "효과 정보가 없습니다.")
				: EffectDescription));
	}

	const bool bIsEquippedInCurrentSlot = IsSelectedBumperEquippedInCurrentSlot();
	if (IsValid(EquipActionLabel))
	{
		EquipActionLabel->SetText(bIsEquippedInCurrentSlot
			? NSLOCTEXT("PBBumperEquipUI", "UnequipAction", "해제")
			: NSLOCTEXT("PBBumperEquipUI", "EquipSelectedAction", "장착"));
	}
	if (IsValid(EquipActionButton))
	{
		EquipActionButton->SetIsEnabled(true);
		EquipActionButton->SetBackgroundColor(bIsEquippedInCurrentSlot
			? UnequipActionColor
			: GetSlotColor(GetSelectedBumperSlotType()));
	}
}

FLinearColor UPBBumperEquipUI::GetSlotColor(const EPBBumperSlotType SlotType) const
{
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
		return TopSlotColor;
	case EPBBumperSlotType::Side:
		return SideSlotColor;
	case EPBBumperSlotType::Rebound:
		return ReboundSlotColor;
	case EPBBumperSlotType::Special:
		return SpecialSlotColor;
	default:
		return FLinearColor::White;
	}
}

void UPBBumperEquipUI::SetBoardSlotButtonState(
	UButton* Button,
	const EPBBumperEquipSlot EquipSlot) const
{
	if (!IsValid(Button))
	{
		return;
	}

	EPBBumperSlotType SlotType;
	if (!PBBumperEquipSlotUtils::TryGetSlotType(EquipSlot, SlotType))
	{
		return;
	}

	FName EquippedRowName = NAME_None;
	const bool bHasEquippedBumper =
		GetEquippedBumperForEquipSlot(EquipSlot, EquippedRowName) && !EquippedRowName.IsNone();
	const FPBBumperTableRow* EquippedRow = bHasEquippedBumper
		? PBBumperEquipUIBuilder::FindBumperRow(BumperRowNames, BumperRows, EquippedRowName)
		: nullptr;

	const FText SlotDisplayName = PBBumperEquipUIBuilder::GetBumperEquipSlotDisplayName(EquipSlot);
	const FText EquippedDisplayName = EquippedRow
		? EquippedRow->DisplayName
		: bHasEquippedBumper
			? FText::FromName(EquippedRowName)
			: NSLOCTEXT("PBBumperEquipUI", "EmptyBoardSlot", "비어 있음");

	const bool bIsSelected = SelectedBumperEquipSlot == EquipSlot;
	const bool bIsDropHovered = HoveredDropSlot.IsSet() && HoveredDropSlot.GetValue() == EquipSlot;
	const FLinearColor SlotColor = GetSlotColor(SlotType);
	FLinearColor BackgroundColor = InactiveCategoryColor
		+ (SlotColor - InactiveCategoryColor) * 0.16f;
	BackgroundColor.A = 1.0f;
	if (bIsDropHovered)
	{
		BackgroundColor = InactiveCategoryColor
			+ (SlotColor - InactiveCategoryColor) * 0.82f;
		BackgroundColor.A = 1.0f;
	}
	else if (bIsSelected)
	{
		BackgroundColor = SlotColor;
	}
	else if (bHasEquippedBumper)
	{
		BackgroundColor = InactiveCategoryColor
			+ (SlotColor - InactiveCategoryColor) * EquippedSlotTintStrength;
		BackgroundColor.A = 1.0f;
	}
	Button->SetBackgroundColor(BackgroundColor);
	Button->SetToolTipText(FText::Format(
		NSLOCTEXT("PBBumperEquipUI", "BoardSlotTooltipFormat", "{0}: {1}"),
		SlotDisplayName,
		EquippedDisplayName));

	if (const TWeakObjectPtr<UImage>* IconReference = BoardSlotIconImages.Find(EquipSlot))
	{
		if (UImage* IconImage = IconReference->Get())
		{
			UTexture2D* IconTexture = EquippedRow
				? PBBumperEquipUIBuilder::ResolveBumperIconTexture(
					GameDataLoadSubsystem.Get(),
					EquippedRowName,
					*EquippedRow)
				: nullptr;
			IconImage->SetBrushFromTexture(IconTexture);
			IconImage->SetVisibility(IsValid(IconTexture)
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Hidden);
		}
	}

	if (const TWeakObjectPtr<UTextBlock>* LabelReference = BoardSlotLabels.Find(EquipSlot))
	{
		if (UTextBlock* Label = LabelReference->Get())
		{
			Label->SetText(bIsDropHovered
				? FText::Format(
					NSLOCTEXT("PBBumperEquipUI", "BoardSlotDropLabelFormat", "{0}\n여기에 장착"),
					SlotDisplayName)
				: FText::Format(
					NSLOCTEXT("PBBumperEquipUI", "BoardSlotLabelFormat", "{0}\n{1}"),
					SlotDisplayName,
					EquippedDisplayName));

			FLinearColor LabelColor = SlotColor;
			LabelColor.A = UnselectedSlotOpacity;
			if (bIsDropHovered || bIsSelected || bHasEquippedBumper)
			{
				LabelColor = FLinearColor::White;
			}
			Label->SetColorAndOpacity(FSlateColor(LabelColor));
		}
	}
}

bool UPBBumperEquipUI::CanEquipBumperRowAtSlot(
	const FName RowName,
	const EPBBumperEquipSlot EquipSlot) const
{
	if (RowName.IsNone())
	{
		return false;
	}

	EPBBumperSlotType BumperSlotType;
	EPBBumperSlotType TargetSlotType;
	return PBBumperEquipUIBuilder::TryGetBumperSlotTypeByRowName(
			BumperRowNames,
			BumperRows,
			RowName,
			BumperSlotType)
		&& PBBumperEquipSlotUtils::TryGetSlotType(EquipSlot, TargetSlotType)
		&& BumperSlotType == TargetSlotType;
}

bool UPBBumperEquipUI::FindBoardSlotAtScreenPosition(
	const FVector2D& ScreenPosition,
	EPBBumperEquipSlot& OutEquipSlot) const
{
	const auto IsUnderPointer = [&ScreenPosition](const UButton* Button)
	{
		return IsValid(Button) && Button->GetCachedGeometry().IsUnderLocation(ScreenPosition);
	};

	if (IsUnderPointer(TopLeftMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::TopLeft;
		return true;
	}
	if (IsUnderPointer(TopRightMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::TopRight;
		return true;
	}
	if (IsUnderPointer(SideLeftMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::SideLeft;
		return true;
	}
	if (IsUnderPointer(SideRightMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::SideRight;
		return true;
	}
	if (IsUnderPointer(ReboundLeftMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::ReboundLeft;
		return true;
	}
	if (IsUnderPointer(ReboundRightMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::ReboundRight;
		return true;
	}
	if (IsUnderPointer(SpecialCenterMarker))
	{
		OutEquipSlot = EPBBumperEquipSlot::Special;
		return true;
	}

	return false;
}

void UPBBumperEquipUI::SetHoveredDropSlot(const TOptional<EPBBumperEquipSlot> EquipSlot)
{
	const bool bUnchanged = HoveredDropSlot.IsSet() == EquipSlot.IsSet()
		&& (!EquipSlot.IsSet() || HoveredDropSlot.GetValue() == EquipSlot.GetValue());
	if (bUnchanged)
	{
		return;
	}

	HoveredDropSlot = EquipSlot;
	RefreshBoardSlotSelection();
}

void UPBBumperEquipUI::SetCategoryButtonState(
	UButton* Button,
	const EPBBumperSlotType SlotType) const
{
	if (!IsValid(Button))
	{
		return;
	}

	const bool bIsSelected = GetSelectedBumperSlotType() == SlotType;
	Button->SetBackgroundColor(bIsSelected ? GetSlotColor(SlotType) : InactiveCategoryColor);

	if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent()))
	{
		Label->SetColorAndOpacity(FSlateColor(
			bIsSelected ? FLinearColor::White : InactiveControlTextColor));
	}
}

void UPBBumperEquipUI::BroadcastSelectedSlotChanged()
{
	OnSelectedBumperEquipSlotChanged.Broadcast(SelectedBumperEquipSlot);
	OnSelectedBumperSlotChanged.Broadcast(GetSelectedBumperSlotType());
}

void UPBBumperEquipUI::HandleTopLeftSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::TopLeft);
}

void UPBBumperEquipUI::HandleTopRightSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::TopRight);
}

void UPBBumperEquipUI::HandleSideLeftSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::SideLeft);
}

void UPBBumperEquipUI::HandleSideRightSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::SideRight);
}

void UPBBumperEquipUI::HandleReboundLeftSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::ReboundLeft);
}

void UPBBumperEquipUI::HandleReboundRightSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::ReboundRight);
}

void UPBBumperEquipUI::HandleSpecialSlotClicked()
{
	SelectBumperEquipSlot(EPBBumperEquipSlot::Special);
}

void UPBBumperEquipUI::HandleTopCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Top);
}

void UPBBumperEquipUI::HandleSideCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Side);
}

void UPBBumperEquipUI::HandleReboundCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Rebound);
}

void UPBBumperEquipUI::HandleSpecialCategoryClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Special);
}

void UPBBumperEquipUI::HandleEquipActionClicked()
{
	if (SelectedBumperRowName.IsNone())
	{
		return;
	}

	if (IsSelectedBumperEquippedInCurrentSlot())
	{
		UnequipSelectedBumper();
	}
	else
	{
		EquipSelectedBumper();
	}
}

bool UPBBumperEquipUI::IsSelectedBumperEquippedInCurrentSlot() const
{
	FName EquippedRowName = NAME_None;
	return !SelectedBumperRowName.IsNone()
		&& GetEquippedBumperForEquipSlot(SelectedBumperEquipSlot, EquippedRowName)
		&& EquippedRowName == SelectedBumperRowName;
}
