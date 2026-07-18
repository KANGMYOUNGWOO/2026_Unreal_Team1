#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperCounterShieldComponent.h"
#include "PinBallLike/Actor/Bumper/PBBumperSpawner.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperEquipController.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperCounterShieldSourceTransformTest,
	"PinBallLike.Bumper.Runtime.CounterShieldSourceTransform",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperCounterShieldSourceTransformTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UPBBumperCounterShieldComponent* Component = NewObject<UPBBumperCounterShieldComponent>();
	if (!TestNotNull(TEXT("Counter shield component can be created"), Component))
	{
		return false;
	}

	Component->ArmedSourceTransform = FTransform(
		FRotator(0.0, 90.0, 0.0),
		FVector(120.0, -35.0, 15.0));
	Component->ArmedSpawnOffset = FVector(40.0, 10.0, 5.0);
	const FVector ExpectedLocation = Component->ArmedSourceTransform.TransformPosition(
		Component->ArmedSpawnOffset);

	TestTrue(
		TEXT("Counter projectile location is resolved from the transform captured at activation"),
		Component->ResolveProjectileSpawnLocation().Equals(ExpectedLocation, KINDA_SMALL_NUMBER));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperAssetPreparationStateTest,
	"PinBallLike.Bumper.Runtime.AssetPreparationProtectsSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperAssetPreparationStateTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FPBBumperAssetPreparationState State;
	TestTrue(TEXT("An idle preparation can begin loading"), State.TryBeginLoading());
	TestTrue(TEXT("The preparation reports an active snapshot while loading"), State.HasPendingSnapshot());
	TestFalse(TEXT("A second request cannot replace a loading snapshot"), State.TryBeginLoading());

	State.MarkAssetsLoaded();
	TestTrue(TEXT("A loaded snapshot remains reserved until spawning"), State.IsReadyToSpawn());
	TestFalse(TEXT("A second request cannot replace a ready-to-spawn snapshot"), State.TryBeginLoading());

	State.Reset();
	TestFalse(TEXT("Reset releases the consumed snapshot"), State.HasPendingSnapshot());
	TestTrue(TEXT("A new request can start after reset"), State.TryBeginLoading());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperEquipEmptyAssetIdsTest,
	"PinBallLike.Bumper.UI.EmptyAssetIdsCompleteCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperEquipEmptyAssetIdsTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPBBumperEquipController* Controller = NewObject<UPBBumperEquipController>(GameInstance);
	if (!TestNotNull(TEXT("Bumper equip controller can be created"), Controller))
	{
		return false;
	}

	Controller->bInitialized = true;
	Controller->GameDataLoadSubsystem = NewObject<UPBGameDataLoadSubsystem>(GameInstance);
	Controller->BumperRowNames.Add(NAME_None);
	Controller->BumperRows.AddDefaulted();
	int32 CatalogReadyCount = 0;
	Controller->OnCatalogReady.AddLambda([&CatalogReadyCount]()
	{
		++CatalogReadyCount;
	});

	Controller->RequestBumperUIAssetsAsync();
	TestTrue(TEXT("A catalog with no valid asset ids still reaches a ready state"),
		Controller->bBumperListItemObjectsBuilt);
	TestEqual(TEXT("Catalog readiness is broadcast exactly once"), CatalogReadyCount, 1);
	return true;
}

#endif
