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

void UPBBumperEquipUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

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
	TArray<UPBBumperListItemObject*>& OutReboundItems,
	TArray<UPBBumperListItemObject*>& OutSideItems,
	TArray<UPBBumperListItemObject*>& OutTopTargetItems) const
{
	OutReboundItems.Reset();
	OutSideItems.Reset();
	OutTopTargetItems.Reset();

	PBBumperEquipUIBuilder::AppendListItemObjects(ReboundItems, OutReboundItems);
	PBBumperEquipUIBuilder::AppendListItemObjects(SideItems, OutSideItems);
	PBBumperEquipUIBuilder::AppendListItemObjects(TopTargetItems, OutTopTargetItems);
}

void UPBBumperEquipUI::SelectBumperRow(const FName RowName)
{
	if (RowName.IsNone())
	{
		return;
	}

	SelectedBumperRowName = RowName;
	UpdateInfoPanelByRowName(RowName);
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
	if (bBumperListItemObjectsBuilt || !bBumperUIAssetLoadRequested)
	{
		return;
	}

	BuildBumperListItemObjects();
	OnBumperListItemsReady.Broadcast();
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

	if (IsValid(GameDataLoadSubsystem))
	{
		GameDataLoadSubsystem->OnPrimaryAssetsLoaded.AddUniqueDynamic(
			this,
			&UPBBumperEquipUI::HandleBumperUIAssetsLoaded);
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

	if (IsValid(GameDataLoadSubsystem))
	{
		GameDataLoadSubsystem->OnPrimaryAssetsLoaded.RemoveDynamic(
			this,
			&UPBBumperEquipUI::HandleBumperUIAssetsLoaded);
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
	if (bBumperUIAssetLoadRequested || !IsValid(GameDataLoadSubsystem) || BumperRowNames.IsEmpty())
	{
		return;
	}

	TArray<FName> BundleNames;
	BundleNames.Add(PBAssetBundleNames::UI);

	bBumperUIAssetLoadRequested = true;
	GameDataLoadSubsystem->LoadPrimaryAssetsByNamesAsync(
		PBBumperAssetIds::Type::BumperData,
		BumperRowNames,
		BundleNames);
}

void UPBBumperEquipUI::BuildBumperListItemObjects()
{
	if (bBumperListItemObjectsBuilt)
	{
		return;
	}

	ReboundItems.Reset();
	SideItems.Reset();
	TopTargetItems.Reset();

	const TSet<FName> EquippedRowIds = PBBumperEquipUIBuilder::MakeEquippedBumperRowIdSet(PlayerDataSubsystem.Get());

	PBBumperEquipUIBuilder::BuildBumperListItemObjects(
		this, this, BumperRowNames, BumperRows, EquippedRowIds, GameDataLoadSubsystem.Get(),
		ReboundItems, SideItems, TopTargetItems);

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
	PBBumperEquipUIBuilder::UpdateBumperListEquipStates(EquippedRowIds, ReboundItems, SideItems, TopTargetItems);
}

void UPBBumperEquipUI::RefreshBumperEquipState(const FName RowName)
{
	SelectedBumperRowName = RowName;
	UpdateBumperListEquipStates();
	UpdateInfoPanelByRowName(RowName);
}
