#include "PBBumperEquipUIBuilder.h"

#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Utils/PBTextFormatUtils.h"

namespace PBBumperEquipUIBuilder
{
	namespace
	{
		void AddBumperListItemObject(
			UObject* Outer,
			const FName RowName,
			const FPBBumperTableRow& Row,
			const TSet<FName>& EquippedRowIds,
			const UPBGameDataLoadSubsystem* GameDataLoadSubsystem,
			TArray<TObjectPtr<UPBBumperListItemObject>>& OutTopItems,
			TArray<TObjectPtr<UPBBumperListItemObject>>& OutSideItems,
			TArray<TObjectPtr<UPBBumperListItemObject>>& OutReboundItems,
			TArray<TObjectPtr<UPBBumperListItemObject>>& OutSpecialItems)
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
			switch (Row.BumperType)
			{
			case EPBBumperType::TopTarget:
				OutTopItems.Add(ItemObject);
				break;
			case EPBBumperType::Rebound:
				OutReboundItems.Add(ItemObject);
				break;
			case EPBBumperType::Side:
				OutSideItems.Add(ItemObject);
				break;
			case EPBBumperType::Gate:
				OutSpecialItems.Add(ItemObject);
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
		const TArray<FName>& RowNames,
		const TArray<FPBBumperTableRow>& Rows,
		const TSet<FName>& EquippedRowIds,
		const UPBGameDataLoadSubsystem* GameDataLoadSubsystem,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutTopItems,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutSideItems,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutReboundItems,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutSpecialItems)
	{
		for (int32 RowIndex = 0; RowIndex < Rows.Num(); ++RowIndex)
		{
			AddBumperListItemObject(
				Outer,
				RowNames[RowIndex],
				Rows[RowIndex],
				EquippedRowIds,
				GameDataLoadSubsystem,
				OutTopItems,
				OutSideItems,
				OutReboundItems,
				OutSpecialItems);
		}
	}

	void UpdateBumperListEquipStates(
		const TSet<FName>& EquippedRowIds,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& TopItems,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& SideItems,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& ReboundItems,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& SpecialItems)
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

		UpdateItems(TopItems);
		UpdateItems(SideItems);
		UpdateItems(ReboundItems);
		UpdateItems(SpecialItems);
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
		case EPBBumperType::TopTarget:
			OutSlotType = EPBBumperSlotType::Top;
			return true;
		case EPBBumperType::Rebound:
			OutSlotType = EPBBumperSlotType::Rebound;
			return true;
		case EPBBumperType::Side:
			OutSlotType = EPBBumperSlotType::Side;
			return true;
		case EPBBumperType::Gate:
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

	FText GetBumperSlotTypeDisplayName(const EPBBumperSlotType SlotType)
	{
		switch (SlotType)
		{
		case EPBBumperSlotType::Top:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "TopSlot", "탑");
		case EPBBumperSlotType::Side:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SideSlot", "사이드");
		case EPBBumperSlotType::Rebound:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "ReboundSlot", "리바운드");
		case EPBBumperSlotType::Special:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SpecialSlot", "스페셜");
		default:
			return FText::GetEmpty();
		}
	}

	FText GetBumperEquipSlotDisplayName(const EPBBumperEquipSlot EquipSlot)
	{
		switch (EquipSlot)
		{
		case EPBBumperEquipSlot::TopLeft:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "TopLeftSlot", "탑 왼쪽");
		case EPBBumperEquipSlot::TopRight:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "TopRightSlot", "탑 오른쪽");
		case EPBBumperEquipSlot::SideLeft:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SideLeftSlot", "사이드 왼쪽");
		case EPBBumperEquipSlot::SideRight:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SideRightSlot", "사이드 오른쪽");
		case EPBBumperEquipSlot::ReboundLeft:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "ReboundLeftSlot", "리바운드 왼쪽");
		case EPBBumperEquipSlot::ReboundRight:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "ReboundRightSlot", "리바운드 오른쪽");
		case EPBBumperEquipSlot::Special:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SpecialEquipSlot", "스페셜 중앙");
		default:
			return FText::GetEmpty();
		}
	}

	FText GetBumperRoleDisplayName(const EPBBumperRoleType RoleType)
	{
		switch (RoleType)
		{
		case EPBBumperRoleType::Attack:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "AttackRole", "공격형");
		case EPBBumperRoleType::Spawn:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SpawnRole", "생성형");
		case EPBBumperRoleType::Support:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SupportRole", "지원형");
		case EPBBumperRoleType::Zone:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "ZoneRole", "영역형");
		default:
			return FText::GetEmpty();
		}
	}

	FText GetBumperEffectTypeDisplayName(const EPBBumperEffectType EffectType)
	{
		switch (EffectType)
		{
		case EPBBumperEffectType::Instant:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "InstantEffect", "즉시 효과");
		case EPBBumperEffectType::Buff:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "BuffEffect", "버프");
		case EPBBumperEffectType::Area:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "AreaEffect", "영역 효과");
		case EPBBumperEffectType::Summon:
			return NSLOCTEXT("PBBumperEquipUIBuilder", "SummonEffect", "생성 효과");
		default:
			return FText::GetEmpty();
		}
	}

	FText ResolveBumperTriggerDescription(
		const UPBTableDataSubsystem* TableDataSubsystem,
		const FPBBumperTableRow& Row)
	{
		if (!IsValid(TableDataSubsystem) || Row.TriggerID.IsNone())
		{
			return FText::GetEmpty();
		}

		FPBBumperTriggerRow TriggerRow;
		if (!TableDataSubsystem->FindBumperTriggerRow(Row.TriggerID, TriggerRow))
		{
			return FText::GetEmpty();
		}

		return PBTextFormatUtils::FormatSingleValueTemplate(
			TriggerRow.TriggerDescription,
			FText::AsNumber(Row.RequiredTriggerCount));
	}

	FText ResolveBumperEffectDescription(
		const UPBTableDataSubsystem* TableDataSubsystem,
		const FPBBumperTableRow& Row)
	{
		if (!IsValid(TableDataSubsystem) || Row.EffectID.IsNone())
		{
			return FText::GetEmpty();
		}

		FPBBumperEffectRow EffectRow;
		if (!TableDataSubsystem->FindBumperEffectRow(Row.EffectID, EffectRow))
		{
			return FText::GetEmpty();
		}

		return PBTextFormatUtils::FormatSingleValueTemplate(
			EffectRow.Description,
			FText::AsNumber(EffectRow.Power));
	}

	FText ResolveBumperDescription(
		const UPBTableDataSubsystem* TableDataSubsystem,
		const FPBBumperTableRow& Row)
	{
		const FText TriggerDescription = ResolveBumperTriggerDescription(TableDataSubsystem, Row);
		const FText EffectDescription = ResolveBumperEffectDescription(TableDataSubsystem, Row);

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
