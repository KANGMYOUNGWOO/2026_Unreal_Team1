#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "Tests/AutomationCommon.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/Component/PBBallPhysicsComponent.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Bumper/Component/PBBumperCounterShieldComponent.h"
#include "PinBallLike/Actor/Bumper/Component/PBTurretFireComponent.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Effect/PBGateAccelerationBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBPercentShieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PinBallLike/Actor/Bumper/PBBumperSpawner.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateAccelerationField.h"
#include "PinBallLike/Actor/Bumper/Summon/PBTurretSummonActor.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBCollisionBumperTriggerActor.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBGateBumperTriggerActor.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperDragDropOperation.h"
#include "PinBallLike/Actor/Bumper/UI/Equip/PBBumperEquipController.h"
#include "PinBallLike/Actor/Common/Component/Stat/PBBaseStatComponent.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Interface/Movable.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Subsystem/PBGameDataLoadSubsystem.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Utils/PBInterfaceUtils.h"

namespace
{
	APBModularBumperBase* SpawnRuntimeTestBumper(
		UWorld* World,
		UClass* BumperClass,
		TSubclassOf<APBBumperTriggerActorBase> TriggerClass,
		const TArray<EPBBumperPositionId>& PositionIds,
		const int32 RequiredTriggerCount,
		TSubclassOf<UPBBumperEffectBase> EffectClass = nullptr,
		const FPBBumperEffectRow EffectRow = FPBBumperEffectRow())
	{
		if (!IsValid(World) || !IsValid(BumperClass) || !TriggerClass || PositionIds.IsEmpty())
		{
			return nullptr;
		}

		FPBBumperTableRow BumperRow;
		BumperRow.RequiredTriggerCount = RequiredTriggerCount;

		FPBBumperTriggerSpawnInfo SpawnInfo;
		SpawnInfo.TriggerClass = TriggerClass;
		SpawnInfo.PositionIds = PositionIds;

		TMap<EPBBumperPositionId, FTransform> AnchorTransforms;
		for (int32 Index = 0; Index < PositionIds.Num(); ++Index)
		{
			AnchorTransforms.Add(
				PositionIds[Index],
				FTransform(FVector(0.0, Index * 200.0 - 100.0, 0.0)));
		}

		APBModularBumperBase* Bumper = World->SpawnActorDeferred<APBModularBumperBase>(
			BumperClass,
			FTransform::Identity,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!IsValid(Bumper))
		{
			return nullptr;
		}

		Bumper->InitializeBumper(
			TEXT("Bumper_Runtime_Acceptance"),
			BumperRow,
			{SpawnInfo},
			EffectRow,
			EffectClass,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			AnchorTransforms);
		UGameplayStatics::FinishSpawningActor(Bumper, FTransform::Identity);
		return Bumper;
	}

	AActor* SpawnMovableRuntimeTestActor(UWorld* World, const bool bIncludeStatProvider = true)
	{
		if (!IsValid(World))
		{
			return nullptr;
		}

		AActor* Actor = World->SpawnActor<AActor>(
			AActor::StaticClass(),
			FTransform::Identity);
		if (!IsValid(Actor))
		{
			return nullptr;
		}

		USphereComponent* CollisionComponent = NewObject<USphereComponent>(Actor);
		UPBBaseStatComponent* StatComponent = bIncludeStatProvider
			? NewObject<UPBBaseStatComponent>(Actor)
			: nullptr;
		UPBBallPhysicsComponent* PhysicsComponent = NewObject<UPBBallPhysicsComponent>(Actor);
		Actor->AddInstanceComponent(CollisionComponent);
		if (IsValid(StatComponent))
		{
			Actor->AddInstanceComponent(StatComponent);
		}
		Actor->AddInstanceComponent(PhysicsComponent);
		Actor->SetRootComponent(CollisionComponent);
		CollisionComponent->RegisterComponent();
		if (IsValid(StatComponent))
		{
			StatComponent->RegisterComponent();
		}
		PhysicsComponent->RegisterComponent();
		PhysicsComponent->InitializeDependencies(CollisionComponent, StatComponent);
		PhysicsComponent->SetVelocity(FVector(2000.0, 0.0, 0.0));
		return Actor;
	}

	TArray<APBBumperTriggerActorBase*> FindOwnedRuntimeTestTriggers(
		UWorld* World,
		const APBModularBumperBase* OwnerBumper)
	{
		TArray<APBBumperTriggerActorBase*> Result;
		for (TActorIterator<APBBumperTriggerActorBase> It(World); It; ++It)
		{
			APBBumperTriggerActorBase* Trigger = *It;
			if (IsValid(Trigger) && Trigger->GetOwnerBumper() == OwnerBumper)
			{
				Result.Add(Trigger);
			}
		}

		Result.Sort([](const APBBumperTriggerActorBase& Left, const APBBumperTriggerActorBase& Right)
		{
			return static_cast<uint8>(Left.GetPositionId())
				< static_cast<uint8>(Right.GetPositionId());
		});
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperBallOwnedActorEffectTargetTest,
	"PinBallLike.Bumper.Runtime.BallOwnedActorEffectTarget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperBallOwnedActorEffectTargetTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary Ball-owned interaction world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	UClass* BumperClass = LoadClass<APBModularBumperBase>(
		nullptr,
		TEXT("/Game/Blueprints/Bumper/BP_ModularBumper.BP_ModularBumper_C"));
	APBBallBase* OwnerBall = IsValid(World)
		? World->SpawnActor<APBBallBase>(APBBallBase::StaticClass(), FTransform::Identity)
		: nullptr;
	AActor* BallOwnedMovable = SpawnMovableRuntimeTestActor(World, false);
	AActor* GenericMovable = SpawnMovableRuntimeTestActor(World, false);
	if (!TestNotNull(TEXT("The production modular Bumper Blueprint resolves"), BumperClass)
		|| !TestNotNull(TEXT("An owner Ball can be spawned"), OwnerBall)
		|| !TestNotNull(TEXT("A Ball-owned IMovable can be spawned"), BallOwnedMovable)
		|| !TestNotNull(TEXT("An unrelated IMovable can be spawned"), GenericMovable))
	{
		return false;
	}

	BallOwnedMovable->SetOwner(OwnerBall);
	TestEqual(
		TEXT("A direct Ball remains its own Ball effect target"),
		UPBBumperEffectBase::ResolveBallEffectTargetOrSource(OwnerBall),
		static_cast<AActor*>(OwnerBall));
	TestEqual(
		TEXT("A Ball-owned IMovable resolves its owner Ball as the effect target"),
		UPBBumperEffectBase::ResolveBallEffectTargetOrSource(BallOwnedMovable),
		static_cast<AActor*>(OwnerBall));
	TestEqual(
		TEXT("An unrelated IMovable remains the effect target"),
		UPBBumperEffectBase::ResolveBallEffectTargetOrSource(GenericMovable),
		GenericMovable);

	UPBBaseResourceComponent* ResourceComponent = OwnerBall->GetResourceComponent();
	if (!TestNotNull(TEXT("The owner Ball exposes its resource component"), ResourceComponent))
	{
		return false;
	}
	ResourceComponent->SetResource(PBResourceNames::Health, 100.0f, 100.0f);

	FPBBumperEffectRow EffectRow;
	EffectRow.Power = 10.0f;
	APBModularBumperBase* Bumper = SpawnRuntimeTestBumper(
		World,
		BumperClass,
		APBCollisionBumperTriggerActor::StaticClass(),
		{EPBBumperPositionId::SideLeft},
		1,
		UPBPercentShieldBumperEffect::StaticClass(),
		EffectRow);
	if (!TestNotNull(TEXT("A shield Bumper can be spawned"), Bumper))
	{
		return false;
	}

	Bumper->DispatchBeginPlay();
	const TArray<APBBumperTriggerActorBase*> Triggers =
		FindOwnedRuntimeTestTriggers(World, Bumper);
	if (!TestEqual(TEXT("The shield Bumper creates one Trigger"), Triggers.Num(), 1))
	{
		return false;
	}

	Bumper->HandleTriggerActorActivated(Triggers[0], BallOwnedMovable, FHitResult());
	TestEqual(
		TEXT("The Ball-owned interaction applies shield to the owner Ball"),
		ResourceComponent->GetResourceCurrent(PBResourceNames::Shield),
		10.0f);
	TestEqual(TEXT("The interaction records one meaningful contact"),
		Bumper->GetMeaningfulContactCount(), 1);
	TestEqual(TEXT("The interaction is not counted as a direct Ball contact"),
		Bumper->GetDirectBallContactCount(), 0);
	TestEqual(TEXT("The interaction is counted as a Ball-owned Actor contact"),
		Bumper->GetBallOwnedActorContactCount(), 1);
	TestEqual(TEXT("The interaction is not counted as another Movable contact"),
		Bumper->GetOtherMovableContactCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBSummonActorDestructionRecoveryTest,
	"PinBallLike.Bumper.Runtime.SummonActorDestructionRecovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBSummonActorDestructionRecoveryTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary summon recovery world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UClass* BumperClass = LoadClass<APBModularBumperBase>(
		nullptr,
		TEXT("/Game/Blueprints/Bumper/BP_ModularBumper.BP_ModularBumper_C"));
	if (!TestNotNull(TEXT("The production modular bumper Blueprint class resolves"), BumperClass))
	{
		return false;
	}

	FPBBumperEffectRow EffectRow;
	EffectRow.Power = 25.0f;
	EffectRow.Duration = 30.0f;

	UWorld* World = TestWorld.GetTestWorld();
	APBModularBumperBase* Bumper = SpawnRuntimeTestBumper(
		World,
		BumperClass,
		APBGateBumperTriggerActor::StaticClass(),
		{EPBBumperPositionId::GateCenterMid},
		1,
		UPBGateAccelerationBumperEffect::StaticClass(),
		EffectRow);
	AActor* GenericMovable = SpawnMovableRuntimeTestActor(World, false);
	if (!TestNotNull(TEXT("The summon recovery bumper can be spawned"), Bumper)
		|| !TestNotNull(TEXT("A generic IMovable can be spawned"), GenericMovable))
	{
		return false;
	}

	Bumper->DispatchBeginPlay();
	const TArray<APBBumperTriggerActorBase*> Triggers =
		FindOwnedRuntimeTestTriggers(World, Bumper);
	if (!TestEqual(TEXT("The summon recovery bumper has one Trigger"), Triggers.Num(), 1))
	{
		return false;
	}

	Bumper->HandleTriggerActorActivated(Triggers[0], GenericMovable, FHitResult());
	TestEqual(
		TEXT("The long-running summon effect occupies the execution lane"),
		Bumper->GetBumperState(),
		EPBBumperState::Activated);

	APBGateAccelerationField* SpawnedField = nullptr;
	for (TActorIterator<APBGateAccelerationField> It(World); It; ++It)
	{
		if (IsValid(*It) && It->GetOwner() == Bumper)
		{
			SpawnedField = *It;
			break;
		}
	}
	if (!TestNotNull(TEXT("The active effect owns its spawned field"), SpawnedField))
	{
		return false;
	}

	SpawnedField->Destroy();
	TestWorld.TickTestWorld();
	TestEqual(
		TEXT("Destroying an active summon releases the execution lane"),
		Bumper->GetBumperState(),
		EPBBumperState::Idle);
	TestNull(
		TEXT("Destroying an active summon clears the active Trigger"),
		Bumper->GetActiveTriggerActor());

	Bumper->HandleTriggerActorActivated(Triggers[0], GenericMovable, FHitResult());
	TestEqual(
		TEXT("A replacement summon can activate after recovery"),
		Bumper->GetBumperState(),
		EPBBumperState::Activated);
	return true;
}

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
	FPBTurretAimRotationTest,
	"PinBallLike.Bumper.Runtime.TurretAimRotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBTurretAimRotationTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	const FVector SourceLocation(100.0f, 50.0f, 20.0f);
	const FVector TargetLocation(100.0f, 250.0f, 520.0f);
	const FRotator CurrentRotation(12.0f, -30.0f, 7.0f);
	FRotator ResolvedRotation;

	TestTrue(
		TEXT("A horizontal target direction resolves to an aim rotation"),
		UPBTurretFireComponent::TryResolveAimRotation(
			SourceLocation,
			TargetLocation,
			CurrentRotation,
			true,
			0.0f,
			ResolvedRotation));
	TestTrue(
		TEXT("The turret +X forward axis turns toward world +Y"),
		FMath::IsNearlyEqual(ResolvedRotation.Yaw, 90.0f));
	TestTrue(
		TEXT("Yaw-only aiming preserves the placed pitch"),
		FMath::IsNearlyEqual(ResolvedRotation.Pitch, CurrentRotation.Pitch));
	TestTrue(
		TEXT("Yaw-only aiming preserves the placed roll"),
		FMath::IsNearlyEqual(ResolvedRotation.Roll, CurrentRotation.Roll));

	TestFalse(
		TEXT("A target at the same horizontal location does not overwrite the current aim"),
		UPBTurretFireComponent::TryResolveAimRotation(
			SourceLocation,
			FVector(SourceLocation.X, SourceLocation.Y, SourceLocation.Z + 500.0f),
			CurrentRotation,
			true,
			0.0f,
			ResolvedRotation));

	TestTrue(
		TEXT("An authored mesh offset resolves from the actual muzzle direction"),
		UPBTurretFireComponent::TryResolveAimRotationFromDirections(
			FVector::ForwardVector,
			FVector::RightVector,
			FRotator(0.0f, -90.0f, 0.0f),
			true,
			0.0f,
			ResolvedRotation));
	TestTrue(
		TEXT("The authored minus ninety degree head offset receives only the required yaw delta"),
		FMath::IsNearlyEqual(ResolvedRotation.Yaw, 0.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBTurretProductionAimPivotTest,
	"PinBallLike.Bumper.Runtime.TurretProductionAimPivot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBTurretProductionAimPivotTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary game world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UClass* TurretClass = LoadClass<AActor>(
		nullptr,
		TEXT("/Game/Blueprints/Bumper/Effect/BP_TurretSummon.BP_TurretSummon_C"));
	if (!TestNotNull(TEXT("The production turret Blueprint class resolves"), TurretClass))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	AActor* Turret = IsValid(World)
		? World->SpawnActor<AActor>(TurretClass, FTransform(FVector(100.0f, 50.0f, 0.0f)))
		: nullptr;
	AActor* Target = IsValid(World)
		? World->SpawnActor<AActor>(
			AActor::StaticClass(),
			FTransform(FVector(100.0f, 1050.0f, 0.0f)))
		: nullptr;
	if (!TestNotNull(TEXT("The production turret can be spawned"), Turret)
		|| !TestNotNull(TEXT("A target Actor can be spawned"), Target))
	{
		return false;
	}

	Turret->DispatchBeginPlay();
	UPBTurretFireComponent* FireComponent = Turret->FindComponentByClass<UPBTurretFireComponent>();
	if (!TestNotNull(TEXT("The production turret owns its fire component"), FireComponent))
	{
		return false;
	}

	const TArray<UActorComponent*> AimPivotComponents = Turret->GetComponentsByTag(
		USceneComponent::StaticClass(),
		TEXT("TurretAimPivot"));
	if (!TestEqual(TEXT("The production turret has exactly one explicit aim pivot"),
		AimPivotComponents.Num(), 1))
	{
		return false;
	}

	USceneComponent* AimPivot = Cast<USceneComponent>(AimPivotComponents[0]);
	if (!TestNotNull(TEXT("The aim pivot is a scene component"), AimPivot))
	{
		return false;
	}
	const TArray<UActorComponent*> MuzzleComponents = Turret->GetComponentsByTag(
		USceneComponent::StaticClass(),
		TEXT("TurretMuzzle"));
	if (!TestEqual(TEXT("The production turret has exactly one muzzle point"),
		MuzzleComponents.Num(), 1))
	{
		return false;
	}
	USceneComponent* Muzzle = Cast<USceneComponent>(MuzzleComponents[0]);
	if (!TestNotNull(TEXT("The production muzzle is a scene component"), Muzzle))
	{
		return false;
	}

	const FRotator BaseRotationBeforeAim = Turret->GetActorRotation();
	FireComponent->ConfigureAttack(
		Target,
		EPBBumperProjectilePayload::BossDamage,
		1,
		1);

	AimPivot->SetWorldRotation(FRotator::ZeroRotator);
	FireComponent->TickComponent(1.0f / 60.0f, LEVELTICK_All, nullptr);

	const FVector ExpectedDirection = (Target->GetActorLocation() - AimPivot->GetComponentLocation())
		.GetSafeNormal2D();
	const FVector ActualDirection = (Muzzle->GetComponentLocation() - AimPivot->GetComponentLocation())
		.GetSafeNormal2D();
	TestTrue(TEXT("The turret muzzle points at the target after a Blueprint rotation overwrite"),
		FVector::DotProduct(ExpectedDirection, ActualDirection) > 0.999f);
	TestTrue(TEXT("Aiming the head does not rotate the turret base Actor"),
		Turret->GetActorRotation().Equals(BaseRotationBeforeAim, KINDA_SMALL_NUMBER));
	TestEqual(TEXT("The final aim correction runs after ordinary Blueprint timeline updates"),
		FireComponent->PrimaryComponentTick.TickGroup, TG_PostUpdateWork);

	FireComponent->ConfigureAttack(
		nullptr,
		EPBBumperProjectilePayload::None,
		0,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBTurretGenericMovableActivationTest,
	"PinBallLike.Bumper.Runtime.TurretGenericMovableActivation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBTurretGenericMovableActivationTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary turret action world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UClass* TurretClass = LoadClass<APBTurretSummonActor>(
		nullptr,
		TEXT("/Game/Blueprints/Bumper/Effect/BP_TurretSummon.BP_TurretSummon_C"));
	if (!TestNotNull(TEXT("The production turret Blueprint class resolves"), TurretClass))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	APBTurretSummonActor* Turret = IsValid(World)
		? World->SpawnActor<APBTurretSummonActor>(TurretClass, FTransform::Identity)
		: nullptr;
	APBBossBase* Boss = IsValid(World)
		? World->SpawnActor<APBBossBase>(
			APBBossBase::StaticClass(),
			FTransform(FVector(1000.0f, 0.0f, 0.0f)))
		: nullptr;
	AActor* GenericMovable = SpawnMovableRuntimeTestActor(World, false);
	if (!TestNotNull(TEXT("The production turret can be spawned"), Turret)
		|| !TestNotNull(TEXT("A Boss target can be spawned"), Boss)
		|| !TestNotNull(TEXT("A non-APBBallBase IMovable can be spawned"), GenericMovable))
	{
		return false;
	}
	if (!TestNotNull(TEXT("The turret action world has WorldSettings"),
		World->GetWorldSettings()))
	{
		return false;
	}
	World->GetWorldSettings()->DefaultGameMode = AGameModeBase::StaticClass();

	if (!TestTrue(TEXT("The turret action world can enter play"),
		TestWorld.BeginPlayInTestWorld()))
	{
		return false;
	}

	Turret->SetAttackPayload(
		EPBBumperProjectilePayload::BossDamage,
		1,
		1);
	Turret->StartActionForActor(nullptr, GenericMovable);
	TestWorld.TickTestWorld();

	TInlineComponentArray<UTimelineComponent*> TimelineComponents(Turret);
	const auto IsAnyActionTimelinePlaying = [&TimelineComponents]()
	{
		for (const UTimelineComponent* Timeline : TimelineComponents)
		{
			if (IsValid(Timeline)
				&& !Timeline->GetName().Contains(TEXT("Reaction"))
				&& Timeline->IsPlaying())
			{
				return true;
			}
		}

		return false;
	};

	if (!TestTrue(
		TEXT("A generic IMovable starts the production turret action timeline"),
		IsAnyActionTimelinePlaying()))
	{
		return false;
	}

	for (int32 TickIndex = 0; TickIndex < 3000 && IsAnyActionTimelinePlaying(); ++TickIndex)
	{
		TestWorld.TickTestWorld(0.01f);
	}
	if (!TestFalse(
		TEXT("The production turret action timeline finishes within 30 seconds"),
		IsAnyActionTimelinePlaying()))
	{
		for (const UTimelineComponent* Timeline : TimelineComponents)
		{
			if (IsValid(Timeline) && Timeline->IsPlaying())
			{
				AddInfo(FString::Printf(
					TEXT("Playing timeline: %s Position=%.2f Length=%.2f Looping=%s"),
					*Timeline->GetName(),
					Timeline->GetPlaybackPosition(),
					Timeline->GetTimelineLength(),
					Timeline->IsLooping() ? TEXT("true") : TEXT("false")));
			}
		}
		return false;
	}

	Turret->SetAttackPayload(
		EPBBumperProjectilePayload::BossDamage,
		1,
		1);
	Turret->StartActionForActor(nullptr, GenericMovable);
	TestWorld.TickTestWorld();
	TestTrue(
		TEXT("The same turret can start a second generic IMovable action"),
		IsAnyActionTimelinePlaying());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperProjectileVisualMeshTest,
	"PinBallLike.Bumper.Runtime.ProjectileCustomMeshVisualScale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperProjectileVisualMeshTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary projectile world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UClass* ProjectileClass = LoadClass<APBBumperProjectile>(
		nullptr,
		TEXT("/Game/Blueprints/Bumper/Effect/BP_TestBullet.BP_TestBullet_C"));
	UStaticMesh* CustomMesh = LoadObject<UStaticMesh>(
		nullptr,
		TEXT("/Game/Resources/Bumper/Mesh/SM_BumperProjectile_TurretRound_01.SM_BumperProjectile_TurretRound_01"));
	UMaterialInterface* ProjectileMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/Resources/Bumper/Material/M_BumperProjectiles.M_BumperProjectiles"));
	if (!TestNotNull(TEXT("The production bumper projectile class resolves"), ProjectileClass)
		|| !TestNotNull(TEXT("The turret round mesh resolves"), CustomMesh)
		|| !TestNotNull(TEXT("The authored bumper projectile material resolves"), ProjectileMaterial))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	APBBumperProjectile* Projectile = IsValid(World)
		? World->SpawnActor<APBBumperProjectile>(ProjectileClass, FTransform::Identity)
		: nullptr;
	AActor* Target = IsValid(World)
		? World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(FVector(1000.0f, 0.0f, 0.0f)))
		: nullptr;
	if (!TestNotNull(TEXT("The production bumper projectile can be spawned"), Projectile)
		|| !TestNotNull(TEXT("A projectile target can be spawned"), Target))
	{
		return false;
	}

	Projectile->ConfigureForTarget(
		Target,
		EPBBumperProjectilePayload::BossDamage,
		1,
		true,
		0.0f,
		nullptr,
		nullptr,
		nullptr,
		CustomMesh);
	UStaticMeshComponent* VisualMesh = Projectile->FindComponentByClass<UStaticMeshComponent>();
	if (!TestNotNull(TEXT("The production projectile has a visual mesh component"), VisualMesh))
	{
		return false;
	}

	TestTrue(TEXT("The requested projectile mesh is applied"),
		VisualMesh->GetStaticMesh() == CustomMesh);
	TestTrue(TEXT("A custom projectile mesh uses its authored material instead of a component override"),
		VisualMesh->GetMaterial(0) == ProjectileMaterial
		&& CustomMesh->GetMaterial(0) == ProjectileMaterial);
	TestTrue(TEXT("A custom projectile mesh uses its authored visual scale"),
		VisualMesh->GetRelativeScale3D().Equals(FVector(1.0f), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("A custom projectile mesh compensates for the authored Y-forward axis"),
		VisualMesh->GetRelativeRotation().Equals(FRotator(0.0f, -90.0f, 0.0f), KINDA_SMALL_NUMBER));
	USphereComponent* CollisionSphere = Projectile->FindComponentByClass<USphereComponent>();
	if (!TestNotNull(TEXT("The production projectile has a collision sphere"), CollisionSphere))
	{
		return false;
	}
	TestTrue(TEXT("Changing the visual mesh scale does not resize the collision sphere"),
		FMath::IsNearlyEqual(CollisionSphere->GetUnscaledSphereRadius(), 12.0f));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperEquippedDragSourceTest,
	"PinBallLike.Bumper.UI.EquippedDragTracksSourceSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperEquippedDragSourceTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	UPBBumperDragDropOperation* DragOperation = NewObject<UPBBumperDragDropOperation>();
	if (!TestNotNull(TEXT("Bumper drag operation can be created"), DragOperation))
	{
		return false;
	}

	const FName BumperRowName(TEXT("Bumper_Top_Test"));
	DragOperation->InitializeBumperDrag(BumperRowName);
	TestTrue(TEXT("A catalog drag remains valid"), DragOperation->IsValidBumperDrag());
	TestFalse(TEXT("A catalog drag has no equipped source slot"), DragOperation->HasSourceEquipSlot());
	TestEqual(TEXT("A new catalog drag starts pending"),
		DragOperation->GetResult(), EPBBumperDragResult::Pending);
	TestFalse(TEXT("A pending drag has not been handled"), DragOperation->WasDropHandled());

	DragOperation->InitializeEquippedBumperDrag(
		BumperRowName,
		EPBBumperEquipSlot::TopRight);
	TestTrue(TEXT("An equipped drag records its source slot"), DragOperation->HasSourceEquipSlot());
	TestEqual(
		TEXT("The equipped drag preserves the physical source slot"),
		DragOperation->SourceEquipSlot,
		EPBBumperEquipSlot::TopRight);
	TestEqual(
		TEXT("The equipped drag visual preserves the pointer offset inside its source icon"),
		DragOperation->Pivot,
		EDragPivot::MouseDown);
	TestEqual(
		TEXT("The equipped drag visual has no additional pointer offset"),
		DragOperation->Offset,
		FVector2D::ZeroVector);
	DragOperation->SetResult(EPBBumperDragResult::Cancelled);
	TestEqual(TEXT("A cancelled drag records cancellation without implying a drop"),
		DragOperation->GetResult(), EPBBumperDragResult::Cancelled);
	TestFalse(TEXT("Cancellation is not a handled drop"), DragOperation->WasDropHandled());

	DragOperation->InitializeEquippedBumperDrag(
		BumperRowName,
		EPBBumperEquipSlot::TopRight);
	DragOperation->SetResult(EPBBumperDragResult::Moved);
	DragOperation->SetResult(EPBBumperDragResult::Cancelled);
	TestEqual(TEXT("A completed drop result cannot be overwritten by late cancellation"),
		DragOperation->GetResult(), EPBBumperDragResult::Moved);
	TestTrue(TEXT("A move is a handled drop"), DragOperation->WasDropHandled());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperTriggerRuntimeStateTest,
	"PinBallLike.Bumper.Runtime.IndependentTriggerAndQueuedActivation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperTriggerRuntimeStateTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary game world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UClass* BumperClass = LoadClass<APBModularBumperBase>(
		nullptr,
		TEXT("/Game/Blueprints/Bumper/BP_ModularBumper.BP_ModularBumper_C"));
	if (!TestNotNull(TEXT("The production modular bumper Blueprint class resolves"), BumperClass))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	APBModularBumperBase* Bumper = SpawnRuntimeTestBumper(
		World,
		BumperClass,
		APBCollisionBumperTriggerActor::StaticClass(),
		{EPBBumperPositionId::ReboundLeft, EPBBumperPositionId::ReboundRight},
		3);
	AActor* InteractionActor = World
		? World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity)
		: nullptr;
	if (!TestNotNull(TEXT("The runtime acceptance bumper is spawned"), Bumper)
		|| !TestNotNull(TEXT("The interaction Actor is spawned"), InteractionActor))
	{
		return false;
	}
	Bumper->DispatchBeginPlay();

	TArray<APBBumperTriggerActorBase*> Triggers = FindOwnedRuntimeTestTriggers(World, Bumper);
	if (!TestEqual(TEXT("Two physical positions create two independent Trigger instances"), Triggers.Num(), 2))
	{
		return false;
	}

	APBBumperTriggerActorBase* LeftTrigger = Triggers[0];
	APBBumperTriggerActorBase* RightTrigger = Triggers[1];
	const FHitResult EmptyHit;
	Bumper->HandleTriggerActorActivated(LeftTrigger, InteractionActor, EmptyHit);
	Bumper->HandleTriggerActorActivated(RightTrigger, InteractionActor, EmptyHit);
	Bumper->HandleTriggerActorActivated(RightTrigger, InteractionActor, EmptyHit);

	TestEqual(TEXT("The left position keeps its own progress"), LeftTrigger->GetCurrentTriggerCount(), 1);
	TestEqual(TEXT("The right position keeps its own progress"), RightTrigger->GetCurrentTriggerCount(), 2);
	TestEqual(TEXT("Partial progress does not activate the Bumper"), Bumper->GetActivationCount(), 0);

	Bumper->HandleTriggerActorActivated(LeftTrigger, InteractionActor, EmptyHit);
	Bumper->HandleTriggerActorActivated(LeftTrigger, InteractionActor, EmptyHit);
	TestEqual(TEXT("Reaching the threshold activates exactly once"), Bumper->GetActivationCount(), 1);
	TestEqual(TEXT("The completed position resets after its effect finishes"),
		LeftTrigger->GetCurrentTriggerCount(), 0);
	TestEqual(TEXT("Completing one position does not reset the other position"),
		RightTrigger->GetCurrentTriggerCount(), 2);

	Bumper->ActiveTriggerActor = LeftTrigger;
	Bumper->RuntimeState.CurrentState = EPBBumperState::Activated;
	Bumper->HandleTriggerActorActivated(RightTrigger, InteractionActor, EmptyHit);
	TestEqual(TEXT("A ready position waits while the execution lane is occupied"),
		Bumper->GetPendingActivationCount(), 1);
	TestEqual(TEXT("The waiting position records the queued state"),
		RightTrigger->GetTriggerProgressState(), EPBBumperTriggerProgressState::Queued);

	Bumper->ActiveTriggerActor.Reset();
	Bumper->RuntimeState.CurrentState = EPBBumperState::Idle;
	Bumper->ProcessNextPendingActivation();
	TestEqual(TEXT("The queued request activates after the lane becomes idle"),
		Bumper->GetActivationCount(), 2);
	TestEqual(TEXT("The processed queue is empty"), Bumper->GetPendingActivationCount(), 0);
	TestEqual(TEXT("The queued position resets after completion"),
		RightTrigger->GetCurrentTriggerCount(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperTriggerBlueprintCompositionTest,
	"PinBallLike.Bumper.Runtime.TriggerBlueprintComposition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperTriggerBlueprintCompositionTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BumperTable = Settings ? Settings->BumperTable.LoadSynchronous() : nullptr;
	if (!TestNotNull(TEXT("The configured Bumper table resolves"), BumperTable))
	{
		return false;
	}

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary composition test world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	for (const TPair<FName, uint8*>& RowPair : BumperTable->GetRowMap())
	{
		const FString Context = RowPair.Key.ToString();
		const FPBBumperTableRow* BumperRow =
			reinterpret_cast<const FPBBumperTableRow*>(RowPair.Value);
		if (!TestNotNull(*FString::Printf(TEXT("Bumper row resolves: %s"), *Context), BumperRow))
		{
			continue;
		}

		UPBBumperDataAsset* DataAsset = BumperRow->BumperDataAsset.LoadSynchronous();
		UClass* TriggerClass = IsValid(DataAsset)
			? DataAsset->TriggerClass.LoadSynchronous()
			: nullptr;
		if (!TestNotNull(*FString::Printf(TEXT("Trigger class resolves: %s"), *Context), TriggerClass))
		{
			continue;
		}

		APBBumperTriggerActorBase* Trigger = World->SpawnActor<APBBumperTriggerActorBase>(
			TriggerClass,
			FTransform::Identity,
			FActorSpawnParameters());
		if (!TestNotNull(*FString::Printf(TEXT("Trigger Blueprint spawns: %s"), *Context), Trigger))
		{
			continue;
		}

		const TArray<UActorComponent*> TriggerAreas = Trigger->GetComponentsByTag(
			UPrimitiveComponent::StaticClass(),
			TEXT("BumperTrigger"));
		TestTrue(
			*FString::Printf(TEXT("Trigger Area tag exists: %s"), *Context),
			!TriggerAreas.IsEmpty());

		if (BumperRow->BumperType == EPBBumperType::Gate)
		{
			TestTrue(
				*FString::Printf(TEXT("Gate row uses the Gate trigger implementation: %s"), *Context),
				Trigger->IsA<APBGateBumperTriggerActor>());
		}
		else
		{
			TestTrue(
				*FString::Printf(TEXT("Physical row uses the collision trigger implementation: %s"), *Context),
				Trigger->IsA<APBCollisionBumperTriggerActor>());
			const TArray<UActorComponent*> CollisionAreas = Trigger->GetComponentsByTag(
				UPrimitiveComponent::StaticClass(),
				TEXT("BumperCollision"));
			TestTrue(
				*FString::Printf(TEXT("Collision Area tag exists: %s"), *Context),
				!CollisionAreas.IsEmpty());
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperRepresentativeInputPathTest,
	"PinBallLike.Bumper.Runtime.RepresentativeCollisionAndGateInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperRepresentativeInputPathTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FTestWorldWrapper TestWorld;
	if (!TestTrue(TEXT("A temporary representative test world can be created"),
		TestWorld.CreateTestWorld(EWorldType::Game)))
	{
		return false;
	}

	UWorld* World = TestWorld.GetTestWorld();
	UClass* BumperClass = LoadClass<APBModularBumperBase>(
		nullptr,
		TEXT("/Game/Blueprints/Bumper/BP_ModularBumper.BP_ModularBumper_C"));
	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BumperTable = Settings ? Settings->BumperTable.LoadSynchronous() : nullptr;
	const FPBBumperTableRow* ReboundRow = BumperTable
		? BumperTable->FindRow<FPBBumperTableRow>(
			PBBumperAssetIds::Bumper::Rebound_PowerPush,
			TEXT("Representative Rebound"),
			false)
		: nullptr;
	const FPBBumperTableRow* GateRow = BumperTable
		? BumperTable->FindRow<FPBBumperTableRow>(
			PBBumperAssetIds::Bumper::Gate_SpeedUp,
			TEXT("Representative Gate"),
			false)
		: nullptr;
	UPBBumperDataAsset* ReboundDataAsset = ReboundRow
		? ReboundRow->BumperDataAsset.LoadSynchronous()
		: nullptr;
	UPBBumperDataAsset* GateDataAsset = GateRow
		? GateRow->BumperDataAsset.LoadSynchronous()
		: nullptr;
	UClass* ReboundTriggerClass = IsValid(ReboundDataAsset)
		? ReboundDataAsset->TriggerClass.LoadSynchronous()
		: nullptr;
	UClass* GateTriggerClass = IsValid(GateDataAsset)
		? GateDataAsset->TriggerClass.LoadSynchronous()
		: nullptr;
	AActor* MovableActor = SpawnMovableRuntimeTestActor(World);
	UPBBallPhysicsComponent* PhysicsComponent = IsValid(MovableActor)
		? MovableActor->FindComponentByClass<UPBBallPhysicsComponent>()
		: nullptr;
	if (!TestNotNull(TEXT("The production modular Bumper class resolves"), BumperClass)
		|| !TestNotNull(TEXT("The representative Rebound row resolves"), ReboundRow)
		|| !TestNotNull(TEXT("The representative Gate row resolves"), GateRow)
		|| !TestNotNull(TEXT("The representative Rebound Trigger Blueprint resolves"), ReboundTriggerClass)
		|| !TestNotNull(TEXT("The representative Gate Trigger Blueprint resolves"), GateTriggerClass)
		|| !TestNotNull(TEXT("A component-based IMovable test Actor is spawned"), MovableActor)
		|| !TestNotNull(TEXT("The test Actor owns its movement component"), PhysicsComponent))
	{
		return false;
	}
	if (!TestNotNull(
		TEXT("The component-based test Actor resolves through the project IMovable lookup"),
		PBInterfaceUtils::FindInterface<IMovable>(MovableActor)))
	{
		return false;
	}

	APBModularBumperBase* CollisionBumper = SpawnRuntimeTestBumper(
		World,
		BumperClass,
		ReboundTriggerClass,
		{EPBBumperPositionId::ReboundLeft},
		3);
	if (!TestNotNull(TEXT("The representative collision Bumper is spawned"), CollisionBumper))
	{
		return false;
	}
	CollisionBumper->DispatchBeginPlay();
	TArray<APBBumperTriggerActorBase*> CollisionTriggers =
		FindOwnedRuntimeTestTriggers(World, CollisionBumper);
	if (!TestEqual(TEXT("The collision Bumper creates one Trigger"), CollisionTriggers.Num(), 1))
	{
		return false;
	}

	APBCollisionBumperTriggerActor* CollisionTrigger =
		Cast<APBCollisionBumperTriggerActor>(CollisionTriggers[0]);
	if (!TestNotNull(TEXT("The representative Trigger uses collision semantics"), CollisionTrigger))
	{
		return false;
	}
	CollisionTrigger->DispatchBeginPlay();
	const TArray<UActorComponent*> CollisionAreaComponents = CollisionTrigger->GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		TEXT("BumperCollision"));
	const TArray<UActorComponent*> TriggerAreaComponents = CollisionTrigger->GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		TEXT("BumperTrigger"));
	if (!TestTrue(TEXT("The representative collision Trigger has a collision area"),
		!CollisionAreaComponents.IsEmpty())
		|| !TestTrue(TEXT("The representative collision Trigger has an intended face area"),
			!TriggerAreaComponents.IsEmpty()))
	{
		return false;
	}
	TestTrue(TEXT("The collision Trigger can currently increase progress"),
		CollisionTrigger->CanIncreaseTrigger());

	UPrimitiveComponent* CollisionArea = Cast<UPrimitiveComponent>(CollisionAreaComponents[0]);
	UPrimitiveComponent* TriggerArea = Cast<UPrimitiveComponent>(TriggerAreaComponents[0]);
	UPrimitiveComponent* MovablePrimitive = Cast<UPrimitiveComponent>(MovableActor->GetRootComponent());
	TestTrue(TEXT("The intended face overlap delegate is bound"),
		TriggerArea->OnComponentBeginOverlap.IsBound());
	TestTrue(TEXT("The physical collision delegate is bound"),
		CollisionArea->OnComponentHit.IsBound());
	FHitResult OverlapHit;
	CollisionTrigger->HandleTriggerBeginOverlap(
		TriggerArea,
		MovableActor,
		MovablePrimitive,
		0,
		false,
		OverlapHit);
	FHitResult CollisionHit;
	CollisionHit.ImpactPoint = TriggerArea->GetComponentLocation();
	FVector ExpectedBounceDirection = MovableActor->GetActorLocation() - CollisionHit.ImpactPoint;
	ExpectedBounceDirection.Z = 0.0f;
	if (!ExpectedBounceDirection.Normalize())
	{
		ExpectedBounceDirection = -FVector::XAxisVector;
	}
	CollisionHit.ImpactNormal = -ExpectedBounceDirection;
	CollisionHit.Normal = CollisionHit.ImpactNormal;
	const FVector VelocityBeforeHit = PhysicsComponent->GetVelocity();
	CollisionTrigger->HandleComponentHit(
		CollisionArea,
		MovableActor,
		MovablePrimitive,
		FVector::ZeroVector,
		CollisionHit);
	TestTrue(TEXT("The Bumper boost waits until the base collision response has completed"),
		PhysicsComponent->GetVelocity().Equals(VelocityBeforeHit));
	CollisionTrigger->HandleComponentHit(
		CollisionArea,
		MovableActor,
		MovablePrimitive,
		FVector::ZeroVector,
		CollisionHit);
	TestEqual(TEXT("Repeated hit events from the same contact add only one count"),
		CollisionTrigger->GetCurrentTriggerCount(), 1);

	AActor* MovableWithoutStats = SpawnMovableRuntimeTestActor(World, false);
	UPBBallPhysicsComponent* PhysicsWithoutStats = IsValid(MovableWithoutStats)
		? MovableWithoutStats->FindComponentByClass<UPBBallPhysicsComponent>()
		: nullptr;
	if (!TestNotNull(TEXT("A generic IMovable Actor without a stat provider can be spawned"),
		MovableWithoutStats)
		|| !TestNotNull(TEXT("The generic IMovable Actor owns its movement component"),
		PhysicsWithoutStats))
	{
		return false;
	}
	const FVector GenericIncomingVelocity = -ExpectedBounceDirection * 2000.0f;
	PhysicsWithoutStats->SetVelocity(GenericIncomingVelocity);
	TestTrue(TEXT("A generic IMovable Actor can queue the base Bumper rebound"),
		CollisionTrigger->QueueBounceVelocity(MovableWithoutStats, CollisionHit));
	TestTrue(TEXT("The generic IMovable rebound is also deferred"),
		PhysicsWithoutStats->GetVelocity().Equals(GenericIncomingVelocity));

	World->GetTimerManager().Tick(0.001f);
	TestEqual(TEXT("A hit on the intended face adds one independent count"),
		CollisionTrigger->GetCurrentTriggerCount(), 1);
	TestEqual(TEXT("A valid hit is recorded as one meaningful contact"),
		CollisionBumper->GetMeaningfulContactCount(), 1);
	TestFalse(TEXT("The physical rebound path changes the Movable velocity"),
		PhysicsComponent->GetVelocity().Equals(VelocityBeforeHit));
	TestTrue(TEXT("The deferred Bumper boost is oriented away from the contact point"),
		FVector::DotProduct(
			PhysicsComponent->GetVelocity() - VelocityBeforeHit,
			ExpectedBounceDirection) >= CollisionTrigger->BounceVelocityStrength);
	TestTrue(TEXT("The base Bumper rebound does not require a stat provider"),
		PhysicsWithoutStats->GetVelocity().Equals(
			GenericIncomingVelocity
				+ ExpectedBounceDirection * CollisionTrigger->BounceVelocityStrength,
			KINDA_SMALL_NUMBER));

	APBModularBumperBase* GateBumper = SpawnRuntimeTestBumper(
		World,
		BumperClass,
		GateTriggerClass,
		{EPBBumperPositionId::GateCenterMid},
		3);
	if (!TestNotNull(TEXT("The representative Gate Bumper is spawned"), GateBumper))
	{
		return false;
	}
	GateBumper->DispatchBeginPlay();
	TArray<APBBumperTriggerActorBase*> GateTriggers =
		FindOwnedRuntimeTestTriggers(World, GateBumper);
	if (!TestEqual(TEXT("The Gate Bumper creates one Trigger"), GateTriggers.Num(), 1))
	{
		return false;
	}

	APBGateBumperTriggerActor* GateTrigger = Cast<APBGateBumperTriggerActor>(GateTriggers[0]);
	if (!TestNotNull(TEXT("The representative Trigger uses Gate semantics"), GateTrigger))
	{
		return false;
	}
	GateTrigger->DispatchBeginPlay();
	const TArray<UActorComponent*> GateAreaComponents = GateTrigger->GetComponentsByTag(
		UPrimitiveComponent::StaticClass(),
		TEXT("BumperTrigger"));
	if (!TestTrue(TEXT("The Gate Trigger has a pass area"), !GateAreaComponents.IsEmpty()))
	{
		return false;
	}
	TestTrue(TEXT("The Gate Trigger can currently increase progress"),
		GateTrigger->CanIncreaseTrigger());

	UPrimitiveComponent* GateArea = Cast<UPrimitiveComponent>(GateAreaComponents[0]);
	TestTrue(TEXT("The Gate pass delegates are bound"),
		GateArea->OnComponentBeginOverlap.IsBound()
		&& GateArea->OnComponentEndOverlap.IsBound());
	GateTrigger->HandleGateBeginOverlap(
		GateArea,
		MovableActor,
		MovablePrimitive,
		0,
		false,
		OverlapHit);
	GateTrigger->HandleGateEndOverlap(
		GateArea,
		MovableActor,
		MovablePrimitive,
		0);
	TestEqual(TEXT("One complete Gate pass adds exactly one count"),
		GateTrigger->GetCurrentTriggerCount(), 1);
	TestEqual(TEXT("The Gate pass is recorded as one meaningful contact"),
		GateBumper->GetMeaningfulContactCount(), 1);

	return true;
}

#endif
