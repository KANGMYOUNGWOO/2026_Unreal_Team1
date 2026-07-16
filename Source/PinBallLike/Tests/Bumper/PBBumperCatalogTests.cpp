#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Utils/PBTextFormatUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperCatalogTest,
	"PinBallLike.Bumper.Catalog.FivePerTypeAndPrimaryAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperCatalogTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	const FName ExpectedRows[] =
	{
		PBBumperAssetIds::Bumper::Rebound_PowerPush,
		PBBumperAssetIds::Bumper::Rebound_CounterShell,
		PBBumperAssetIds::Bumper::Rebound_ComboPulse,
		PBBumperAssetIds::Bumper::Rebound_ManaOrb,
		PBBumperAssetIds::Bumper::Rebound_GroggyHammer,
		PBBumperAssetIds::Bumper::Side_ShieldCharge,
		PBBumperAssetIds::Bumper::Side_ManaCharge,
		PBBumperAssetIds::Bumper::Side_RepairPickup,
		PBBumperAssetIds::Bumper::Side_LaunchCharge,
		PBBumperAssetIds::Bumper::Side_StrengthCharge,
		PBBumperAssetIds::Bumper::Top_ComboUp,
		PBBumperAssetIds::Bumper::Top_GroggyStrike,
		PBBumperAssetIds::Bumper::Top_DirectStrike,
		PBBumperAssetIds::Bumper::Top_ComboCashout,
		PBBumperAssetIds::Bumper::Top_ComboPickup,
		PBBumperAssetIds::Bumper::Gate_SpeedUp,
		PBBumperAssetIds::Bumper::Gate_RecoveryField,
		PBBumperAssetIds::Bumper::Gate_ManaField,
		PBBumperAssetIds::Bumper::Gate_StrengthField,
		PBBumperAssetIds::Bumper::Gate_LaunchCharge
	};

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BumperTable = Settings ? Settings->BumperTable.LoadSynchronous() : nullptr;
	if (!TestNotNull(TEXT("Bumper DataTable is configured"), BumperTable))
	{
		return false;
	}

	TestEqual(TEXT("Bumper catalog contains exactly 20 rows"), BumperTable->GetRowMap().Num(), 20);

	TMap<EPBBumperType, int32> TypeCounts;
	for (const TPair<FName, uint8*>& RowPair : BumperTable->GetRowMap())
	{
		const FPBBumperTableRow* Row = reinterpret_cast<const FPBBumperTableRow*>(RowPair.Value);
		if (!TestNotNull(*FString::Printf(TEXT("Row data exists: %s"), *RowPair.Key.ToString()), Row))
		{
			continue;
		}

		TypeCounts.FindOrAdd(Row->BumperType) += 1;
		TestFalse(*FString::Printf(TEXT("TriggerID is configured: %s"), *RowPair.Key.ToString()),
			Row->TriggerID.IsNone());
		TestFalse(*FString::Printf(TEXT("EffectID is configured: %s"), *RowPair.Key.ToString()),
			Row->EffectID.IsNone());
		TestNotNull(*FString::Printf(TEXT("DataAsset resolves: %s"), *RowPair.Key.ToString()),
			Row->BumperDataAsset.LoadSynchronous());

		const FString FormattedDescription = PBTextFormatUtils::FormatSingleValueTemplate(
			Row->Description,
			FText::AsNumber(Row->RequiredTriggerCount)).ToString();
		TestFalse(
			*FString::Printf(TEXT("Bumper description resolves {0}: %s"), *RowPair.Key.ToString()),
			FormattedDescription.Contains(TEXT("{0}")));
	}

	TestEqual(TEXT("Rebound catalog count"), TypeCounts.FindRef(EPBBumperType::Rebound), 5);
	TestEqual(TEXT("Side catalog count"), TypeCounts.FindRef(EPBBumperType::Side), 5);
	TestEqual(TEXT("Top catalog count"), TypeCounts.FindRef(EPBBumperType::TopTarget), 5);
	TestEqual(TEXT("Gate catalog count"), TypeCounts.FindRef(EPBBumperType::Gate), 5);

	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> RegisteredBumperIds;
	AssetManager.GetPrimaryAssetIdList(PBBumperAssetIds::Type::BumperData, RegisteredBumperIds);
	TestEqual(TEXT("AssetManager registers exactly 20 BumperData assets"),
		RegisteredBumperIds.Num(),
		20);

	for (const FName ExpectedRow : ExpectedRows)
	{
		const FString Context = ExpectedRow.ToString();
		TestNotNull(*FString::Printf(TEXT("Bumper row exists: %s"), *Context),
			BumperTable->FindRow<FPBBumperTableRow>(ExpectedRow, Context, false));

		const FPrimaryAssetId ExpectedAssetId(PBBumperAssetIds::Type::BumperData, ExpectedRow);
		TestTrue(*FString::Printf(TEXT("PrimaryAssetId is registered: %s"), *Context),
			RegisteredBumperIds.Contains(ExpectedAssetId));

		FAssetData AssetData;
		if (TestTrue(*FString::Printf(TEXT("PrimaryAssetId resolves to AssetData: %s"), *Context),
			AssetManager.GetPrimaryAssetData(ExpectedAssetId, AssetData)))
		{
			const FString ExpectedAssetName = FString::Printf(TEXT("DA_Bumper_%s"), *Context);
			TestEqual(*FString::Printf(TEXT("Primary asset path uses the expected name: %s"), *Context),
				AssetData.AssetName.ToString(),
				ExpectedAssetName);
		}
	}

	return true;
}

#endif
