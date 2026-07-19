#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "PinBallLike/Collection/UI/PBCollectionCatalogItemObject.h"
#include "PinBallLike/Collection/UI/PBCollectionTabController.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"

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
	UDataTable* SynergyEffect = MakeTable();
	UDataTable* SynergyModifier = MakeTable();
	UDataTable* SynergyTrigger = MakeTable();

	TableData->SetBumperTables(Bumper, Trigger, BumperEffect);
	TableData->SetBallTables(Ball, BallStar);
	TableData->SetSkillTable(Skill);
	TableData->SetBossTables(Boss, BossHitPoint, BossPattern);
	TableData->SetRelicTable(Relic, RelicModifier);
	TableData->SetSynergyTables(
		Synergy,
		SynergyTier,
		nullptr,
		SynergyEffect,
		SynergyModifier,
		SynergyTrigger);

	TestTrue(TEXT("Collection catalog readiness uses every source table required by its five tabs"),
		TableData->IsCollectionCatalogDataReady());
	TestTrue(TEXT("DT_Collection is optional display metadata"),
		!TableData->IsCollectionTableReady() && TableData->IsCollectionCatalogDataReady());

	TableData->SetRelicTable(Relic, nullptr);
	TestFalse(TEXT("A missing source table makes the collection catalog unavailable"),
		TableData->IsCollectionCatalogDataReady());

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

	FPBCollectionItemSummary Beta;
	Beta.SourceRowName = TEXT("Ball_Beta");
	Beta.DisplayName = FText::FromString(TEXT("베타 볼"));
	Beta.Description = FText::FromString(TEXT("지원형 볼"));

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
	Items.Reset();
	SelectedItem = nullptr;
	const int32 RestoredDataIndex = Controller->BuildCatalogItems(
		EPBCollectionCategory::Ball,
		{Alpha, Beta},
		{4, 7},
		Items,
		SelectedItem);
	TestEqual(TEXT("Selection is restored by source RowName"), RestoredDataIndex, 7);
	TestTrue(TEXT("The restored item points to the selected source row"),
		SelectedItem && SelectedItem->Summary.SourceRowName == Beta.SourceRowName);

	return true;
}

#endif
