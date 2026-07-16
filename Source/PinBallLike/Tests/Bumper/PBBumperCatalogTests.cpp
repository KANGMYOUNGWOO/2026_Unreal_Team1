#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossDamageBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossGroggyBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Effect/PBComboCashoutBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBDirectRewardBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBTurretSummonBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
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
	UDataTable* TriggerTable = Settings ? Settings->BumperTriggerTable.LoadSynchronous() : nullptr;
	UDataTable* EffectTable = Settings ? Settings->BumperEffectTable.LoadSynchronous() : nullptr;
	if (!TestNotNull(TEXT("Bumper DataTable is configured"), BumperTable)
		|| !TestNotNull(TEXT("Bumper Trigger DataTable is configured"), TriggerTable)
		|| !TestNotNull(TEXT("Bumper Effect DataTable is configured"), EffectTable))
	{
		return false;
	}

	TestEqual(TEXT("Bumper catalog contains exactly 20 rows"), BumperTable->GetRowMap().Num(), 20);

	TMap<EPBBumperType, int32> TypeCounts;
	TSet<FName> ReferencedTriggerIds;
	TSet<FName> ReferencedEffectIds;
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

			if (EffectRow && !EffectRow->ActivationVfxId.IsNone())
			{
				TestNotNull(
					*FString::Printf(TEXT("Configured ActivationVfx resolves: %s"), *RowPair.Key.ToString()),
					BumperDataAsset->ActivationVfx.LoadSynchronous());
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
		{PBBumperAssetIds::Bumper::Rebound_CounterShell, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			10.0f, UPBBossDamageBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Rebound_GroggyHammer, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			12.0f, UPBBossGroggyBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Top_GroggyStrike, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			20.0f, UPBBossGroggyBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Top_DirectStrike, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			20.0f, UPBBossDamageBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Top_ComboCashout, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::Immediate,
			2.0f, UPBComboCashoutBumperEffect::StaticClass(), TEXT("명중")},
		{PBBumperAssetIds::Bumper::Rebound_ManaOrb, EPBBumperRoleType::Support,
			EPBBumperEffectType::Instant, EPBBumperEffectExecutionPolicy::Immediate,
			15.0f, UPBDirectRewardBumperEffect::StaticClass(), TEXT("즉시")},
		{PBBumperAssetIds::Bumper::Side_RepairPickup, EPBBumperRoleType::Support,
			EPBBumperEffectType::Instant, EPBBumperEffectExecutionPolicy::Immediate,
			20.0f, UPBDirectRewardBumperEffect::StaticClass(), TEXT("즉시")},
		{PBBumperAssetIds::Bumper::Top_ComboPickup, EPBBumperRoleType::Support,
			EPBBumperEffectType::Instant, EPBBumperEffectExecutionPolicy::Immediate,
			10.0f, UPBDirectRewardBumperEffect::StaticClass(), TEXT("즉시")},
		{PBBumperAssetIds::Bumper::Side_LaunchCharge, EPBBumperRoleType::Attack,
			EPBBumperEffectType::Summon, EPBBumperEffectExecutionPolicy::QueueIfBusy,
			5.0f, UPBTurretSummonBumperEffect::StaticClass(), TEXT("포탑")},
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
				*FString::Printf(TEXT("Delivery description communicates its timing: %s"), *Context),
				EffectRow->Description.ToString().Contains(Expectation.DescriptionKeyword));
		}

		UPBBumperDataAsset* DataAsset = BumperRow->BumperDataAsset.LoadSynchronous();
		UClass* EffectClass = IsValid(DataAsset)
			? DataAsset->EffectClass.LoadSynchronous()
			: nullptr;
		TestTrue(
			*FString::Printf(TEXT("Delivery Effect class matches responsibility: %s"), *Context),
			IsValid(EffectClass) && EffectClass->IsChildOf(Expectation.EffectBaseClass));
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
