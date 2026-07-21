#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "PinBallLike/Collection/PBCollectionEffectSetProjector.h"
#include "PinBallLike/Collection/UI/PBCollectionCatalogItemObject.h"
#include "PinBallLike/Collection/UI/PBCollectionTabController.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectSetRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBCollectionCatalogReadinessTest,
	"PinBallLike.Collection.Catalog.SpecificReadiness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBCollectionCatalogReadinessTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPBTableDataSubsystem* TableData = NewObject<UPBTableDataSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Table data subsystem can be created"), TableData))
	{
		return false;
	}

	auto MakeTable = []()
	{
		return NewObject<UDataTable>();
	};

	UDataTable* Bumper = MakeTable();
	UDataTable* Trigger = MakeTable();
	UDataTable* BumperEffect = MakeTable();
	UDataTable* Ball = MakeTable();
	UDataTable* BallStar = MakeTable();
	UDataTable* Skill = MakeTable();
	UDataTable* Boss = MakeTable();
	UDataTable* BossHitPoint = MakeTable();
	UDataTable* BossPattern = MakeTable();
	UDataTable* Relic = MakeTable();
	UDataTable* RelicModifier = MakeTable();
	UDataTable* Synergy = MakeTable();
	UDataTable* SynergyTier = MakeTable();
	UDataTable* Effect = MakeTable();
	UDataTable* EffectSet = MakeTable();
	UDataTable* EffectParam = MakeTable();

	TableData->SetBumperTables(Bumper, Trigger, BumperEffect);
	TableData->SetBallTables(Ball, BallStar);
	TableData->SetSkillTable(Skill);
	TableData->SetBossTables(Boss, BossHitPoint, BossPattern);
	TableData->SetRelicTable(Relic, RelicModifier);
	TableData->SetSynergyTables(
		Synergy,
		SynergyTier);
	TableData->SetEffectTables(Effect, EffectSet, EffectParam);

	TestTrue(TEXT("Collection catalog readiness uses every source table required by its five tabs"),
		TableData->IsCollectionCatalogDataReady());
	TestTrue(TEXT("Every category reports ready when its own tables are assigned"),
		TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Ball)
		&& TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Synergy)
		&& TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Relic)
		&& TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Bumper)
		&& TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Boss));
	TestTrue(TEXT("DT_Collection is optional display metadata"),
		!TableData->IsCollectionTableReady() && TableData->IsCollectionCatalogDataReady());

	TableData->SetRelicTable(Relic, nullptr);
	TestFalse(TEXT("A missing source table makes the collection catalog unavailable"),
		TableData->IsCollectionCatalogDataReady());
	TestFalse(TEXT("A missing Relic table blocks only the Relic category"),
		TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Relic));
	TestTrue(TEXT("A missing Relic table does not block the Ball category"),
		TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Ball));
	TestTrue(TEXT("At least one usable category keeps the collection available"),
		TableData->IsAnyCollectionCatalogDataReady());
	TableData->SetRelicTable(Relic, RelicModifier);
	TableData->SetEffectTables(Effect, nullptr, EffectParam);
	TestFalse(TEXT("A missing EffectSet table makes the synergy catalog unavailable"),
		TableData->IsCollectionCatalogDataReady());
	TestFalse(TEXT("A missing EffectSet table blocks the Synergy category"),
		TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Synergy));
	TestTrue(TEXT("A missing EffectSet table does not block the Bumper category"),
		TableData->IsCollectionCatalogDataReady(EPBCollectionCategory::Bumper));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBCollectionEffectSetProjectionTest,
	"PinBallLike.Collection.Catalog.EffectSetProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBCollectionEffectSetProjectionTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPBTableDataSubsystem* TableData = NewObject<UPBTableDataSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Table data subsystem can be created"), TableData))
	{
		return false;
	}

	UDataTable* EffectTable = NewObject<UDataTable>();
	EffectTable->RowStruct = FPBEffectTableRow::StaticStruct();
	FPBEffectTableRow EffectRow;
	EffectRow.EffectType = PBEffectTypes::EffectType::ResourceBuff;
	EffectRow.TargetType = PBEffectTypes::TargetType::PartyBalls;
	EffectRow.Description = TEXT("두 번째 효과입니다.");
	EffectTable->AddRow(TEXT("Effect_TestMana"), EffectRow);
	FPBEffectTableRow FirstEffectRow = EffectRow;
	FirstEffectRow.Description = TEXT("첫 번째 효과입니다.");
	EffectTable->AddRow(TEXT("Effect_TestFirst"), FirstEffectRow);

	UDataTable* EffectSetTable = NewObject<UDataTable>();
	EffectSetTable->RowStruct = FPBEffectSetRow::StaticStruct();
	FPBEffectSetRow EffectSetRow;
	EffectSetRow.EffectSetId = TEXT("EffectSet_Test");
	EffectSetRow.EffectId = TEXT("Effect_TestMana");
	EffectSetRow.Order = 2;
	EffectSetTable->AddRow(TEXT("EffectSet_Test_02"), EffectSetRow);
	FPBEffectSetRow FirstEffectSetRow = EffectSetRow;
	FirstEffectSetRow.EffectId = TEXT("Effect_TestFirst");
	FirstEffectSetRow.Order = 1;
	EffectSetTable->AddRow(TEXT("EffectSet_Test_01"), FirstEffectSetRow);

	UDataTable* EffectParamTable = NewObject<UDataTable>();
	EffectParamTable->RowStruct = FPBEffectParamRow::StaticStruct();
	FPBEffectParamRow EffectParamRow;
	EffectParamRow.EffectId = TEXT("Effect_TestMana");
	EffectParamRow.ParamKey = PBEffectTypes::ParamKey::Value;
	EffectParamRow.ParamValue = TEXT("10");
	EffectParamTable->AddRow(TEXT("Effect_TestMana_Value"), EffectParamRow);

	TableData->SetEffectTables(EffectTable, EffectSetTable, EffectParamTable);
	const int32 EffectRowCountBeforeProjection = EffectTable->GetRowMap().Num();
	const FPBCollectionEffectSetProjection Projection =
		FPBCollectionEffectSetProjector::Build(*TableData, TEXT("EffectSet_Test"));

	TestTrue(TEXT("A complete EffectSet projection is valid"), Projection.bIsValid);
	TestEqual(TEXT("The declared effect count is preserved"), Projection.DeclaredEffectCount, 2);
	TestEqual(TEXT("Every referenced effect resolves"), Projection.ResolvedEffectCount, 2);
	TestFalse(TEXT("The user-facing effect summary is populated"), Projection.EffectSummary.IsEmpty());
	const FString OrderedSummary = Projection.EffectSummary.ToString();
	TestTrue(TEXT("EffectSet projection preserves the declared Order"),
		OrderedSummary.Find(TEXT("첫 번째")) < OrderedSummary.Find(TEXT("두 번째")));
	TestFalse(TEXT("The parameter summary is populated"), Projection.ParameterSummary.IsEmpty());
	TestFalse(TEXT("The target summary is populated"), Projection.TargetSummary.IsEmpty());
	TestEqual(TEXT("Projection does not mutate the Effect table"),
		EffectTable->GetRowMap().Num(), EffectRowCountBeforeProjection);

	EffectSetRow.EffectId = TEXT("Effect_Missing");
	EffectSetTable->RemoveRow(TEXT("EffectSet_Test_02"));
	EffectSetTable->AddRow(TEXT("EffectSet_Test_02"), EffectSetRow);
	AddExpectedError(TEXT("requested row 'Effect_Missing'"), EAutomationExpectedErrorFlags::Contains, 1);
	const FPBCollectionEffectSetProjection InvalidProjection =
		FPBCollectionEffectSetProjector::Build(*TableData, TEXT("EffectSet_Test"));
	TestFalse(TEXT("An EffectSet with a missing Effect reference is invalid"), InvalidProjection.bIsValid);
	TestFalse(TEXT("An invalid projection explains the broken reference"), InvalidProjection.ValidationText.IsEmpty());

	EffectSetRow.EffectId = TEXT("Effect_TestMana");
	EffectSetTable->RemoveRow(TEXT("EffectSet_Test_02"));
	EffectSetTable->AddRow(TEXT("EffectSet_Test_02"), EffectSetRow);
	EffectRow.EffectType = TEXT("UnsupportedEffectType");
	EffectTable->RemoveRow(TEXT("Effect_TestMana"));
	EffectTable->AddRow(TEXT("Effect_TestMana"), EffectRow);
	const FPBCollectionEffectSetProjection UnsupportedHandlerProjection =
		FPBCollectionEffectSetProjector::Build(*TableData, TEXT("EffectSet_Test"));
	TestFalse(TEXT("An EffectType without a registered Handler is rejected"),
		UnsupportedHandlerProjection.bIsValid);

	EffectRow.EffectType = PBEffectTypes::EffectType::ResourceBuff;
	EffectRow.TargetType = PBEffectTypes::TargetType::Player;
	EffectTable->RemoveRow(TEXT("Effect_TestMana"));
	EffectTable->AddRow(TEXT("Effect_TestMana"), EffectRow);
	const FPBCollectionEffectSetProjection UnsupportedTargetProjection =
		FPBCollectionEffectSetProjector::Build(*TableData, TEXT("EffectSet_Test"));
	TestFalse(TEXT("A TargetType without a registered TargetResolver is rejected"),
		UnsupportedTargetProjection.bIsValid);

	EffectRow.TargetType = PBEffectTypes::TargetType::PartyBalls;
	EffectTable->RemoveRow(TEXT("Effect_TestMana"));
	EffectTable->AddRow(TEXT("Effect_TestMana"), EffectRow);
	EffectParamRow.ParamKey = TEXT("UnsupportedParamKey");
	EffectParamTable->RemoveRow(TEXT("Effect_TestMana_Value"));
	EffectParamTable->AddRow(TEXT("Effect_TestMana_Value"), EffectParamRow);
	const FPBCollectionEffectSetProjection UnsupportedParamProjection =
		FPBCollectionEffectSetProjector::Build(*TableData, TEXT("EffectSet_Test"));
	TestFalse(TEXT("An unknown Effect parameter key is rejected"), UnsupportedParamProjection.bIsValid);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBCollectionEffectSetAssetIntegrityTest,
	"PinBallLike.Collection.Catalog.EffectSetAssetIntegrity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBCollectionEffectSetAssetIntegrityTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UDataTable* EffectTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Data/Tables/Effect/DT_Effect.DT_Effect"));
	UDataTable* EffectSetTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Data/Tables/Effect/DT_EffectSet.DT_EffectSet"));
	UDataTable* EffectParamTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Data/Tables/Effect/DT_EffectParam.DT_EffectParam"));
	UDataTable* SynergyTierTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Data/Tables/Synergy/DT_SynergyTier.DT_SynergyTier"));

	if (!TestNotNull(TEXT("DT_Effect loads"), EffectTable)
		|| !TestNotNull(TEXT("DT_EffectSet loads"), EffectSetTable)
		|| !TestNotNull(TEXT("DT_EffectParam loads"), EffectParamTable)
		|| !TestNotNull(TEXT("DT_SynergyTier loads"), SynergyTierTable))
	{
		return false;
	}

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPBTableDataSubsystem* TableData = NewObject<UPBTableDataSubsystem>(GameInstance);
	TableData->SetEffectTables(EffectTable, EffectSetTable, EffectParamTable);

	TArray<FPBSynergyTierRow*> TierRows;
	SynergyTierTable->GetAllRows<FPBSynergyTierRow>(TEXT("CollectionEffectSetAssetIntegrity"), TierRows);
	TestTrue(TEXT("The project contains at least one synergy tier"), !TierRows.IsEmpty());

	for (const FPBSynergyTierRow* TierRow : TierRows)
	{
		if (!TierRow)
		{
			continue;
		}

		const FPBCollectionEffectSetProjection Projection =
			FPBCollectionEffectSetProjector::Build(*TableData, TierRow->EffectSetId);
		TestTrue(
			*FString::Printf(
				TEXT("Synergy %s tier %d resolves EffectSet %s: %s"),
				*TierRow->SynergyId.ToString(),
				TierRow->RequiredCount,
				*TierRow->EffectSetId.ToString(),
				*Projection.ValidationText.ToString()),
			Projection.bIsValid);
		if (Projection.bIsValid)
		{
			TestFalse(TEXT("A valid EffectSet produces a user-facing summary"), Projection.EffectSummary.IsEmpty());
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBCollectionBallSkillDescriptionContractTest,
	"PinBallLike.Collection.Catalog.BallSkillDescriptionContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBCollectionBallSkillDescriptionContractTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);
	constexpr int32 PreviewAttack = 10;

	FPBBallSkillTableRow TestRow;
	TestRow.Description = FText::FromString(TEXT("{Power}|{Life}|{Effect}|{Groggy}"));
	TestRow.PowerValue = 0.8f;
	TestRow.LifeValue = 2.0f;
	TestRow.EffectValue = 3;
	TestRow.GroggyValue = 4;

	TestEqual(
		TEXT("Skill damage uses the shared ball attack multiplier"),
		TestRow.CalculateBaseDamage(PreviewAttack),
		8);
	TestEqual(TEXT("Negative attack values cannot create negative damage"), TestRow.CalculateBaseDamage(-10), 0);
	TestEqual(
		TEXT("Every supported named placeholder is resolved"),
		TestRow.GetDescription(PreviewAttack).ToString(),
		FString(TEXT("8|2|3|4")));

	UDataTable* SkillTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Data/Tables/DT_Skill.DT_Skill"));
	if (!TestNotNull(TEXT("DT_Skill loads"), SkillTable))
	{
		return false;
	}

	TestTrue(
		TEXT("DT_Skill uses FPBBallSkillTableRow"),
		SkillTable->GetRowStruct() == FPBBallSkillTableRow::StaticStruct());
	const TArray<FName> RowNames = SkillTable->GetRowNames();
	TestTrue(TEXT("DT_Skill contains at least one skill"), !RowNames.IsEmpty());

	for (const FName RowName : RowNames)
	{
		const FPBBallSkillTableRow* SkillRow = SkillTable->FindRow<FPBBallSkillTableRow>(
			RowName,
			TEXT("PBCollectionBallSkillDescriptionContract"),
			false);
		if (!TestNotNull(*FString::Printf(TEXT("Skill row %s is readable"), *RowName.ToString()), SkillRow))
		{
			continue;
		}

		const FString FormattedDescription = SkillRow->GetDescription(PreviewAttack).ToString();
		TestFalse(
			*FString::Printf(TEXT("Skill row %s has no unresolved placeholders"), *RowName.ToString()),
			FormattedDescription.Contains(TEXT("{")) || FormattedDescription.Contains(TEXT("}")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBCollectionTabControllerTest,
	"PinBallLike.Collection.UI.ControllerSearchAndSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBCollectionTabControllerTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UPBCollectionTabController* Controller = NewObject<UPBCollectionTabController>();
	if (!TestNotNull(TEXT("Collection tab controller can be created"), Controller))
	{
		return false;
	}

	FPBCollectionItemSummary Alpha;
	Alpha.SourceRowName = TEXT("Ball_Alpha");
	Alpha.DisplayName = FText::FromString(TEXT("알파 볼"));
	Alpha.Description = FText::FromString(TEXT("기본 공격형 볼"));
	Alpha.SortOrder = 10;

	FPBCollectionItemSummary Beta;
	Beta.SourceRowName = TEXT("Ball_Beta");
	Beta.DisplayName = FText::FromString(TEXT("베타 볼"));
	Beta.Description = FText::FromString(TEXT("지원형 볼"));
	Beta.SortOrder = 20;

	Controller->SetSearchText(FText::FromString(TEXT("알파")));
	TestTrue(TEXT("Search matches a display name"), Controller->MatchesSearch(Alpha));
	TestFalse(TEXT("Search excludes unrelated entries"), Controller->MatchesSearch(Beta));
	Controller->SetSearchText(FText::GetEmpty());

	TArray<UPBCollectionCatalogItemObject*> Items;
	UPBCollectionCatalogItemObject* SelectedItem = nullptr;
	const int32 InitialDataIndex = Controller->BuildCatalogItems(
		EPBCollectionCategory::Ball,
		{Alpha, Beta},
		{4, 7},
		Items,
		SelectedItem);
	TestEqual(TEXT("The first item is selected initially"), InitialDataIndex, 4);
	TestEqual(TEXT("Two list item objects are created"), Items.Num(), 2);

	Controller->ResolveCatalogItem(Items[1]);
	Controller->SetSortMode(EPBCollectionSortMode::NameDesc);
	Items.Reset();
	SelectedItem = nullptr;
	const int32 DescendingDataIndex = Controller->BuildCatalogItems(
		EPBCollectionCategory::Ball,
		{Alpha, Beta},
		{4, 7},
		Items,
		SelectedItem);
	TestEqual(TEXT("Name descending keeps the selected data row"), DescendingDataIndex, 7);
	TestTrue(TEXT("Name descending places Alpha before Beta"),
		Items.Num() == 2 && Items[0]->Summary.SourceRowName == Alpha.SourceRowName);
	TestTrue(TEXT("Selection is restored by source RowName after descending sort"),
		SelectedItem && SelectedItem->Summary.SourceRowName == Beta.SourceRowName);

	Controller->SetSortMode(EPBCollectionSortMode::NameAsc);
	Items.Reset();
	SelectedItem = nullptr;
	const int32 AscendingDataIndex = Controller->BuildCatalogItems(
		EPBCollectionCategory::Ball,
		{Alpha, Beta},
		{4, 7},
		Items,
		SelectedItem);
	TestEqual(TEXT("Name ascending keeps the selected data row"), AscendingDataIndex, 7);
	TestTrue(TEXT("Name ascending places Beta before Alpha"),
		Items.Num() == 2 && Items[0]->Summary.SourceRowName == Beta.SourceRowName);
	TestTrue(TEXT("Selection points to Beta after its list position changes"),
		SelectedItem == Items[0]);

	return true;
}

#endif
