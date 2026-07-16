#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "PinBallLike/Struct/Bumper/PBBumperEquipSlot.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"

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
	};

	const FExpectedSlotMapping ExpectedMappings[] =
	{
		{ EPBBumperEquipSlot::TopLeft, EPBBumperSlotType::Top, EPBBumperPositionId::TopTargetLeft },
		{ EPBBumperEquipSlot::TopRight, EPBBumperSlotType::Top, EPBBumperPositionId::TopTargetRight },
		{ EPBBumperEquipSlot::SideLeft, EPBBumperSlotType::Side, EPBBumperPositionId::SideLeft },
		{ EPBBumperEquipSlot::SideRight, EPBBumperSlotType::Side, EPBBumperPositionId::SideRight },
		{ EPBBumperEquipSlot::ReboundLeft, EPBBumperSlotType::Rebound, EPBBumperPositionId::ReboundLeft },
		{ EPBBumperEquipSlot::ReboundRight, EPBBumperSlotType::Rebound, EPBBumperPositionId::ReboundRight },
		{ EPBBumperEquipSlot::Special, EPBBumperSlotType::Special, EPBBumperPositionId::GateCenterMid }
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
	TestTrue(TEXT("Legacy equip populated SideLeft"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::SideLeft, ActualRow));
	TestEqual(TEXT("SideLeft has the shared row"), ActualRow, SharedSideRow);
	TestTrue(TEXT("Legacy equip populated SideRight"),
		PlayerData->GetEquippedBumperAtSlot(EPBBumperEquipSlot::SideRight, ActualRow));
	TestEqual(TEXT("SideRight has the shared row"), ActualRow, SharedSideRow);

	const TArray<FPBEquippedBumperSlot> EquippedSlots = PlayerData->GetEquippedBumperSlots();
	TestEqual(TEXT("Three physical slots are currently equipped"), EquippedSlots.Num(), 3);
	const TArray<FName> UniqueRows = PlayerData->GetEquippedBumperRowIds();
	TestEqual(TEXT("Repeated rows are unique in the asset load list"), UniqueRows.Num(), 2);

	const EPBBumperSlotType InvalidSlotType = static_cast<EPBBumperSlotType>(255);
	TestFalse(TEXT("Invalid legacy category is rejected"),
		PlayerData->GetEquippedBumper(InvalidSlotType, ActualRow));
	TestTrue(TEXT("Invalid query clears the output row"), ActualRow.IsNone());

	return true;
}

#endif
