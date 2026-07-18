#include "PBBumperEquipController.h"

#include "PBBumperEquipUIBuilder.h"
#include "PBBumperInfoPanelViewModel.h"
#include "PBBumperListItemObject.h"
#include "Engine/GameInstance.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/PBAssetBundleNames.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"

namespace
{
	constexpr EPBBumperEquipSlot EquipSlots[] =
	{
		EPBBumperEquipSlot::TopLeft,
		EPBBumperEquipSlot::TopRight,
		EPBBumperEquipSlot::SideLeft,
		EPBBumperEquipSlot::SideRight,
		EPBBumperEquipSlot::ReboundLeft,
		EPBBumperEquipSlot::ReboundRight,
		EPBBumperEquipSlot::Special
	};
}

void UPBBumperEquipController::Initialize(UGameInstance* InGameInstance)
{
	if (bInitialized || !IsValid(InGameInstance))
	{
		return;
	}

	bInitialized = true;
	CacheRequiredSubsystems(InGameInstance);
	EnsureInfoPanelViewModel();
	BindDataLoadEvents();

	if (LoadBumperRowsOnce())
	{
		RequestBumperUIAssetsAsync();
	}
}

void UPBBumperEquipController::Shutdown()
{
	if (!bInitialized)
	{
		return;
	}

	UnbindDataLoadEvents();
	bInitialized = false;
}

void UPBBumperEquipController::GetBumperListItemObjects(
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

bool UPBBumperEquipController::SelectBumperSlot(const EPBBumperSlotType SlotType)
{
	switch (SlotType)
	{
	case EPBBumperSlotType::Top:
	case EPBBumperSlotType::Side:
	case EPBBumperSlotType::Rebound:
	case EPBBumperSlotType::Special:
		return SelectBumperEquipSlot(PBBumperEquipSlotUtils::GetDefaultEquipSlot(SlotType));
	default:
		return false;
	}
}

bool UPBBumperEquipController::SelectBumperEquipSlot(const EPBBumperEquipSlot EquipSlot)
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

	OnSelectionChanged.Broadcast();
	return true;
}

void UPBBumperEquipController::SelectBumperRow(const FName RowName)
{
	if (RowName.IsNone() || !FindBumperRow(RowName))
	{
		return;
	}

	SelectedBumperRowName = RowName;

	EPBBumperSlotType BumperSlotType;
	if (PBBumperEquipUIBuilder::TryGetBumperSlotTypeByRowName(
		BumperRowNames,
		BumperRows,
		RowName,
		BumperSlotType))
	{
		EPBBumperSlotType CurrentSlotType;
		if (!PBBumperEquipSlotUtils::TryGetSlotType(SelectedBumperEquipSlot, CurrentSlotType)
			|| CurrentSlotType != BumperSlotType)
		{
			SelectedBumperEquipSlot = PBBumperEquipSlotUtils::GetDefaultEquipSlot(BumperSlotType);
		}
	}

	UpdateInfoPanelByRowName(RowName);
	OnSelectionChanged.Broadcast();
}

bool UPBBumperEquipController::EquipBumperRow(const FName RowName)
{
	SelectBumperRow(RowName);
	return EquipBumperRowAtSlot(RowName, SelectedBumperEquipSlot);
}

bool UPBBumperEquipController::EquipBumperRowAtSlot(
	const FName RowName,
	const EPBBumperEquipSlot EquipSlot)
{
	if (!IsValid(PlayerDataSubsystem)
		|| !CanEquipBumperRowAtSlot(RowName, EquipSlot)
		|| !PlayerDataSubsystem->EquipBumperAtSlot(EquipSlot, RowName))
	{
		return false;
	}

	SelectedBumperEquipSlot = EquipSlot;
	SelectedBumperRowName = RowName;
	RefreshBumperEquipState(RowName);
	return true;
}

bool UPBBumperEquipController::UnequipBumperRow(const FName RowName)
{
	if (RowName.IsNone() || !IsValid(PlayerDataSubsystem))
	{
		return false;
	}

	EPBBumperSlotType BumperSlotType;
	if (!PBBumperEquipUIBuilder::TryGetBumperSlotTypeByRowName(
		BumperRowNames,
		BumperRows,
		RowName,
		BumperSlotType))
	{
		return false;
	}

	SelectBumperRow(RowName);
	FName EquippedRowName = NAME_None;
	if (GetSelectedBumperSlotType() != BumperSlotType
		|| !PlayerDataSubsystem->GetEquippedBumperAtSlot(SelectedBumperEquipSlot, EquippedRowName)
		|| EquippedRowName != RowName
		|| !PlayerDataSubsystem->UnequipBumperAtSlot(SelectedBumperEquipSlot))
	{
		return false;
	}

	RefreshBumperEquipState(RowName);
	return true;
}

bool UPBBumperEquipController::EquipSelectedBumper()
{
	return EquipBumperRow(SelectedBumperRowName);
}

bool UPBBumperEquipController::UnequipSelectedBumper()
{
	return UnequipBumperRow(SelectedBumperRowName);
}

EPBBumperSlotType UPBBumperEquipController::GetSelectedBumperSlotType() const
{
	EPBBumperSlotType SlotType = EPBBumperSlotType::Top;
	PBBumperEquipSlotUtils::TryGetSlotType(SelectedBumperEquipSlot, SlotType);
	return SlotType;
}

bool UPBBumperEquipController::GetEquippedBumperForSlot(
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

bool UPBBumperEquipController::GetEquippedBumperForEquipSlot(
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

int32 UPBBumperEquipController::GetEquippedBumperCount() const
{
	int32 EquippedCount = 0;
	for (const EPBBumperEquipSlot EquipSlot : EquipSlots)
	{
		FName RowName = NAME_None;
		EquippedCount += GetEquippedBumperForEquipSlot(EquipSlot, RowName) && !RowName.IsNone() ? 1 : 0;
	}
	return EquippedCount;
}

const FPBBumperTableRow* UPBBumperEquipController::FindBumperRow(const FName RowName) const
{
	return PBBumperEquipUIBuilder::FindBumperRow(BumperRowNames, BumperRows, RowName);
}

UTexture2D* UPBBumperEquipController::ResolveBumperIconTexture(
	const FName RowName,
	const FPBBumperTableRow& Row) const
{
	return PBBumperEquipUIBuilder::ResolveBumperIconTexture(GameDataLoadSubsystem.Get(), RowName, Row);
}

FText UPBBumperEquipController::ResolveBumperTriggerDescription(const FPBBumperTableRow& Row) const
{
	return PBBumperEquipUIBuilder::ResolveBumperTriggerDescription(TableDataSubsystem.Get(), Row);
}

FText UPBBumperEquipController::ResolveBumperEffectDescription(const FPBBumperTableRow& Row) const
{
	return PBBumperEquipUIBuilder::ResolveBumperEffectDescription(TableDataSubsystem.Get(), Row);
}

bool UPBBumperEquipController::CanEquipBumperRowAtSlot(
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
		&& BumperSlotType == TargetSlotType
		&& (!IsValid(PlayerDataSubsystem)
			|| !PlayerDataSubsystem->IsBumperEquippedInAnotherSlot(RowName, EquipSlot));
}

bool UPBBumperEquipController::IsSelectedBumperEquippedInCurrentSlot() const
{
	FName EquippedRowName = NAME_None;
	return !SelectedBumperRowName.IsNone()
		&& GetEquippedBumperForEquipSlot(SelectedBumperEquipSlot, EquippedRowName)
		&& EquippedRowName == SelectedBumperRowName;
}

bool UPBBumperEquipController::IsBumperEquippedInAnotherSlot(
	const FName RowName,
	const EPBBumperEquipSlot TargetSlot) const
{
	return IsValid(PlayerDataSubsystem)
		&& PlayerDataSubsystem->IsBumperEquippedInAnotherSlot(RowName, TargetSlot);
}

void UPBBumperEquipController::HandleStartupGameDataLoaded()
{
	if (bInitialized && LoadBumperRowsOnce())
	{
		RequestBumperUIAssetsAsync();
	}
}

void UPBBumperEquipController::HandleBumperUIAssetsLoaded()
{
	if (bBumperListItemObjectsBuilt || !bBumperUIAssetLoadPending)
	{
		return;
	}

	bBumperUIAssetLoadPending = false;
	BuildBumperListItemObjects();
	SelectBumperEquipSlot(SelectedBumperEquipSlot);
	if (bInitialized)
	{
		OnCatalogReady.Broadcast();
	}
}

void UPBBumperEquipController::CacheRequiredSubsystems(UGameInstance* GameInstance)
{
	if (!IsValid(GameInstance))
	{
		return;
	}

	TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>();
	PlayerDataSubsystem = GameInstance->GetSubsystem<UPBPlayerDataSubsystem>();
	GameDataLoadSubsystem = GameInstance->GetSubsystem<UPBGameDataLoadSubsystem>();
}

void UPBBumperEquipController::BindDataLoadEvents()
{
	if (IsValid(TableDataSubsystem))
	{
		TableDataSubsystem->OnStartupGameDataLoaded.AddUniqueDynamic(
			this,
			&ThisClass::HandleStartupGameDataLoaded);
	}
}

void UPBBumperEquipController::UnbindDataLoadEvents()
{
	if (IsValid(TableDataSubsystem))
	{
		TableDataSubsystem->OnStartupGameDataLoaded.RemoveDynamic(
			this,
			&ThisClass::HandleStartupGameDataLoaded);
	}
}

void UPBBumperEquipController::EnsureInfoPanelViewModel()
{
	if (!IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel = NewObject<UPBBumperInfoPanelViewModel>(this);
	}
}

bool UPBBumperEquipController::LoadBumperRowsOnce()
{
	if (bBumperRowsLoaded)
	{
		return true;
	}

	if (!IsValid(TableDataSubsystem) || !TableDataSubsystem->IsBumperTableReady())
	{
		return false;
	}

	bBumperRowsLoaded = TableDataSubsystem->GetAllBumperRows(BumperRowNames, BumperRows)
		&& BumperRowNames.Num() == BumperRows.Num();
	return bBumperRowsLoaded;
}

void UPBBumperEquipController::RequestBumperUIAssetsAsync()
{
	if (bBumperListItemObjectsBuilt
		|| bBumperUIAssetLoadPending)
	{
		return;
	}
	if (!IsValid(GameDataLoadSubsystem) || BumperRowNames.IsEmpty())
	{
		BuildBumperListItemObjects();
		SelectBumperEquipSlot(SelectedBumperEquipSlot);
		OnCatalogReady.Broadcast();
		return;
	}

	TArray<FPrimaryAssetId> BumperAssetIds;
	BumperAssetIds.Reserve(BumperRowNames.Num());
	for (const FName RowName : BumperRowNames)
	{
		if (!RowName.IsNone())
		{
			BumperAssetIds.Emplace(PBBumperAssetIds::Type::BumperData, RowName);
		}
	}
	if (BumperAssetIds.IsEmpty())
	{
		return;
	}

	bBumperUIAssetLoadPending = true;
	GameDataLoadSubsystem->LoadPrimaryAssetsByIdsAsync(
		BumperAssetIds,
		{PBAssetBundleNames::UI},
		FStreamableDelegate::CreateUObject(this, &ThisClass::HandleBumperUIAssetsLoaded));
}

void UPBBumperEquipController::BuildBumperListItemObjects()
{
	if (bBumperListItemObjectsBuilt)
	{
		return;
	}

	TopItems.Reset();
	SideItems.Reset();
	ReboundItems.Reset();
	SpecialItems.Reset();

	PBBumperEquipUIBuilder::BuildBumperListItemObjects(
		this,
		BumperRowNames,
		BumperRows,
		PBBumperEquipUIBuilder::MakeEquippedBumperRowIdSet(PlayerDataSubsystem.Get()),
		GameDataLoadSubsystem.Get(),
		TopItems,
		SideItems,
		ReboundItems,
		SpecialItems);
	BindListItemSelectionEvents();
	bBumperListItemObjectsBuilt = true;
	if (IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel->ClearBumperInfoPanelData();
	}
}

void UPBBumperEquipController::BindListItemSelectionEvents()
{
	const auto BindItems = [this](const TArray<TObjectPtr<UPBBumperListItemObject>>& Items)
	{
		for (UPBBumperListItemObject* Item : Items)
		{
			if (IsValid(Item))
			{
				Item->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectBumperRow);
			}
		}
	};

	BindItems(TopItems);
	BindItems(SideItems);
	BindItems(ReboundItems);
	BindItems(SpecialItems);
}

void UPBBumperEquipController::UpdateInfoPanelByRowName(const FName RowName)
{
	const FPBBumperTableRow* Row = FindBumperRow(RowName);
	if (!Row)
	{
		if (IsValid(InfoPanelViewModel))
		{
			InfoPanelViewModel->ClearBumperInfoPanelData();
		}
		return;
	}

	FName EquippedRowName = NAME_None;
	const bool bIsEquippedInSelectedSlot = GetEquippedBumperForEquipSlot(
		SelectedBumperEquipSlot,
		EquippedRowName) && EquippedRowName == RowName;
	if (IsValid(InfoPanelViewModel))
	{
		InfoPanelViewModel->SetBumperInfoPanelData(
			Row->DisplayName,
			ResolveBumperIconTexture(RowName, *Row),
			PBBumperEquipUIBuilder::ResolveBumperDescription(TableDataSubsystem.Get(), *Row),
			bIsEquippedInSelectedSlot);
	}
}

void UPBBumperEquipController::UpdateBumperListEquipStates()
{
	PBBumperEquipUIBuilder::UpdateBumperListEquipStates(
		PBBumperEquipUIBuilder::MakeEquippedBumperRowIdSet(PlayerDataSubsystem.Get()),
		TopItems,
		SideItems,
		ReboundItems,
		SpecialItems);
}

void UPBBumperEquipController::RefreshBumperEquipState(const FName RowName)
{
	UpdateBumperListEquipStates();
	SelectBumperRow(RowName);
}
