#include "PBBumperEquipUIBuilder.h"

#include "PBBumperEquipUI.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"

namespace PBBumperEquipUIBuilder
{
	namespace
	{
		void AddBumperListItemObject(
			UObject* Outer,
			UPBBumperEquipUI* ClickHandler,
			const FName RowName,
			const FPBBumperTableRow& Row,
			const TSet<FName>& EquippedRowIds,
			const UPBGameDataLoadSubsystem* GameDataLoadSubsystem,
			TArray<TObjectPtr<UPBBumperListItemObject>>& OutReboundItems,
			TArray<TObjectPtr<UPBBumperListItemObject>>& OutSideItems,
			TArray<TObjectPtr<UPBBumperListItemObject>>& OutTopTargetItems)
		{
			UPBBumperListItemObject* ItemObject = NewObject<UPBBumperListItemObject>(Outer);
			if (!IsValid(ItemObject))
			{
				return;
			}

			const FPrimaryAssetId PrimaryAssetId(PBBumperAssetIds::Type::BumperData, RowName);
			ItemObject->SetBumperListItemData(
				RowName,
				Row.DisplayName,
				Row.BumperType,
				PrimaryAssetId,
				ResolveBumperIconTexture(GameDataLoadSubsystem, RowName, Row),
				IsBumperEquipped(RowName, EquippedRowIds));
			ItemObject->OnClicked.AddDynamic(ClickHandler, &UPBBumperEquipUI::SelectBumperRow);

			switch (Row.BumperType)
			{
			case EPBBumperType::Rebound:
				OutReboundItems.Add(ItemObject);
				break;
			case EPBBumperType::Side:
				OutSideItems.Add(ItemObject);
				break;
			case EPBBumperType::Gate:
			case EPBBumperType::TopTarget:
				OutTopTargetItems.Add(ItemObject);
				break;
			default:
				break;
			}
		}
	}

	void AppendListItemObjects(
		const TArray<TObjectPtr<UPBBumperListItemObject>>& SourceItems,
		TArray<UPBBumperListItemObject*>& TargetItems)
	{
		for (const TObjectPtr<UPBBumperListItemObject>& ItemObject : SourceItems)
		{
			TargetItems.Add(ItemObject.Get());
		}
	}

	const FPBBumperTableRow* FindBumperRow(
		const TArray<FName>& RowNames,
		const TArray<FPBBumperTableRow>& Rows,
		const FName RowName)
	{
		const int32 RowIndex = RowNames.IndexOfByKey(RowName);
		if (!Rows.IsValidIndex(RowIndex))
		{
			return nullptr;
		}

		return &Rows[RowIndex];
	}

	void BuildBumperListItemObjects(
		UObject* Outer,
		UPBBumperEquipUI* ClickHandler,
		const TArray<FName>& RowNames,
		const TArray<FPBBumperTableRow>& Rows,
		const TSet<FName>& EquippedRowIds,
		const UPBGameDataLoadSubsystem* GameDataLoadSubsystem,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutReboundItems,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutSideItems,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutTopTargetItems)
	{
		for (int32 RowIndex = 0; RowIndex < Rows.Num(); ++RowIndex)
		{
			AddBumperListItemObject(
				Outer,
				ClickHandler,
				RowNames[RowIndex],
				Rows[RowIndex],
				EquippedRowIds,
				GameDataLoadSubsystem,
				OutReboundItems,
				OutSideItems,
				OutTopTargetItems);
		}
	}

	void UpdateBumperListEquipStates(
		const TSet<FName>& EquippedRowIds,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& ReboundItems,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& SideItems,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& TopTargetItems)
	{
		auto UpdateItems = [&EquippedRowIds](const TArray<TObjectPtr<UPBBumperListItemObject>>& Items)
		{
			for (const TObjectPtr<UPBBumperListItemObject>& ItemObject : Items)
			{
				if (IsValid(ItemObject.Get()))
				{
					ItemObject->SetEquip(IsBumperEquipped(ItemObject->RowName, EquippedRowIds));
				}
			}
		};

		UpdateItems(ReboundItems);
		UpdateItems(SideItems);
		UpdateItems(TopTargetItems);
	}

	TSet<FName> MakeEquippedBumperRowIdSet(const UPBPlayerDataSubsystem* PlayerDataSubsystem)
	{
		TSet<FName> EquippedRowIds;
		if (!IsValid(PlayerDataSubsystem))
		{
			return EquippedRowIds;
		}

		for (const FName& EquippedRowId : PlayerDataSubsystem->GetEquippedBumperRowIds())
		{
			EquippedRowIds.Add(EquippedRowId);
		}

		return EquippedRowIds;
	}

	bool IsBumperEquipped(const FName RowName, const TSet<FName>& EquippedRowIds)
	{
		return !RowName.IsNone() && EquippedRowIds.Contains(RowName);
	}

	bool TryGetBumperSlotType(const FPBBumperTableRow& Row, EPBBumperSlotType& OutSlotType)
	{
		switch (Row.BumperType)
		{
		case EPBBumperType::Rebound:
			OutSlotType = EPBBumperSlotType::Rebound;
			return true;
		case EPBBumperType::Side:
			OutSlotType = EPBBumperSlotType::Side;
			return true;
		case EPBBumperType::Gate:
		case EPBBumperType::TopTarget:
			OutSlotType = EPBBumperSlotType::Special;
			return true;
		default:
			break;
		}

		return false;
	}

	bool TryGetBumperSlotTypeByRowName(
		const TArray<FName>& RowNames,
		const TArray<FPBBumperTableRow>& Rows,
		const FName RowName,
		EPBBumperSlotType& OutSlotType)
	{
		const FPBBumperTableRow* Row = FindBumperRow(RowNames, Rows, RowName);
		if (!Row)
		{
			return false;
		}

		return TryGetBumperSlotType(*Row, OutSlotType);
	}

	FText ResolveBumperDescription(
		const UPBTableDataSubsystem* TableDataSubsystem,
		const FPBBumperTableRow& Row)
	{
		if (!IsValid(TableDataSubsystem))
		{
			return Row.Description;
		}

		FText TriggerDescription;
		if (!Row.TriggerID.IsNone())
		{
			FPBBumperTriggerRow TriggerRow;
			if (TableDataSubsystem->FindBumperTriggerRow(Row.TriggerID, TriggerRow))
			{
				TriggerDescription = TriggerRow.TriggerDescription;
			}
		}

		FText EffectDescription;
		if (!Row.EffectID.IsNone())
		{
			FPBBumperEffectRow EffectRow;
			if (TableDataSubsystem->FindBumperEffectRow(Row.EffectID, EffectRow))
			{
				EffectDescription = EffectRow.Description;
			}
		}

		const bool bHasTriggerDescription = !TriggerDescription.IsEmptyOrWhitespace();
		const bool bHasEffectDescription = !EffectDescription.IsEmptyOrWhitespace();
		if (bHasTriggerDescription && bHasEffectDescription)
		{
			return FText::Format(
				NSLOCTEXT("PBBumperEquipUIDataHelper", "TriggerEffectDescriptionFormat", "{0}\n{1}"),
				TriggerDescription,
				EffectDescription);
		}

		if (bHasTriggerDescription)
		{
			return TriggerDescription;
		}

		if (bHasEffectDescription)
		{
			return EffectDescription;
		}

		return Row.Description;
	}

	UTexture2D* ResolveBumperIconTexture(
		const UPBGameDataLoadSubsystem* GameDataLoadSubsystem,
		const FName RowName,
		const FPBBumperTableRow& Row)
	{
		if (const UPBBumperDataAsset* BumperDataAsset = Row.BumperDataAsset.Get())
		{
			return BumperDataAsset->Icon.Get();
		}

		if (IsValid(GameDataLoadSubsystem))
		{
			const FPrimaryAssetId PrimaryAssetId(PBBumperAssetIds::Type::BumperData, RowName);
			const UPBBumperDataAsset* BumperDataAsset =
				Cast<UPBBumperDataAsset>(GameDataLoadSubsystem->GetLoadedPrimaryAsset(PrimaryAssetId));
			if (IsValid(BumperDataAsset))
			{
				return BumperDataAsset->Icon.Get();
			}
		}

		return nullptr;
	}
}
