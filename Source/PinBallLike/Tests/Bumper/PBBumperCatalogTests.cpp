#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "NiagaraEffectType.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossDamageBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossGroggyBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossVulnerabilityBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBloodOverdriveBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Effect/PBComboArcBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBComboCashoutBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBCounterShieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBDirectRewardBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBGateSupportFieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBKineticShellBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBPercentShieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBSummonBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBTimedAttackBoostBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBTurretSummonBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBVelocityBoostBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonAnchor.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateAccelerationField.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateSupportField.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectRow.h"
#include "PinBallLike/Utils/PBTextFormatUtils.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperCatalogTest,
	"PinBallLike.Bumper.Catalog.FivePerTypeAndPrimaryAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperCatalogTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	const APBGateAccelerationField* AccelerationField = GetDefault<APBGateAccelerationField>();
	const APBGateSupportField* SupportField = GetDefault<APBGateSupportField>();
	TestEqual(
		TEXT("Gate field radius is twice the previous 180uu radius"),
		PBGateFieldTuning::DefaultRadius,
		360.0f);
	const FTransform ExampleGateTransform(
		FRotator(0.0f, 37.0f, 0.0f),
		FVector(125.0f, -240.0f, 18.0f),
		FVector(2.0f, 0.5f, 3.0f));
	const FTransform FieldAtGateOrigin =
		PBGateFieldTuning::MakeTransformAtGateOrigin(ExampleGateTransform);
	TestTrue(
		TEXT("Gate field keeps the Gate world origin"),
		FieldAtGateOrigin.GetLocation().Equals(ExampleGateTransform.GetLocation()));
	TestTrue(
		TEXT("Gate field keeps the Gate world rotation"),
		FieldAtGateOrigin.GetRotation().Equals(ExampleGateTransform.GetRotation()));
	TestTrue(
		TEXT("Gate field ignores Gate mesh scale"),
		FieldAtGateOrigin.GetScale3D().Equals(FVector::OneVector));
	TestNotNull(
		TEXT("Gate field debug console variable is registered"),
		IConsoleManager::Get().FindConsoleVariable(TEXT("pb.Bumper.DebugGateFields")));
	TestEqual(
		TEXT("Gate acceleration field uses the shared combat radius"),
		AccelerationField->GetFieldRadius(),
		PBGateFieldTuning::DefaultRadius);
	TestEqual(
		TEXT("Gate support field uses the shared combat radius"),
		SupportField->GetFieldRadius(),
		PBGateFieldTuning::DefaultRadius);
	const USphereComponent* AccelerationArea =
		AccelerationField->FindComponentByClass<USphereComponent>();
	const USphereComponent* SupportArea =
		SupportField->FindComponentByClass<USphereComponent>();
	if (TestNotNull(TEXT("Gate acceleration field has a radial collision area"), AccelerationArea))
	{
		TestEqual(
			TEXT("Gate acceleration radial collision matches the shared radius"),
			AccelerationArea->GetUnscaledSphereRadius(),
			PBGateFieldTuning::DefaultRadius);
	}
	if (TestNotNull(TEXT("Gate support field has a radial collision area"), SupportArea))
	{
		TestEqual(
			TEXT("Gate support radial collision matches the shared radius"),
			SupportArea->GetUnscaledSphereRadius(),
			PBGateFieldTuning::DefaultRadius);
	}

	const UPBSummonBumperEffect* GenericSummonEffect = GetDefault<UPBSummonBumperEffect>();
	const UPBTurretSummonBumperEffect* TurretSummonEffect = GetDefault<UPBTurretSummonBumperEffect>();
	TestEqual(
		TEXT("Generic summon effects preserve Bumper-relative placement"),
		static_cast<uint8>(GenericSummonEffect->GetSpawnAnchorType()),
		static_cast<uint8>(EPBBumperSummonAnchorType::None));
	TestEqual(
		TEXT("Turret summon effects opt into Turret Anchors"),
		static_cast<uint8>(TurretSummonEffect->GetSpawnAnchorType()),
		static_cast<uint8>(EPBBumperSummonAnchorType::Turret));

	const FName ExpectedRows[] =
	{
		PBBumperAssetIds::Bumper::Rebound_PowerPush,
		PBBumperAssetIds::Bumper::Rebound_KineticShell,
		PBBumperAssetIds::Bumper::Rebound_ComboPulse,
		PBBumperAssetIds::Bumper::Rebound_BloodOverdrive,
		PBBumperAssetIds::Bumper::Rebound_GroggyHammer,
		PBBumperAssetIds::Bumper::Side_ShieldCharge,
		PBBumperAssetIds::Bumper::Side_ManaCharge,
		PBBumperAssetIds::Bumper::Side_RepairPickup,
		PBBumperAssetIds::Bumper::Side_CounterShield,
		PBBumperAssetIds::Bumper::Side_StrengthCharge,
		PBBumperAssetIds::Bumper::Top_ComboArc,
		PBBumperAssetIds::Bumper::Top_GroggyStrike,
		PBBumperAssetIds::Bumper::Top_DirectStrike,
		PBBumperAssetIds::Bumper::Top_ComboCashout,
		PBBumperAssetIds::Bumper::Top_VulnerabilityShell,
		PBBumperAssetIds::Bumper::Gate_SpeedUp,
		PBBumperAssetIds::Bumper::Gate_RecoveryField,
		PBBumperAssetIds::Bumper::Gate_ReactiveRepair,
		PBBumperAssetIds::Bumper::Gate_ManaReactor,
		PBBumperAssetIds::Bumper::Gate_LaunchCharge
	};

	const TMap<FName, int32> ExpectedTriggerCounts =
	{
		{PBBumperAssetIds::Bumper::Rebound_PowerPush, 6},
		{PBBumperAssetIds::Bumper::Rebound_KineticShell, 7},
		{PBBumperAssetIds::Bumper::Rebound_ComboPulse, 7},
		{PBBumperAssetIds::Bumper::Rebound_BloodOverdrive, 8},
		{PBBumperAssetIds::Bumper::Rebound_GroggyHammer, 8},
		{PBBumperAssetIds::Bumper::Side_ShieldCharge, 6},
		{PBBumperAssetIds::Bumper::Side_ManaCharge, 7},
		{PBBumperAssetIds::Bumper::Side_RepairPickup, 6},
		{PBBumperAssetIds::Bumper::Side_CounterShield, 7},
		{PBBumperAssetIds::Bumper::Side_StrengthCharge, 7},
		{PBBumperAssetIds::Bumper::Top_ComboArc, 4},
		{PBBumperAssetIds::Bumper::Top_GroggyStrike, 5},
		{PBBumperAssetIds::Bumper::Top_DirectStrike, 5},
		{PBBumperAssetIds::Bumper::Top_ComboCashout, 5},
		{PBBumperAssetIds::Bumper::Top_VulnerabilityShell, 4},
		{PBBumperAssetIds::Bumper::Gate_SpeedUp, 3},
		{PBBumperAssetIds::Bumper::Gate_RecoveryField, 3},
		{PBBumperAssetIds::Bumper::Gate_ReactiveRepair, 3},
		{PBBumperAssetIds::Bumper::Gate_ManaReactor, 3},
		{PBBumperAssetIds::Bumper::Gate_LaunchCharge, 4}
	};

	struct FEffectBalanceExpectation
	{
		float Power = 0.0f;
		float SecondaryPower = 0.0f;
		float Duration = 0.0f;
		int32 Count = 0;
		FName SharedEffectId = NAME_None;
	};

	const TMap<FName, FEffectBalanceExpectation> ExpectedEffectBalance =
	{
		{PBBumperAssetIds::Effect::Effect_VelocityBoost_01, {3000.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_KineticShell_01,
			{3.0f, 1000.0f, 0.0f, 8, TEXT("E_Bumper_Rebound_KineticShell")}},
		{PBBumperAssetIds::Effect::Effect_ComboPulse_01, {5.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_BloodOverdrive_01,
			{25.0f, 1.0f, 5.0f, 1, TEXT("E_Bumper_Rebound_BloodOverdrive")}},
		{PBBumperAssetIds::Effect::Effect_GroggyHammer_02, {10.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_Shield_02, {1.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_PartyMana_02, {5.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_RepairPickup_02, {5.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_CounterShield_01, {1.0f, 5.0f, 10.0f, 1, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_StrengthCharge_02, {15.0f, 0.0f, 5.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_ComboArc_01,
			{2.0f, 10.0f, 10.0f, 0, TEXT("E_Bumper_Top_ComboArc")}},
		{PBBumperAssetIds::Effect::Effect_BossGroggy_02, {20.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_DirectStrike_02, {10.0f, 0.0f, 0.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_ComboCashout_02, {1.0f, 0.0f, 0.0f, 20, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_VulnerabilityShell_01, {20.0f, 0.0f, 6.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_SpeedUp_01, {25.0f, 0.0f, 5.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_RecoveryField_02, {5.0f, 0.0f, 5.0f, 0, NAME_None}},
		{PBBumperAssetIds::Effect::Effect_ReactiveRepair_01,
			{1.0f, 0.0f, 10.0f, 2, TEXT("E_Bumper_Gate_ReactiveRepair")}},
		{PBBumperAssetIds::Effect::Effect_ManaReactor_01,
			{25.0f, 0.0f, 8.0f, 0, TEXT("E_Bumper_Gate_ManaReactor")}},
		{PBBumperAssetIds::Effect::Effect_LaunchCharge_02, {5.0f, 0.0f, 0.0f, 3, NAME_None}}
	};

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BumperTable = Settings ? Settings->BumperTable.LoadSynchronous() : nullptr;
	UDataTable* TriggerTable = Settings ? Settings->BumperTriggerTable.LoadSynchronous() : nullptr;
	UDataTable* EffectTable = Settings ? Settings->BumperEffectTable.LoadSynchronous() : nullptr;
	UDataTable* GameplayEffectTable = Settings ? Settings->GameplayEffectTable.LoadSynchronous() : nullptr;
	UDataTable* GameplayEffectParamTable = Settings
		? Settings->GameplayEffectParamTable.LoadSynchronous()
		: nullptr;
	if (!TestNotNull(TEXT("Bumper DataTable is configured"), BumperTable)
		|| !TestNotNull(TEXT("Bumper Trigger DataTable is configured"), TriggerTable)
		|| !TestNotNull(TEXT("Bumper Effect DataTable is configured"), EffectTable)
		|| !TestNotNull(TEXT("Gameplay Effect DataTable is configured"), GameplayEffectTable)
		|| !TestNotNull(TEXT("Gameplay Effect Param DataTable is configured"), GameplayEffectParamTable))
	{
		return false;
	}

	TestEqual(TEXT("Bumper catalog contains exactly 20 rows"), BumperTable->GetRowMap().Num(), 20);
	TestEqual(TEXT("Bumper Trigger table contains exactly 20 rows"), TriggerTable->GetRowMap().Num(), 20);
	TestEqual(TEXT("Bumper Effect table contains exactly 20 rows"), EffectTable->GetRowMap().Num(), 20);

	TMap<EPBBumperType, int32> TypeCounts;
	TSet<FName> ReferencedTriggerIds;
	TSet<FName> ReferencedEffectIds;
	const TSet<FName> GateAreaEffectIds =
	{
		TEXT("Effect_SpeedUp_01"),
		TEXT("Effect_RecoveryField_02"),
		TEXT("Effect_ReactiveRepair_01"),
		TEXT("Effect_ManaReactor_01")
	};
	const FClassProperty* SummonActorClassProperty = FindFProperty<FClassProperty>(
		UPBSummonBumperEffect::StaticClass(),
		TEXT("SummonActorClass"));
	TestNotNull(TEXT("Summon effect exposes its configured Actor class"), SummonActorClassProperty);
	int32 ConfiguredVfxStageCount = 0;
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
		TestTrue(*FString::Printf(TEXT("RequiredTriggerCount is positive: %s"), *RowPair.Key.ToString()),
			Row->RequiredTriggerCount > 0);
		const int32* ExpectedTriggerCount = ExpectedTriggerCounts.Find(RowPair.Key);
		if (TestNotNull(
			*FString::Printf(TEXT("Balance trigger count is registered: %s"), *RowPair.Key.ToString()),
			ExpectedTriggerCount))
		{
			TestEqual(
				*FString::Printf(TEXT("Balance trigger count matches: %s"), *RowPair.Key.ToString()),
				Row->RequiredTriggerCount,
				*ExpectedTriggerCount);
		}

		ReferencedTriggerIds.Add(Row->TriggerID);
		ReferencedEffectIds.Add(Row->EffectID);

		const FPBBumperTriggerRow* TriggerRow = TriggerTable->FindRow<FPBBumperTriggerRow>(
			Row->TriggerID,
			RowPair.Key.ToString(),
			false);
		if (TestNotNull(
			*FString::Printf(TEXT("Linked Trigger row resolves: %s"), *RowPair.Key.ToString()),
			TriggerRow))
		{
			TArray<EPBBumperPositionId> ExpectedPositions;
			switch (Row->BumperType)
			{
			case EPBBumperType::Rebound:
				ExpectedPositions = {EPBBumperPositionId::ReboundLeft, EPBBumperPositionId::ReboundRight};
				break;
			case EPBBumperType::Side:
				ExpectedPositions = {EPBBumperPositionId::SideLeft, EPBBumperPositionId::SideRight};
				break;
			case EPBBumperType::TopTarget:
				ExpectedPositions = {EPBBumperPositionId::TopTargetLeft, EPBBumperPositionId::TopTargetRight};
				break;
			case EPBBumperType::Gate:
				ExpectedPositions = {EPBBumperPositionId::GateCenterMid};
				break;
			default:
				AddError(FString::Printf(TEXT("Unsupported BumperType: %s"), *RowPair.Key.ToString()));
				break;
			}

			TestEqual(
				*FString::Printf(TEXT("Trigger position count matches type: %s"), *RowPair.Key.ToString()),
				TriggerRow->PositionIds.Num(),
				ExpectedPositions.Num());
			for (const EPBBumperPositionId ExpectedPosition : ExpectedPositions)
			{
				TestTrue(
					*FString::Printf(
						TEXT("Trigger supports expected position %s: %s"),
						*UEnum::GetValueAsString(ExpectedPosition),
						*RowPair.Key.ToString()),
					TriggerRow->PositionIds.Contains(ExpectedPosition));
			}

			const FString FormattedTriggerDescription = PBTextFormatUtils::FormatSingleValueTemplate(
				TriggerRow->TriggerDescription,
				FText::AsNumber(Row->RequiredTriggerCount)).ToString();
			TestFalse(
				*FString::Printf(TEXT("Trigger description resolves {0}: %s"), *RowPair.Key.ToString()),
				FormattedTriggerDescription.Contains(TEXT("{0}")));
		}

		const FPBBumperEffectRow* EffectRow = EffectTable->FindRow<FPBBumperEffectRow>(
			Row->EffectID,
			RowPair.Key.ToString(),
			false);
		if (TestNotNull(
			*FString::Printf(TEXT("Linked Effect row resolves: %s"), *RowPair.Key.ToString()),
			EffectRow))
		{
			TestEqual(
				*FString::Printf(TEXT("Bumper and Effect types match: %s"), *RowPair.Key.ToString()),
				static_cast<uint8>(Row->EffectType),
				static_cast<uint8>(EffectRow->EffectType));
			TestTrue(
				*FString::Printf(TEXT("Effect Power is finite and positive: %s"), *RowPair.Key.ToString()),
				FMath::IsFinite(EffectRow->Power) && EffectRow->Power > 0.0f);

			const FString FormattedEffectDescription = PBTextFormatUtils::FormatSingleValueTemplate(
				EffectRow->Description,
				FText::AsNumber(EffectRow->Power)).ToString();
			TestFalse(
				*FString::Printf(TEXT("Effect description resolves {0}: %s"), *RowPair.Key.ToString()),
				FormattedEffectDescription.Contains(TEXT("{0}")));
		}

		UPBBumperDataAsset* BumperDataAsset = Row->BumperDataAsset.LoadSynchronous();
		if (TestNotNull(
			*FString::Printf(TEXT("DataAsset resolves: %s"), *RowPair.Key.ToString()),
			BumperDataAsset))
		{
			TestEqual(
				*FString::Printf(TEXT("DataAsset RowName matches table: %s"), *RowPair.Key.ToString()),
				BumperDataAsset->RowName,
				RowPair.Key);
			TestEqual(
				*FString::Printf(TEXT("DataAsset PrimaryAssetId matches table: %s"), *RowPair.Key.ToString()),
				BumperDataAsset->GetPrimaryAssetId().ToString(),
				FPrimaryAssetId(PBBumperAssetIds::Type::BumperData, RowPair.Key).ToString());
			TestNotNull(
				*FString::Printf(TEXT("Icon resolves: %s"), *RowPair.Key.ToString()),
				BumperDataAsset->Icon.LoadSynchronous());

			UClass* TriggerClass = BumperDataAsset->TriggerClass.LoadSynchronous();
			TestTrue(
				*FString::Printf(TEXT("TriggerClass derives from the bumper trigger base: %s"), *RowPair.Key.ToString()),
				IsValid(TriggerClass) && TriggerClass->IsChildOf(APBBumperTriggerActorBase::StaticClass()));
			UClass* EffectClass = BumperDataAsset->EffectClass.LoadSynchronous();
			TestTrue(
				*FString::Printf(TEXT("EffectClass derives from the bumper effect base: %s"), *RowPair.Key.ToString()),
				IsValid(EffectClass) && EffectClass->IsChildOf(UPBBumperEffectBase::StaticClass()));

			if (GateAreaEffectIds.Contains(Row->EffectID)
				&& IsValid(EffectClass)
				&& SummonActorClassProperty)
			{
				const UObject* EffectDefaultObject = EffectClass->GetDefaultObject();
				UClass* SummonActorClass = Cast<UClass>(
					SummonActorClassProperty->GetObjectPropertyValue_InContainer(EffectDefaultObject));
				if (TestNotNull(
					*FString::Printf(TEXT("Gate area SummonActorClass resolves: %s"), *RowPair.Key.ToString()),
					SummonActorClass))
				{
					const bool bIsAccelerationField = Row->EffectID == TEXT("Effect_SpeedUp_01");
					if (bIsAccelerationField)
					{
						TestTrue(
							TEXT("SpeedUp summons a Gate acceleration field"),
							SummonActorClass->IsChildOf(APBGateAccelerationField::StaticClass()));
						const APBGateAccelerationField* FieldDefaultObject =
							SummonActorClass->GetDefaultObject<APBGateAccelerationField>();
						if (TestNotNull(TEXT("SpeedUp field defaults resolve"), FieldDefaultObject))
						{
							TestEqual(
								TEXT("SpeedUp Blueprint field radius matches the shared radius"),
								FieldDefaultObject->GetFieldRadius(),
								PBGateFieldTuning::DefaultRadius);
						}
					}
					else
					{
						TestTrue(
							*FString::Printf(TEXT("Support effect summons a Gate support field: %s"), *RowPair.Key.ToString()),
							SummonActorClass->IsChildOf(APBGateSupportField::StaticClass()));
						const APBGateSupportField* FieldDefaultObject =
							SummonActorClass->GetDefaultObject<APBGateSupportField>();
						if (TestNotNull(
							*FString::Printf(TEXT("Support field defaults resolve: %s"), *RowPair.Key.ToString()),
							FieldDefaultObject))
						{
							TestEqual(
								*FString::Printf(TEXT("Support Blueprint field radius matches the shared radius: %s"), *RowPair.Key.ToString()),
								FieldDefaultObject->GetFieldRadius(),
								PBGateFieldTuning::DefaultRadius);
						}
					}
				}
			}

			if (EffectRow)
			{
				TestFalse(
					*FString::Printf(TEXT("ActivationVfxId is configured: %s"), *RowPair.Key.ToString()),
					EffectRow->ActivationVfxId.IsNone());

				auto ValidateVfxStage = [this, &ConfiguredVfxStageCount, &RowPair](
					const TCHAR* StageName,
					const FName VfxId,
					const TSoftObjectPtr<UNiagaraSystem>& VfxReference)
				{
					const FString Context = FString::Printf(
						TEXT("%s VFX: %s"),
						StageName,
						*RowPair.Key.ToString());
					if (VfxId.IsNone())
					{
						TestTrue(*FString::Printf(TEXT("Unused %s reference is empty"), *Context), VfxReference.IsNull());
						return;
					}

					++ConfiguredVfxStageCount;
					UNiagaraSystem* System = VfxReference.LoadSynchronous();
					if (!TestNotNull(*FString::Printf(TEXT("%s resolves"), *Context), System))
					{
						return;
					}
					TestEqual(*FString::Printf(TEXT("%s asset name matches sheet ID"), *Context), System->GetFName(), VfxId);
					TestEqual(*FString::Printf(TEXT("%s uses three visual layers"), *Context), System->GetEmitterHandles().Num(), 3);
					UNiagaraEffectType* EffectType = System->GetEffectType();
					if (TestNotNull(*FString::Printf(TEXT("%s has a shared Effect Type"), *Context), EffectType))
					{
						TestEqual(
							*FString::Printf(TEXT("%s uses NET_BumperGameplay"), *Context),
							EffectType->GetFName(),
							FName(TEXT("NET_BumperGameplay")));
					}
				};

				ValidateVfxStage(TEXT("Activation"), EffectRow->ActivationVfxId, BumperDataAsset->ActivationVfx);
				ValidateVfxStage(TEXT("Delivery"), EffectRow->DeliveryVfxId, BumperDataAsset->DeliveryVfx);
				ValidateVfxStage(TEXT("Impact"), EffectRow->ImpactVfxId, BumperDataAsset->ImpactVfx);
				ValidateVfxStage(TEXT("Status"), EffectRow->StatusVfxId, BumperDataAsset->StatusVfx);
			}
		}

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
	TestEqual(TEXT("Bumper VFX stage reference count"), ConfiguredVfxStageCount, 64);

	struct FDeliveryExpectation
	{
		FName BumperId;
		EPBBumperRoleType RoleType;
		EPBBumperEffectType EffectType;
		EPBBumperEffectExecutionPolicy ExecutionPolicy;
		float Power;
		UClass* EffectBaseClass;
		FString DescriptionKeyword;
	};

	const FDeliveryExpectation DeliveryExpectations[] =
	{
		{PBBumperAssetIds::Bumper::Rebound_PowerPush, EPBBumperRoleType::Support,
			EPBBumperEffectType::Instant, EPBBumperEffectExecutionPolicy::Immediate,
			3000.0f, UPBVelocityBoostBumperEffect::StaticClass(), TEXT("보정")},
		{PBBumperAssetIds::Bumper::Rebound_KineticShell, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			3.0f, UPBKineticShellBumperEffect::StaticClass(), TEXT("속도")},
		{PBBumperAssetIds::Bumper::Rebound_GroggyHammer, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			10.0f, UPBBossGroggyBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Side_ShieldCharge, EPBBumperRoleType::Support,
			EPBBumperEffectType::Buff, EPBBumperEffectExecutionPolicy::Immediate,
			1.0f, UPBPercentShieldBumperEffect::StaticClass(), TEXT("최대 체력")},
		{PBBumperAssetIds::Bumper::Side_StrengthCharge, EPBBumperRoleType::Support,
			EPBBumperEffectType::Buff, EPBBumperEffectExecutionPolicy::Immediate,
			15.0f, UPBTimedAttackBoostBumperEffect::StaticClass(), TEXT("5초")},
		{PBBumperAssetIds::Bumper::Top_GroggyStrike, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			20.0f, UPBBossGroggyBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Top_DirectStrike, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			10.0f, UPBBossDamageBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Top_ComboCashout, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			1.0f, UPBComboCashoutBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Rebound_BloodOverdrive, EPBBumperRoleType::Support,
			EPBBumperEffectType::Buff, EPBBumperEffectExecutionPolicy::Immediate,
			25.0f, UPBBloodOverdriveBumperEffect::StaticClass(), TEXT("HP")},
		{PBBumperAssetIds::Bumper::Side_RepairPickup, EPBBumperRoleType::Support,
			EPBBumperEffectType::Instant, EPBBumperEffectExecutionPolicy::Immediate,
			5.0f, UPBDirectRewardBumperEffect::StaticClass(), TEXT("회복")},
		{PBBumperAssetIds::Bumper::Top_VulnerabilityShell, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			20.0f, UPBBossVulnerabilityBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Side_CounterShield, EPBBumperRoleType::Support,
			EPBBumperEffectType::Buff, EPBBumperEffectExecutionPolicy::Immediate,
			1.0f, UPBCounterShieldBumperEffect::StaticClass(), TEXT("보호막")},
		{PBBumperAssetIds::Bumper::Top_ComboArc, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Buff, EPBBumperEffectExecutionPolicy::Immediate,
			2.0f, UPBComboArcBumperEffect::StaticClass(), TEXT("콤보")},
		{PBBumperAssetIds::Bumper::Gate_ReactiveRepair, EPBBumperRoleType::Zone,
			EPBBumperEffectType::Area, EPBBumperEffectExecutionPolicy::QueueIfBusy,
			1.0f, UPBGateSupportFieldBumperEffect::StaticClass(), TEXT("피격")},
		{PBBumperAssetIds::Bumper::Gate_ManaReactor, EPBBumperRoleType::Zone,
			EPBBumperEffectType::Area, EPBBumperEffectExecutionPolicy::QueueIfBusy,
			25.0f, UPBGateSupportFieldBumperEffect::StaticClass(), TEXT("마나")},
		{PBBumperAssetIds::Bumper::Gate_LaunchCharge, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::QueueIfBusy,
			5.0f, UPBTurretSummonBumperEffect::StaticClass(), TEXT("포탑")}
	};

	for (const FDeliveryExpectation& Expectation : DeliveryExpectations)
	{
		const FString Context = Expectation.BumperId.ToString();
		const FPBBumperTableRow* BumperRow = BumperTable->FindRow<FPBBumperTableRow>(
			Expectation.BumperId,
			Context,
			false);
		if (!TestNotNull(*FString::Printf(TEXT("Delivery Bumper row resolves: %s"), *Context), BumperRow))
		{
			continue;
		}

		TestEqual(
			*FString::Printf(TEXT("Delivery role matches: %s"), *Context),
			static_cast<uint8>(BumperRow->RoleType),
			static_cast<uint8>(Expectation.RoleType));
		TestEqual(
			*FString::Printf(TEXT("Delivery type matches: %s"), *Context),
			static_cast<uint8>(BumperRow->EffectType),
			static_cast<uint8>(Expectation.EffectType));

		const FPBBumperEffectRow* EffectRow = EffectTable->FindRow<FPBBumperEffectRow>(
			BumperRow->EffectID,
			Context,
			false);
		if (TestNotNull(*FString::Printf(TEXT("Delivery Effect row resolves: %s"), *Context), EffectRow))
		{
			TestEqual(
				*FString::Printf(TEXT("Delivery policy matches: %s"), *Context),
				static_cast<uint8>(EffectRow->ExecutionPolicy),
				static_cast<uint8>(Expectation.ExecutionPolicy));
			TestTrue(
				*FString::Printf(TEXT("Delivery Power matches: %s"), *Context),
				FMath::IsNearlyEqual(EffectRow->Power, Expectation.Power));
			TestTrue(
				*FString::Printf(TEXT("Delivery description communicates its behavior: %s"), *Context),
				EffectRow->Description.ToString().Contains(Expectation.DescriptionKeyword));
		}

		UPBBumperDataAsset* DataAsset = BumperRow->BumperDataAsset.LoadSynchronous();
		UClass* EffectClass = IsValid(DataAsset)
			? DataAsset->EffectClass.LoadSynchronous()
			: nullptr;
		TestTrue(
			*FString::Printf(TEXT("Delivery Effect class matches responsibility: %s"), *Context),
			IsValid(EffectClass) && EffectClass->IsChildOf(Expectation.EffectBaseClass));

		if (Expectation.EffectBaseClass == UPBTurretSummonBumperEffect::StaticClass()
			&& IsValid(EffectClass))
		{
			const UPBSummonBumperEffect* SummonEffectDefault =
				Cast<UPBSummonBumperEffect>(EffectClass->GetDefaultObject());
			if (TestNotNull(
				*FString::Printf(TEXT("Turret Effect CDO resolves: %s"), *Context),
				SummonEffectDefault))
			{
				TestEqual(
					*FString::Printf(TEXT("Turret Effect uses Turret Anchors: %s"), *Context),
					static_cast<uint8>(SummonEffectDefault->GetSpawnAnchorType()),
					static_cast<uint8>(EPBBumperSummonAnchorType::Turret));
			}
		}
	}

	for (const TPair<FName, uint8*>& TriggerPair : TriggerTable->GetRowMap())
	{
		TestTrue(
			*FString::Printf(TEXT("Trigger row is referenced by a Bumper: %s"), *TriggerPair.Key.ToString()),
			ReferencedTriggerIds.Contains(TriggerPair.Key));
	}
	for (const TPair<FName, uint8*>& EffectPair : EffectTable->GetRowMap())
	{
		TestTrue(
			*FString::Printf(TEXT("Effect row is referenced by a Bumper: %s"), *EffectPair.Key.ToString()),
			ReferencedEffectIds.Contains(EffectPair.Key));
		const FPBBumperEffectRow* EffectRow =
			reinterpret_cast<const FPBBumperEffectRow*>(EffectPair.Value);
		const FEffectBalanceExpectation* ExpectedBalance = ExpectedEffectBalance.Find(EffectPair.Key);
		if (TestNotNull(
			*FString::Printf(TEXT("Balance Effect contract is registered: %s"), *EffectPair.Key.ToString()),
			ExpectedBalance)
			&& TestNotNull(
				*FString::Printf(TEXT("Balance Effect row exists: %s"), *EffectPair.Key.ToString()),
				EffectRow))
		{
			TestTrue(
				*FString::Printf(TEXT("Balance Effect Power matches: %s"), *EffectPair.Key.ToString()),
				FMath::IsNearlyEqual(EffectRow->Power, ExpectedBalance->Power));
			TestTrue(
				*FString::Printf(TEXT("Balance Effect SecondaryPower matches: %s"), *EffectPair.Key.ToString()),
				FMath::IsNearlyEqual(EffectRow->SecondaryPower, ExpectedBalance->SecondaryPower));
			TestTrue(
				*FString::Printf(TEXT("Balance Effect Duration matches: %s"), *EffectPair.Key.ToString()),
				FMath::IsNearlyEqual(EffectRow->Duration, ExpectedBalance->Duration));
			TestEqual(
				*FString::Printf(TEXT("Balance Effect Count matches: %s"), *EffectPair.Key.ToString()),
				EffectRow->Count,
				ExpectedBalance->Count);
			TestEqual(
				*FString::Printf(TEXT("Balance Effect SharedEffectId matches: %s"), *EffectPair.Key.ToString()),
				EffectRow->SharedEffectId,
				ExpectedBalance->SharedEffectId);

			if (!EffectRow->SharedEffectId.IsNone())
			{
				TestNotNull(
					*FString::Printf(TEXT("Shared Effect row resolves: %s"), *EffectPair.Key.ToString()),
					GameplayEffectTable->FindRow<FPBGameplayEffectRow>(
						EffectRow->SharedEffectId,
						EffectPair.Key.ToString(),
						false));
			}
		}
	}

	struct FSharedEffectExpectation
	{
		FName EffectType = NAME_None;
		FName TargetType = NAME_None;
		FName TargetFilter = NAME_None;
	};

	const TMap<FName, FSharedEffectExpectation> ExpectedSharedEffects =
	{
		{TEXT("E_Bumper_Rebound_KineticShell"), {TEXT("VelocityScaledDamage"), TEXT("Battle"), TEXT("All")}},
		{TEXT("E_Bumper_Rebound_BloodOverdrive"), {TEXT("ResourceCostStatBuff"), TEXT("Ball"), TEXT("All")}},
		{TEXT("E_Bumper_Top_ComboArc"), {TEXT("ComboExtraDamage"), TEXT("Battle"), TEXT("All")}},
		{TEXT("E_Bumper_Gate_ReactiveRepair"), {TEXT("PostDamageHeal"), TEXT("Ball"), TEXT("All")}},
		{TEXT("E_Bumper_Gate_ManaReactor"), {TEXT("StatBuff"), TEXT("Ball"), TEXT("All")}}
	};

	TMap<FName, TMap<FName, FString>> ExpectedSharedParameters;
	ExpectedSharedParameters.Add(TEXT("E_Bumper_Rebound_KineticShell"),
		{{TEXT("BaseDamage"), TEXT("3")}, {TEXT("SpeedPerBonus"), TEXT("1000")},
			{TEXT("MaxDamage"), TEXT("8")}});
	ExpectedSharedParameters.Add(TEXT("E_Bumper_Rebound_BloodOverdrive"),
		{{TEXT("ResourceName"), TEXT("Health")}, {TEXT("ResourceCost"), TEXT("1")},
			{TEXT("StatName"), TEXT("Attack")}, {TEXT("ModifyType"), TEXT("PercentAdd")},
			{TEXT("Value"), TEXT("25")}, {TEXT("Duration"), TEXT("5")},
			{TEXT("MinRemainingResource"), TEXT("1")}});
	ExpectedSharedParameters.Add(TEXT("E_Bumper_Top_ComboArc"),
		{{TEXT("ComboInterval"), TEXT("10")}, {TEXT("DamageAmount"), TEXT("2")},
			{TEXT("Duration"), TEXT("10")}});
	ExpectedSharedParameters.Add(TEXT("E_Bumper_Gate_ReactiveRepair"),
		{{TEXT("ResourceName"), TEXT("Health")}, {TEXT("Count"), TEXT("2")},
			{TEXT("Value"), TEXT("1")}, {TEXT("Duration"), TEXT("10")}});
	ExpectedSharedParameters.Add(TEXT("E_Bumper_Gate_ManaReactor"),
		{{TEXT("StatName"), TEXT("ManaRecovery")}, {TEXT("ModifyType"), TEXT("PercentAdd")},
			{TEXT("Value"), TEXT("25")}, {TEXT("Duration"), TEXT("8")}});

	TMap<FName, TMap<FName, FString>> ActualSharedParameters;
	for (const TPair<FName, uint8*>& ParamPair : GameplayEffectParamTable->GetRowMap())
	{
		const FPBGameplayEffectParamRow* ParamRow =
			reinterpret_cast<const FPBGameplayEffectParamRow*>(ParamPair.Value);
		if (!ParamRow || !ExpectedSharedEffects.Contains(ParamRow->EffectId))
		{
			continue;
		}

		TMap<FName, FString>& EffectParameters = ActualSharedParameters.FindOrAdd(ParamRow->EffectId);
		TestFalse(
			*FString::Printf(TEXT("Shared Effect parameter is unique: %s.%s"),
				*ParamRow->EffectId.ToString(),
				*ParamRow->ParamKey.ToString()),
			EffectParameters.Contains(ParamRow->ParamKey));
		EffectParameters.Add(ParamRow->ParamKey, ParamRow->ParamValue.TrimStartAndEnd());
	}

	for (const TPair<FName, FSharedEffectExpectation>& SharedEffectExpectation : ExpectedSharedEffects)
	{
		const FString Context = SharedEffectExpectation.Key.ToString();
		const FPBGameplayEffectRow* SharedEffectRow = GameplayEffectTable->FindRow<FPBGameplayEffectRow>(
			SharedEffectExpectation.Key,
			Context,
			false);
		if (TestNotNull(*FString::Printf(TEXT("Approved Shared Effect row exists: %s"), *Context),
			SharedEffectRow))
		{
			TestEqual(
				*FString::Printf(TEXT("Approved Shared Effect type matches: %s"), *Context),
				SharedEffectRow->EffectType,
				SharedEffectExpectation.Value.EffectType);
			TestEqual(
				*FString::Printf(TEXT("Approved Shared Effect target type matches: %s"), *Context),
				SharedEffectRow->TargetType,
				SharedEffectExpectation.Value.TargetType);
			TestEqual(
				*FString::Printf(TEXT("Approved Shared Effect target filter matches: %s"), *Context),
				SharedEffectRow->TargetFilter,
				SharedEffectExpectation.Value.TargetFilter);
		}

		const TMap<FName, FString>* ExpectedParameters =
			ExpectedSharedParameters.Find(SharedEffectExpectation.Key);
		const TMap<FName, FString>* ActualParameters =
			ActualSharedParameters.Find(SharedEffectExpectation.Key);
		if (!TestNotNull(*FString::Printf(TEXT("Approved Shared Effect parameters exist: %s"), *Context),
			ExpectedParameters)
			|| !TestNotNull(*FString::Printf(TEXT("Parsed Shared Effect parameters exist: %s"), *Context),
				ActualParameters))
		{
			continue;
		}

		TestEqual(
			*FString::Printf(TEXT("Approved Shared Effect parameter count matches: %s"), *Context),
			ActualParameters->Num(),
			ExpectedParameters->Num());
		for (const TPair<FName, FString>& ExpectedParameter : *ExpectedParameters)
		{
			const FString* ActualValue = ActualParameters->Find(ExpectedParameter.Key);
			if (TestNotNull(
				*FString::Printf(TEXT("Approved Shared Effect parameter exists: %s.%s"),
					*Context,
					*ExpectedParameter.Key.ToString()),
				ActualValue))
			{
				TestEqual(
					*FString::Printf(TEXT("Approved Shared Effect parameter value matches: %s.%s"),
						*Context,
						*ExpectedParameter.Key.ToString()),
					*ActualValue,
					ExpectedParameter.Value);
			}
		}
	}

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
