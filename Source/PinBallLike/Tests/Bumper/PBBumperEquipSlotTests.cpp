#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "PinBallLike/Actor/Bumper/Save/PBBumperLoadoutSaveGame.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperEquipSlotMappingTest,
	"PinBallLike.Bumper.Equipment.SlotMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperEquipSlotMappingTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	struct FExpectedSlotMapping
	{
		EPBBumperEquipSlot EquipSlot;
		EPBBumperSlotType SlotType;
		EPBBumperPositionId PositionId;
		EPBBumperType BumperType;
	};

	const FExpectedSlotMapping ExpectedMappings[] =
	{
		{ EPBBumperEquipSlot::TopLeft, EPBBumperSlotType::Top, EPBBumperPositionId::TopTargetLeft, EPBBumperType::TopTarget },
		{ EPBBumperEquipSlot::TopRight, EPBBumperSlotType::Top, EPBBumperPositionId::TopTargetRight, EPBBumperType::TopTarget },
		{ EPBBumperEquipSlot::SideLeft, EPBBumperSlotType::Side, EPBBumperPositionId::SideLeft, EPBBumperType::Side },
		{ EPBBumperEquipSlot::SideRight, EPBBumperSlotType::Side, EPBBumperPositionId::SideRight, EPBBumperType::Side },
		{ EPBBumperEquipSlot::ReboundLeft, EPBBumperSlotType::Rebound, EPBBumperPositionId::ReboundLeft, EPBBumperType::Rebound },
		{ EPBBumperEquipSlot::ReboundRight, EPBBumperSlotType::Rebound, EPBBumperPositionId::ReboundRight, EPBBumperType::Rebound },
		{ EPBBumperEquipSlot::Special, EPBBumperSlotType::Special, EPBBumperPositionId::GateCenterMid, EPBBumperType::Gate }
	};

	for (const FExpectedSlotMapping& Expected : ExpectedMappings)
	{
		EPBBumperSlotType ActualSlotType = EPBBumperSlotType::Special;
		EPBBumperPositionId ActualPositionId = EPBBumperPositionId::None;
		TestTrue(TEXT("Physical slot maps to a category"),
			PBBumperEquipSlotUtils::TryGetSlotType(Expected.EquipSlot, ActualSlotType));
		TestTrue(TEXT("Physical slot maps to a board position"),
			PBBumperEquipSlotUtils::TryGetPositionId(Expected.EquipSlot, ActualPositionId));
		TestEqual(TEXT("Mapped category is correct"),
			static_cast<uint8>(ActualSlotType),
			static_cast<uint8>(Expected.SlotType));
		TestEqual(TEXT("Mapped board position is correct"),
			static_cast<uint8>(ActualPositionId),
			static_cast<uint8>(Expected.PositionId));
		TestTrue(TEXT("Expected Bumper type is accepted by the physical slot"),
			PBBumperEquipSlotUtils::DoesBumperTypeMatchEquipSlot(
				Expected.BumperType,
				Expected.EquipSlot));
		const EPBBumperType MismatchedType = Expected.BumperType == EPBBumperType::Gate
			? EPBBumperType::Rebound
			: EPBBumperType::Gate;
		TestFalse(TEXT("Mismatched Bumper type is rejected by the physical slot"),
			PBBumperEquipSlotUtils::DoesBumperTypeMatchEquipSlot(
				MismatchedType,
				Expected.EquipSlot));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperIndependentEquipmentTest,
	"PinBallLike.Bumper.Equipment.IndependentSlots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperIndependentEquipmentTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPBPlayerDataSubsystem* PlayerData = NewObject<UPBPlayerDataSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Player data subsystem can be created for a focused test"), PlayerData))
	{
		return false;
	}

	const FName LeftRow(TEXT("Test_Rebound_Left"));
	const FName RightRow(TEXT("Test_Rebound_Right"));
	TestTrue(TEXT("Left slot accepts its own row"),
		PlayerData->EquipBumperAtSlot(EPBBumperEquipSlot::ReboundLeft, LeftRow));
	TestTrue(TEXT("Right slot accepts a different row"),
		PlayerData->EquipBumperAtSlot(EPBBumperEquipSlot::ReboundRight, RightRow));
	TestTrue(TEXT("Equipping the same row to its current slot is idempotent"),
		PlayerData->EquipBumperAtSlot(EPBBumperEquipSlot::ReboundLeft, LeftRow));
	TestFalse(TEXT("The current slot is not considered another slot"),
		PlayerData->IsBumperEquippedInAnotherSlot(LeftRow, EPBBumperEquipSlot::ReboundLeft));
	TestTrue(TEXT("The same row is detected from another target slot"),
		PlayerData->IsBumperEquippedInAnotherSlot(LeftRow, EPBBumperEquipSlot::ReboundRight));
	TestFalse(TEXT("The same row is rejected in another physical slot"),
		PlayerData->EquipBumperAtSlot(EPBBumperEquipSlot::ReboundRight, LeftRow));

	FName ActualRow = NAME_None;
	TestTrue(TEXT("Left slot can be queried"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::ReboundLeft, ActualRow));
	TestEqual(TEXT("Left slot keeps the left row"), ActualRow, LeftRow);
	TestTrue(TEXT("Right slot can be queried"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::ReboundRight, ActualRow));
	TestEqual(TEXT("Right slot keeps the right row"), ActualRow, RightRow);

	TestTrue(TEXT("Compatibility query prefers the left row"),
		PlayerData->GetEquippedBumper(EPBBumperSlotType::Rebound, ActualRow));
	TestEqual(TEXT("Compatibility query returned the left row"), ActualRow, LeftRow);

	TestTrue(TEXT("Only the left slot is removed"),
		PlayerData->UnequipBumperAtSlot(EPBBumperEquipSlot::ReboundLeft));
	TestFalse(TEXT("Removed left slot is empty"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::ReboundLeft, ActualRow));
	TestTrue(TEXT("Right slot remains equipped"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::ReboundRight, ActualRow));
	TestEqual(TEXT("Right slot still has its original row"), ActualRow, RightRow);

	TestTrue(TEXT("Compatibility query falls back to the right row"),
		PlayerData->GetEquippedBumper(EPBBumperSlotType::Rebound, ActualRow));
	TestEqual(TEXT("Compatibility fallback returned the right row"), ActualRow, RightRow);

	const FName SharedSideRow(TEXT("Test_Side_Shared"));
	TestTrue(TEXT("Legacy category equip remains supported"),
		PlayerData->EquipBumper(EPBBumperSlotType::Side, SharedSideRow));
	TestTrue(TEXT("Legacy equip populated the category default slot"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::SideLeft, ActualRow));
	TestEqual(TEXT("SideLeft has the shared row"), ActualRow, SharedSideRow);
	TestFalse(TEXT("Legacy equip does not create a duplicate in SideRight"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::SideRight, ActualRow));

	const TArray<FPBEquippedBumperSlot> EquippedSlots = PlayerData->GetEquippedBumperSlots();
	TestEqual(TEXT("Two physical slots are currently equipped"), EquippedSlots.Num(), 2);
	const TArray<FName> UniqueRows = PlayerData->GetEquippedBumperRowIds();
	TestEqual(TEXT("Every equipped row is unique"), UniqueRows.Num(), 2);
	TestTrue(TEXT("Removing the Rebound category clears its remaining slot"),
		PlayerData->UnequipBumper(EPBBumperSlotType::Rebound));
	TestTrue(TEXT("Removing the Side category clears its remaining slot"),
		PlayerData->UnequipBumper(EPBBumperSlotType::Side));
	TestTrue(TEXT("An empty Bumper loadout remains a valid stored state"),
		PlayerData->GetEquippedBumperSlots().IsEmpty());

	const EPBBumperSlotType InvalidSlotType = static_cast<EPBBumperSlotType>(255);
	TestFalse(TEXT("Invalid legacy category is rejected"),
		PlayerData->GetEquippedBumper(InvalidSlotType, ActualRow));
	TestTrue(TEXT("Invalid query clears the output row"), ActualRow.IsNone());

	struct FLegacyIdMigration
	{
		FName LegacyId;
		FName CurrentId;
	};
	const FLegacyIdMigration LegacyMigrations[] =
	{
		{PBBumperAssetIds::LegacyBumper::Rebound_CounterShell,
			PBBumperAssetIds::Bumper::Rebound_KineticShell},
		{PBBumperAssetIds::LegacyBumper::Rebound_ManaOrb,
			PBBumperAssetIds::Bumper::Rebound_BloodOverdrive},
		{PBBumperAssetIds::LegacyBumper::Side_LaunchCharge,
			PBBumperAssetIds::Bumper::Side_CounterShield},
		{PBBumperAssetIds::LegacyBumper::Top_ComboUp,
			PBBumperAssetIds::Bumper::Top_ComboArc},
		{PBBumperAssetIds::LegacyBumper::Top_ComboPickup,
			PBBumperAssetIds::Bumper::Top_VulnerabilityShell},
		{PBBumperAssetIds::LegacyBumper::Gate_ManaField,
			PBBumperAssetIds::Bumper::Gate_ReactiveRepair},
		{PBBumperAssetIds::LegacyBumper::Gate_StrengthField,
			PBBumperAssetIds::Bumper::Gate_ManaReactor}
	};
	for (const FLegacyIdMigration& Migration : LegacyMigrations)
	{
		TestEqual(
			*FString::Printf(TEXT("Legacy Bumper ID migrates: %s"), *Migration.LegacyId.ToString()),
			PBBumperAssetIds::NormalizeBumperRowId(Migration.LegacyId),
			Migration.CurrentId);
	}

	UPBPlayerDataSubsystem* MigrationPlayerData = NewObject<UPBPlayerDataSubsystem>(GameInstance);
	TestTrue(TEXT("Legacy equipment input is accepted"),
		MigrationPlayerData->EquipBumperAtSlot(
			EPBBumperEquipSlot::TopLeft,
			PBBumperAssetIds::LegacyBumper::Top_ComboUp));
	TestTrue(TEXT("Migrated equipment can be queried"),
		MigrationPlayerData->GetEquippedBumperAtSlot(
			EPBBumperEquipSlot::TopLeft,
			ActualRow));
	TestEqual(TEXT("Stored equipment uses the current RowName"),
		ActualRow,
		PBBumperAssetIds::Bumper::Top_ComboArc);
	TestFalse(TEXT("Legacy and current IDs cannot bypass duplicate equipment prevention"),
		MigrationPlayerData->EquipBumperAtSlot(
			EPBBumperEquipSlot::TopRight,
			PBBumperAssetIds::Bumper::Top_ComboArc));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperLoadoutSerializationTest,
	"PinBallLike.Bumper.Equipment.SaveGameSerialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperLoadoutSerializationTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UPBBumperLoadoutSaveGame* Source = NewObject<UPBBumperLoadoutSaveGame>();
	if (!TestNotNull(TEXT("Bumper loadout SaveGame can be created"), Source))
	{
		return false;
	}

	FPBEquippedBumperSlot& SourceSlot = Source->EquippedSlots.AddDefaulted_GetRef();
	SourceSlot.EquipSlot = EPBBumperEquipSlot::SideRight;
	SourceSlot.BumperRowId = PBBumperAssetIds::Bumper::Side_ManaCharge;

	TArray<uint8> SaveBytes;
	if (!TestTrue(
		TEXT("Bumper loadout serializes to memory"),
		UGameplayStatics::SaveGameToMemory(Source, SaveBytes)))
	{
		return false;
	}

	const UPBBumperLoadoutSaveGame* Restored = Cast<UPBBumperLoadoutSaveGame>(
		UGameplayStatics::LoadGameFromMemory(SaveBytes));
	if (!TestNotNull(TEXT("Serialized bumper loadout restores as the expected class"), Restored))
	{
		return false;
	}

	TestEqual(
		TEXT("Save version survives serialization"),
		Restored->SaveVersion,
		UPBBumperLoadoutSaveGame::CurrentSaveVersion);
	if (TestEqual(TEXT("Equipped slot count survives serialization"), Restored->EquippedSlots.Num(), 1))
	{
		TestEqual(
			TEXT("Physical slot survives serialization"),
			static_cast<uint8>(Restored->EquippedSlots[0].EquipSlot),
			static_cast<uint8>(EPBBumperEquipSlot::SideRight));
		TestEqual(
			TEXT("Bumper RowName survives serialization"),
			Restored->EquippedSlots[0].BumperRowId,
			PBBumperAssetIds::Bumper::Side_ManaCharge);
	}

	Source->EquippedSlots.Reset();
	SaveBytes.Reset();
	TestTrue(
		TEXT("An intentionally empty loadout serializes"),
		UGameplayStatics::SaveGameToMemory(Source, SaveBytes));
	const UPBBumperLoadoutSaveGame* EmptyRestored = Cast<UPBBumperLoadoutSaveGame>(
		UGameplayStatics::LoadGameFromMemory(SaveBytes));
	if (TestNotNull(TEXT("An intentionally empty loadout restores"), EmptyRestored))
	{
		TestTrue(
			TEXT("Empty loadout is preserved instead of being replaced by defaults"),
			EmptyRestored->EquippedSlots.IsEmpty());
	}

	return true;
}

#endif
