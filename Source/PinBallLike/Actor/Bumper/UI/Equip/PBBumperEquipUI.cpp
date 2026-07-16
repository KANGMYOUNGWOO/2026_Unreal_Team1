#include "PBBumperEquipUI.h"

#include "PBBumperEquipUIBuilder.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperInfoPanelViewModel.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperListItemObject.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "Components/Button.h"

void UPBBumperEquipUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindBoardSlotButtons();
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
	UnbindDataLoadEvents();

	Super::NativeDestruct();
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
	if (GetSelectedBumperSlotType() != SlotType
		|| !PlayerDataSubsystem->EquipBumperAtSlot(SelectedBumperEquipSlot, RowName))
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
	EnsureInfoPanelViewModel();
	if (!IsValid(InfoPanelViewModel))
	{
		return;
	}

	const FPBBumperTableRow* Row = PBBumperEquipUIBuilder::FindBumperRow(BumperRowNames, BumperRows, RowName);
	if (!Row)
	{
		InfoPanelViewModel->ClearBumperInfoPanelData();
		return;
	}

	FName EquippedRowName = NAME_None;
	const bool bIsEquippedInSelectedSlot = IsValid(PlayerDataSubsystem)
		&& PlayerDataSubsystem->GetEquippedBumperAtSlot(SelectedBumperEquipSlot, EquippedRowName)
		&& EquippedRowName == RowName;

	InfoPanelViewModel->SetBumperInfoPanelData(
		Row->DisplayName,
		PBBumperEquipUIBuilder::ResolveBumperIconTexture(GameDataLoadSubsystem.Get(), RowName, *Row),
		PBBumperEquipUIBuilder::ResolveBumperDescription(TableDataSubsystem.Get(), *Row),
		bIsEquippedInSelectedSlot);
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

	FLinearColor SlotColor;
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
		SlotColor = TopSlotColor;
		break;
	case EPBBumperSlotType::Side:
		SlotColor = SideSlotColor;
		break;
	case EPBBumperSlotType::Rebound:
		SlotColor = ReboundSlotColor;
		break;
	case EPBBumperSlotType::Special:
		SlotColor = SpecialSlotColor;
		break;
	default:
		SlotColor = FLinearColor::White;
		break;
	}

	SlotColor.A = SelectedBumperEquipSlot == EquipSlot ? 1.0f : UnselectedSlotOpacity;
	Button->SetBackgroundColor(SlotColor);
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
