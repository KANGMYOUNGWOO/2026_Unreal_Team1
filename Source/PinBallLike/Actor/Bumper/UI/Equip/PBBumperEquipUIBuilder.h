#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperListItemObject.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"

class UPBGameDataLoadSubsystem;
class UPBPlayerDataSubsystem;
class UPBTableDataSubsystem;
class UPBBumperEquipUI;
class UTexture2D;
enum class EPBBumperSlotType : uint8;

namespace PBBumperEquipUIBuilder
{
	void AppendListItemObjects(
		const TArray<TObjectPtr<UPBBumperListItemObject>>& SourceItems,
		TArray<UPBBumperListItemObject*>& TargetItems);
	const FPBBumperTableRow* FindBumperRow(
		const TArray<FName>& RowNames,
		const TArray<FPBBumperTableRow>& Rows,
		FName RowName);
	void BuildBumperListItemObjects(
		UObject* Outer,
		UPBBumperEquipUI* ClickHandler,
		const TArray<FName>& RowNames,
		const TArray<FPBBumperTableRow>& Rows,
		const TSet<FName>& EquippedRowIds,
		const UPBGameDataLoadSubsystem* GameDataLoadSubsystem,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutReboundItems,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutSideItems,
		TArray<TObjectPtr<UPBBumperListItemObject>>& OutTopTargetItems);
	void UpdateBumperListEquipStates(
		const TSet<FName>& EquippedRowIds,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& ReboundItems,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& SideItems,
		const TArray<TObjectPtr<UPBBumperListItemObject>>& TopTargetItems);
	TSet<FName> MakeEquippedBumperRowIdSet(const UPBPlayerDataSubsystem* PlayerDataSubsystem);
	bool IsBumperEquipped(FName RowName, const TSet<FName>& EquippedRowIds);
	bool TryGetBumperSlotType(const FPBBumperTableRow& Row, EPBBumperSlotType& OutSlotType);
	bool TryGetBumperSlotTypeByRowName(
		const TArray<FName>& RowNames,
		const TArray<FPBBumperTableRow>& Rows,
		FName RowName,
		EPBBumperSlotType& OutSlotType);
	FText ResolveBumperDescription(const UPBTableDataSubsystem* TableDataSubsystem, const FPBBumperTableRow& Row);
	UTexture2D* ResolveBumperIconTexture(
		const UPBGameDataLoadSubsystem* GameDataLoadSubsystem,
		FName RowName,
		const FPBBumperTableRow& Row);
}
