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

void UPBBumperEquipUI::NativeDestruct()
{
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

	SelectedBumperSlotType = SlotType;
	SelectedBumperRowName = NAME_None;

	FName EquippedRowName = NAME_None;
	if (GetEquippedBumperForSlot(SlotType, EquippedRowName))
	{
		SelectedBumperRowName = EquippedRowName;
		UpdateInfoPanelByRowName(EquippedRowName);
	}
	else if (IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel->ClearBumperInfoPanelData();
	}

	OnSelectedBumperSlotChanged.Broadcast(SlotType);
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
		SelectedBumperSlotType = SlotType;
		OnSelectedBumperSlotChanged.Broadcast(SlotType);
		RefreshBoardSlotSelection();
	}

	UpdateInfoPanelByRowName(RowName);
}

bool UPBBumperEquipUI::GetEquippedBumperForSlot(
	const EPBBumperSlotType SlotType,
	FName& OutBumperRowId) const
{
	if (!IsValid(PlayerDataSubsystem))
	{
		OutBumperRowId = NAME_None;
		return false;
	}

	return PlayerDataSubsystem->GetEquippedBumper(SlotType, OutBumperRowId);
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

	if (!PlayerDataSubsystem->EquipBumper(SlotType, RowName))
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

	FName EquippedRowName = NAME_None;
	if (!PlayerDataSubsystem->GetEquippedBumper(SlotType, EquippedRowName) || EquippedRowName != RowName)
	{
		return false;
	}

	if (!PlayerDataSubsystem->UnequipBumper(SlotType))
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
	OnBumperListItemsReady.Broadcast();
	SelectBumperSlot(EPBBumperSlotType::Top);
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

	const TSet<FName> EquippedRowIds = PBBumperEquipUIBuilder::MakeEquippedBumperRowIdSet(PlayerDataSubsystem.Get());

	InfoPanelViewModel->SetBumperInfoPanelData(
		Row->DisplayName,
		PBBumperEquipUIBuilder::ResolveBumperIconTexture(GameDataLoadSubsystem.Get(), RowName, *Row),
		PBBumperEquipUIBuilder::ResolveBumperDescription(TableDataSubsystem.Get(), *Row),
		PBBumperEquipUIBuilder::IsBumperEquipped(RowName, EquippedRowIds));
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
		TopLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopSlotClicked);
	}
	if (IsValid(TopRightMarker))
	{
		TopRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleTopSlotClicked);
	}
	if (IsValid(SideLeftMarker))
	{
		SideLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideSlotClicked);
	}
	if (IsValid(SideRightMarker))
	{
		SideRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSideSlotClicked);
	}
	if (IsValid(ReboundLeftMarker))
	{
		ReboundLeftMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundSlotClicked);
	}
	if (IsValid(ReboundRightMarker))
	{
		ReboundRightMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleReboundSlotClicked);
	}
	if (IsValid(SpecialCenterMarker))
	{
		SpecialCenterMarker->OnClicked.AddUniqueDynamic(this, &UPBBumperEquipUI::HandleSpecialSlotClicked);
	}

	RefreshBoardSlotSelection();
}

void UPBBumperEquipUI::RefreshBoardSlotSelection()
{
	SetBoardSlotButtonState(TopLeftMarker, EPBBumperSlotType::Top);
	SetBoardSlotButtonState(TopRightMarker, EPBBumperSlotType::Top);
	SetBoardSlotButtonState(SideLeftMarker, EPBBumperSlotType::Side);
	SetBoardSlotButtonState(SideRightMarker, EPBBumperSlotType::Side);
	SetBoardSlotButtonState(ReboundLeftMarker, EPBBumperSlotType::Rebound);
	SetBoardSlotButtonState(ReboundRightMarker, EPBBumperSlotType::Rebound);
	SetBoardSlotButtonState(SpecialCenterMarker, EPBBumperSlotType::Special);
}

void UPBBumperEquipUI::SetBoardSlotButtonState(
	UButton* Button,
	const EPBBumperSlotType SlotType) const
{
	if (!IsValid(Button))
	{
		return;
	}

	FLinearColor SlotColor;
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
		SlotColor = FLinearColor(0.92f, 0.12f, 0.16f, 1.0f);
		break;
	case EPBBumperSlotType::Side:
		SlotColor = FLinearColor(0.08f, 0.35f, 0.95f, 1.0f);
		break;
	case EPBBumperSlotType::Rebound:
		SlotColor = FLinearColor(1.0f, 0.34f, 0.06f, 1.0f);
		break;
	case EPBBumperSlotType::Special:
		SlotColor = FLinearColor(0.10f, 0.68f, 0.24f, 1.0f);
		break;
	default:
		SlotColor = FLinearColor::White;
		break;
	}

	SlotColor.A = SelectedBumperSlotType == SlotType ? 1.0f : 0.38f;
	Button->SetBackgroundColor(SlotColor);
}

void UPBBumperEquipUI::HandleTopSlotClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Top);
}

void UPBBumperEquipUI::HandleSideSlotClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Side);
}

void UPBBumperEquipUI::HandleReboundSlotClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Rebound);
}

void UPBBumperEquipUI::HandleSpecialSlotClicked()
{
	SelectBumperSlot(EPBBumperSlotType::Special);
}
