#include "Bumper/PBBumperDataSyncCommandlet.h"

#include "AssetToolsModule.h"
#include "BlueprintEditorLibrary.h"
#include "Bumper/PBBumperTableParser.h"
#include "Bumper/PBBumperTriggerTableParser.h"
#include "Bumper/PBBumperEffectTableParser.h"
#include "Effect/PBGameplayEffectParamTableParser.h"
#include "Effect/PBGameplayEffectTableParser.h"
#include "Engine/Blueprint.h"
#include "Engine/DataTable.h"
#include "Factories/DataAssetFactory.h"
#include "Factories/DataTableFactory.h"
#include "GoogleSheetConfig.h"
#include "HttpManager.h"
#include "HttpModule.h"
#include "IAssetTools.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBloodOverdriveBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossDamageBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBossVulnerabilityBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBComboArcBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBCounterShieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBGateSupportFieldBumperEffect.h"
#include "PinBallLike/Actor/Bumper/Effect/PBKineticShellBumperEffect.h"
#include "PinBallLike/Table/Bumper/PBBumperAssetIds.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectRow.h"
#include "UObject/SavePackage.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr double SheetFetchTimeoutSeconds = 60.0;
	const TCHAR* SharedEffectSheetId = TEXT("1rvssGjKqKdvHaZ4fKKAI9zJZm5zMuBMYTsCV1l8Mld4");

	struct FApprovedAssetRename
	{
		const TCHAR* SourcePackage;
		const TCHAR* TargetPackage;
	};

	const TArray<TPair<FString, FString>>& GetLegacyBumperIdPairs()
	{
		static const TArray<TPair<FString, FString>> Pairs =
		{
			{TEXT("Rebound_CounterShell"), TEXT("Rebound_KineticShell")},
			{TEXT("Rebound_ManaOrb"), TEXT("Rebound_BloodOverdrive")},
			{TEXT("Side_LaunchCharge"), TEXT("Side_CounterShield")},
			{TEXT("Top_ComboUp"), TEXT("Top_ComboArc")},
			{TEXT("Top_ComboPickup"), TEXT("Top_VulnerabilityShell")},
			{TEXT("Gate_ManaField"), TEXT("Gate_ReactiveRepair")},
			{TEXT("Gate_StrengthField"), TEXT("Gate_ManaReactor")}
		};
		return Pairs;
	}

	FString MakeObjectPath(const FString& PackagePath)
	{
		return FString::Printf(
			TEXT("%s.%s"),
			*PackagePath,
			*FPackageName::GetLongPackageAssetName(PackagePath));
	}

	UObject* LoadAsset(const FString& PackagePath)
	{
		return StaticLoadObject(
			UObject::StaticClass(),
			nullptr,
			*MakeObjectPath(PackagePath),
			nullptr,
			LOAD_NoWarn);
	}

	FName ReplaceLegacyBumperTokens(const FName Value)
	{
		if (Value.IsNone())
		{
			return NAME_None;
		}

		FString Result = Value.ToString();
		for (const TPair<FString, FString>& Pair : GetLegacyBumperIdPairs())
		{
			Result.ReplaceInline(*Pair.Key, *Pair.Value, ESearchCase::CaseSensitive);
		}
		return FName(*Result);
	}

	bool DuplicateCounterShieldBlueprint(IAssetTools& AssetTools)
	{
		const FString LegacySourcePackage = TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_01");
		const FString CurrentSourcePackage = TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_02");
		const FString TargetPackage = TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_CounterShield_01");
		if (LoadAsset(TargetPackage))
		{
			return true;
		}

		UObject* SourceAsset = LoadAsset(LegacySourcePackage);
		if (!IsValid(SourceAsset))
		{
			SourceAsset = LoadAsset(CurrentSourcePackage);
		}
		if (!IsValid(SourceAsset))
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperSync] CounterShield source is missing. Legacy=%s Current=%s"),
				*LegacySourcePackage,
				*CurrentSourcePackage);
			return false;
		}

		UObject* DuplicatedAsset = AssetTools.DuplicateAsset(
			TEXT("BP_Effect_CounterShield_01"),
			TEXT("/Game/Blueprints/Bumper/Effect"),
			SourceAsset);
		if (!IsValid(DuplicatedAsset))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] Failed to duplicate CounterShield effect Blueprint."));
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("[BumperSync] Created %s"), *TargetPackage);
		return true;
	}

	bool RenameApprovedAssets(IAssetTools& AssetTools)
	{
		static const TArray<FApprovedAssetRename> Renames =
		{
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BossDamage_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_KineticShell_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaPickup_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BloodOverdrive_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboUp_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboArc_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboPickup_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_VulnerabilityShell_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaField_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ReactiveRepair_01")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_StrengthField_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaReactor_01")},

			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_GroggyHammer_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_GroggyHammer_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_Shield_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_Shield_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_PartyMana_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_PartyMana_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RepairPickup_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RepairPickup_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_StrengthCharge_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_StrengthCharge_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BossGroggy_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BossGroggy_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_DirectStrike_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_DirectStrike_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboCashout_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboCashout_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RecoveryField_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_RecoveryField_02")},
			{TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_01"), TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_LaunchCharge_02")},

			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_CounterShell"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_KineticShell")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_ManaOrb"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Rebound_Trigger_BloodOverdrive")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Side_Trigger_LaunchCharge"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Side_Trigger_CounterShield")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_ComboUp"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_ComboArc")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_ComboPickup"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Top_Trigger_VulnerabilityShell")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_ManaField"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_ReactiveRepair")},
			{TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_StrengthField"), TEXT("/Game/Blueprints/Bumper/Trigger/BP_Gate_Trigger_ManaReactor")},

			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_CounterShell"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_KineticShell")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_ManaOrb"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Rebound_BloodOverdrive")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Side_LaunchCharge"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Side_CounterShield")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_ComboUp"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_ComboArc")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_ComboPickup"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Top_VulnerabilityShell")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_ManaField"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_ReactiveRepair")},
			{TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_StrengthField"), TEXT("/Game/Data/DataAssets/Bumper/DA_Bumper_Gate_ManaReactor")},

			{TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Rebound_CounterShell"), TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Rebound_KineticShell")},
			{TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Rebound_ManaOrb"), TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Rebound_BloodOverdrive")},
			{TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Side_LaunchCharge"), TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Side_CounterShield")},
			{TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Top_ComboUp"), TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Top_ComboArc")},
			{TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Top_ComboPickup"), TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Top_VulnerabilityShell")},
			{TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Gate_ManaField"), TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Gate_ReactiveRepair")},
			{TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Gate_StrengthField"), TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/T_Gate_ManaReactor")}
		};

		TArray<FAssetRenameData> RenameData;
		for (const FApprovedAssetRename& Rename : Renames)
		{
			if (LoadAsset(Rename.TargetPackage))
			{
				continue;
			}

			UObject* SourceAsset = LoadAsset(Rename.SourcePackage);
			if (!IsValid(SourceAsset))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Rename source is missing. Source=%s Target=%s"),
					Rename.SourcePackage,
					Rename.TargetPackage);
				return false;
			}

			RenameData.Emplace(
				SourceAsset,
				FPackageName::GetLongPackagePath(Rename.TargetPackage),
				FPackageName::GetLongPackageAssetName(Rename.TargetPackage));
		}

		if (RenameData.IsEmpty())
		{
			return true;
		}

		if (!AssetTools.RenameAssets(RenameData))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] AssetTools failed to rename approved assets."));
			return false;
		}

		UE_LOG(LogTemp, Display, TEXT("[BumperSync] Renamed %d approved assets."), RenameData.Num());
		return true;
	}

	bool SetProjectileClass(UBlueprint* Blueprint)
	{
		UBlueprint* ProjectileBlueprint = Cast<UBlueprint>(
			LoadAsset(TEXT("/Game/Blueprints/Bumper/Effect/BP_TestBullet")));
		if (!IsValid(Blueprint)
			|| !IsValid(Blueprint->GeneratedClass)
			|| !IsValid(ProjectileBlueprint)
			|| !IsValid(ProjectileBlueprint->GeneratedClass))
		{
			return false;
		}

		UObject* ClassDefaultObject = Blueprint->GeneratedClass->GetDefaultObject();
		FClassProperty* ProjectileProperty = FindFProperty<FClassProperty>(
			Blueprint->GeneratedClass,
			TEXT("ProjectileClass"));
		if (!IsValid(ClassDefaultObject) || !ProjectileProperty)
		{
			return false;
		}

		ClassDefaultObject->Modify();
		ProjectileProperty->SetPropertyValue_InContainer(
			ClassDefaultObject,
			ProjectileBlueprint->GeneratedClass);
		Blueprint->MarkPackageDirty();
		return true;
	}

	bool ReparentEffectBlueprint(
		const TCHAR* PackagePath,
		UClass* NewParentClass,
		const bool bNeedsProjectileClass)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(LoadAsset(PackagePath));
		if (!IsValid(Blueprint) || !IsValid(NewParentClass))
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperSync] Cannot reparent Blueprint. Asset=%s Parent=%s"),
				PackagePath,
				*GetNameSafe(NewParentClass));
			return false;
		}

		if (Blueprint->ParentClass != NewParentClass)
		{
			UBlueprintEditorLibrary::ReparentBlueprint(Blueprint, NewParentClass);
			UBlueprintEditorLibrary::CompileBlueprint(Blueprint);
		}

		if (bNeedsProjectileClass && !SetProjectileClass(Blueprint))
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperSync] ProjectileClass could not be assigned. Asset=%s"),
				PackagePath);
			return false;
		}

		Blueprint->MarkPackageDirty();
		return Blueprint->Status != BS_Error;
	}

	bool ReparentApprovedEffectBlueprints()
	{
		return ReparentEffectBlueprint(
			TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_KineticShell_01"),
			UPBKineticShellBumperEffect::StaticClass(),
			true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_BloodOverdrive_01"),
				UPBBloodOverdriveBumperEffect::StaticClass(),
				false)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_CounterShield_01"),
				UPBCounterShieldBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ComboArc_01"),
				UPBComboArcBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_VulnerabilityShell_01"),
				UPBBossVulnerabilityBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_DirectStrike_02"),
				UPBBossDamageBumperEffect::StaticClass(),
				true)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ReactiveRepair_01"),
				UPBGateSupportFieldBumperEffect::StaticClass(),
				false)
			&& ReparentEffectBlueprint(
				TEXT("/Game/Blueprints/Bumper/Effect/BP_Effect_ManaReactor_01"),
				UPBGateSupportFieldBumperEffect::StaticClass(),
				false);
	}

	UDataTable* EnsureDataTable(
		IAssetTools& AssetTools,
		const FString& PackagePath,
		UScriptStruct* RowStruct)
	{
		if (UDataTable* ExistingTable = Cast<UDataTable>(LoadAsset(PackagePath)))
		{
			if (ExistingTable->GetRowStruct() != RowStruct)
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] DataTable row struct mismatch. Table=%s Expected=%s Actual=%s"),
					*PackagePath,
					*GetNameSafe(RowStruct),
					*GetNameSafe(ExistingTable->GetRowStruct()));
				return nullptr;
			}
			return ExistingTable;
		}

		UDataTableFactory* Factory = NewObject<UDataTableFactory>();
		Factory->Struct = RowStruct;
		UDataTable* NewTable = Cast<UDataTable>(AssetTools.CreateAsset(
			FPackageName::GetLongPackageAssetName(PackagePath),
			FPackageName::GetLongPackagePath(PackagePath),
			UDataTable::StaticClass(),
			Factory));
		if (IsValid(NewTable))
		{
			NewTable->MarkPackageDirty();
		}
		return NewTable;
	}

	template <typename TParser>
	UGoogleSheetConfig* EnsureSheetConfig(
		IAssetTools& AssetTools,
		const FString& PackagePath,
		const FString& PageName,
		const FString& RangeTo,
		UDataTable* TargetTable)
	{
		UGoogleSheetConfig* Config = Cast<UGoogleSheetConfig>(LoadAsset(PackagePath));
		if (!IsValid(Config))
		{
			UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();
			Factory->DataAssetClass = UGoogleSheetConfig::StaticClass();
			Config = Cast<UGoogleSheetConfig>(AssetTools.CreateAsset(
				FPackageName::GetLongPackageAssetName(PackagePath),
				FPackageName::GetLongPackagePath(PackagePath),
				UGoogleSheetConfig::StaticClass(),
				Factory));
		}
		if (!IsValid(Config) || !IsValid(TargetTable))
		{
			return nullptr;
		}

		Config->Modify();
		Config->SheetURL = SharedEffectSheetId;
		Config->PageName = PageName;
		Config->RangeFrom = TEXT("A1");
		Config->RangeTo = RangeTo;
		Config->bAutoSaveOnComplete = false;

		TParser* Parser = Cast<TParser>(Config->DataParser);
		if (!IsValid(Parser))
		{
			Parser = NewObject<TParser>(Config, NAME_None, RF_Transactional);
			Config->DataParser = Parser;
		}
		Parser->SetTargetTable(TargetTable);
		Config->MarkPackageDirty();
		return Config;
	}

	bool FetchAndWait(UGoogleSheetConfig* Config)
	{
		if (!IsValid(Config))
		{
			return false;
		}

		Config->Fetch();
		const double Deadline = FPlatformTime::Seconds() + SheetFetchTimeoutSeconds;
		while (Config->FetchStatus == EFetchStatus::Loading
			&& FPlatformTime::Seconds() < Deadline)
		{
			FHttpModule::Get().GetHttpManager().Tick(0.05f);
			FPlatformProcess::Sleep(0.01f);
		}

		if (Config->FetchStatus != EFetchStatus::Success)
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperSync] Sheet fetch failed. Config=%s Status=%d Message=%s"),
				*GetNameSafe(Config),
				static_cast<int32>(Config->FetchStatus),
				*Config->LastMessage);
			return false;
		}

		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Sheet fetch succeeded. Config=%s Range=%s"),
			*GetNameSafe(Config),
			*Config->GetRangeString());
		return true;
	}

	UGoogleSheetConfig* ConfigureExistingSheetConfig(
		const TCHAR* PackagePath,
		const TCHAR* RangeTo)
	{
		UGoogleSheetConfig* Config = Cast<UGoogleSheetConfig>(LoadAsset(PackagePath));
		if (!IsValid(Config) || !IsValid(Config->DataParser))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] Existing Loader is invalid: %s"), PackagePath);
			return nullptr;
		}

		Config->Modify();
		Config->RangeFrom = TEXT("A1");
		Config->RangeTo = RangeTo;
		Config->bAutoSaveOnComplete = false;
		Config->MarkPackageDirty();
		return Config;
	}

	bool RefreshCollectionBumperMetadata()
	{
		UDataTable* CollectionTable = Cast<UDataTable>(
			LoadAsset(TEXT("/Game/Data/Tables/DT_Collection")));
		const UDataTable* BumperTable = Cast<UDataTable>(
			LoadAsset(TEXT("/Game/Data/Tables/DT_Bumper")));
		if (!IsValid(CollectionTable) || !IsValid(BumperTable))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] DT_Collection or DT_Bumper is unavailable."));
			return false;
		}
		if (CollectionTable->GetRowStruct() != FPBCollectionTableRow::StaticStruct())
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperSync] DT_Collection has an unexpected row struct."));
			return false;
		}

		struct FRowReplacement
		{
			FName OldKey;
			FName NewKey;
			FPBCollectionTableRow Row;
		};

		TArray<FRowReplacement> Replacements;
		for (const FName RowKey : CollectionTable->GetRowNames())
		{
			const FPBCollectionTableRow* ExistingRow = CollectionTable->FindRow<FPBCollectionTableRow>(
				RowKey,
				TEXT("BumperSyncCollection"),
				false);
			if (!ExistingRow || ExistingRow->Category != EPBCollectionCategory::Bumper)
			{
				continue;
			}

			FRowReplacement& Replacement = Replacements.AddDefaulted_GetRef();
			Replacement.OldKey = RowKey;
			Replacement.NewKey = ReplaceLegacyBumperTokens(RowKey);
			Replacement.Row = *ExistingRow;
			Replacement.Row.SourceRowName = PBBumperAssetIds::NormalizeBumperRowId(
				Replacement.Row.SourceRowName);
			Replacement.Row.SourceId = Replacement.Row.SourceRowName;
			Replacement.Row.SourceTableName = TEXT("Bumper");
			Replacement.Row.CollectionId = ReplaceLegacyBumperTokens(Replacement.Row.CollectionId);
			Replacement.Row.IconAssetKey = FName(*FString::Printf(
				TEXT("T_%s"),
				*Replacement.Row.SourceRowName.ToString()));

			const FPBBumperTableRow* BumperRow = BumperTable->FindRow<FPBBumperTableRow>(
				Replacement.Row.SourceRowName,
				TEXT("BumperSyncCollection"),
				false);
			if (!BumperRow)
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Collection row points to a missing Bumper. Collection=%s Bumper=%s"),
					*RowKey.ToString(),
					*Replacement.Row.SourceRowName.ToString());
				return false;
			}

			Replacement.Row.DisplayName = BumperRow->DisplayName;
			Replacement.Row.LockedName = BumperRow->DisplayName;
			Replacement.Row.ShortDescription = BumperRow->Description;
			Replacement.Row.DetailDescription = BumperRow->Description;
			Replacement.Row.UnlockConditionText = FText::GetEmpty();
			Replacement.Row.bHiddenUntilDiscovered = false;
		}

		if (Replacements.IsEmpty())
		{
			return true;
		}

		CollectionTable->Modify();
		for (const FRowReplacement& Replacement : Replacements)
		{
			if (Replacement.NewKey != Replacement.OldKey)
			{
				CollectionTable->RemoveRow(Replacement.OldKey);
			}
			CollectionTable->AddRow(Replacement.NewKey, Replacement.Row);
		}
		CollectionTable->MarkPackageDirty();
		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Refreshed %d Bumper metadata rows in DT_Collection."),
			Replacements.Num());
		return true;
	}

	bool IsApprovedDirtyPackage(
		const FString& PackageName,
		const bool bIncludeLegacyMigrationAssets)
	{
		static const TSet<FString> ExactPackages =
		{
			TEXT("/Game/Data/Tables/Effect/DT_GameplayEffect"),
			TEXT("/Game/Data/Tables/Effect/DT_GameplayEffectParam"),
			TEXT("/Game/Data/Tables/DT_Trigger"),
			TEXT("/Game/Data/Tables/DT_Effect"),
			TEXT("/Game/Data/Tables/DT_Bumper"),
			TEXT("/Game/Data/Tables/DT_Collection"),
			TEXT("/Game/Data/Loaders/GSC_GameplayEffect"),
			TEXT("/Game/Data/Loaders/GSC_GameplayEffectParam"),
			TEXT("/Game/Data/Loaders/GSC_Trigger"),
			TEXT("/Game/Data/Loaders/GSC_Effect"),
			TEXT("/Game/Data/Loaders/GSC_Bumper")
		};
		if (ExactPackages.Contains(PackageName)
			|| PackageName.StartsWith(TEXT("/Game/Data/DataAssets/Bumper/")))
		{
			return true;
		}

		return bIncludeLegacyMigrationAssets
			&& (PackageName.StartsWith(TEXT("/Game/Blueprints/Bumper/Effect/"))
				|| PackageName.StartsWith(TEXT("/Game/Blueprints/Bumper/Trigger/"))
				|| PackageName.StartsWith(TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon/")));
	}

	bool SaveApprovedDirtyPackages(const bool bIncludeLegacyMigrationAssets)
	{
		TArray<UPackage*> PackagesToSave;
		for (TObjectIterator<UPackage> It; It; ++It)
		{
			UPackage* Package = *It;
			if (!IsValid(Package) || !Package->IsDirty())
			{
				continue;
			}

			const FString PackageName = Package->GetName();
			if (IsApprovedDirtyPackage(PackageName, bIncludeLegacyMigrationAssets))
			{
				PackagesToSave.Add(Package);
			}
		}

		PackagesToSave.Sort([](const UPackage& Left, const UPackage& Right)
		{
			return Left.GetName() < Right.GetName();
		});

		for (UPackage* Package : PackagesToSave)
		{
			const FString PackageName = Package->GetName();
			const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
			UObject* Asset = FindObject<UObject>(Package, *AssetName);
			if (!IsValid(Asset))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[BumperSync] Package has no top-level asset to save: %s"),
					*PackageName);
				return false;
			}

			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;
			const FString Filename = FPackageName::LongPackageNameToFilename(
				PackageName,
				FPackageName::GetAssetPackageExtension());
			if (!UPackage::SavePackage(Package, Asset, *Filename, SaveArgs))
			{
				UE_LOG(LogTemp, Error, TEXT("[BumperSync] Failed to save package: %s"), *PackageName);
				return false;
			}
		}

		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Saved %d approved dirty packages."),
			PackagesToSave.Num());
		return true;
	}
}

UPBBumperDataSyncCommandlet::UPBBumperDataSyncCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UPBBumperDataSyncCommandlet::Main(const FString& Params)
{
	const bool bMigrateLegacyAssets = FParse::Param(*Params, TEXT("MigrateLegacyAssets"));
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(
		TEXT("AssetTools")).Get();

	if (bMigrateLegacyAssets
		&& (!DuplicateCounterShieldBlueprint(AssetTools)
			|| !RenameApprovedAssets(AssetTools)
			|| !ReparentApprovedEffectBlueprints()))
	{
		return 1;
	}
	if (!bMigrateLegacyAssets)
	{
		UE_LOG(LogTemp, Display,
			TEXT("[BumperSync] Legacy Blueprint rename/reparent migration skipped. Use -MigrateLegacyAssets only for the approved one-time migration."));
	}

	UDataTable* GameplayEffectTable = EnsureDataTable(
		AssetTools,
		TEXT("/Game/Data/Tables/Effect/DT_GameplayEffect"),
		FPBGameplayEffectRow::StaticStruct());
	UDataTable* GameplayEffectParamTable = EnsureDataTable(
		AssetTools,
		TEXT("/Game/Data/Tables/Effect/DT_GameplayEffectParam"),
		FPBGameplayEffectParamRow::StaticStruct());
	if (!IsValid(GameplayEffectTable) || !IsValid(GameplayEffectParamTable))
	{
		return 1;
	}

	UGoogleSheetConfig* GameplayEffectConfig = EnsureSheetConfig<UPBGameplayEffectTableParser>(
		AssetTools,
		TEXT("/Game/Data/Loaders/GSC_GameplayEffect"),
		TEXT("Effect"),
		TEXT("G1000"),
		GameplayEffectTable);
	UGoogleSheetConfig* GameplayEffectParamConfig = EnsureSheetConfig<UPBGameplayEffectParamTableParser>(
		AssetTools,
		TEXT("/Game/Data/Loaders/GSC_GameplayEffectParam"),
		TEXT("EffectParam"),
		TEXT("D2000"),
		GameplayEffectParamTable);
	if (!IsValid(GameplayEffectConfig) || !IsValid(GameplayEffectParamConfig))
	{
		return 1;
	}

	UGoogleSheetConfig* TriggerConfig = ConfigureExistingSheetConfig(
		TEXT("/Game/Data/Loaders/GSC_Trigger"),
		TEXT("F1000"));
	UGoogleSheetConfig* EffectConfig = ConfigureExistingSheetConfig(
		TEXT("/Game/Data/Loaders/GSC_Effect"),
		TEXT("O1000"));
	UGoogleSheetConfig* BumperConfig = ConfigureExistingSheetConfig(
		TEXT("/Game/Data/Loaders/GSC_Bumper"),
		TEXT("K1000"));
	if (!IsValid(TriggerConfig) || !IsValid(EffectConfig) || !IsValid(BumperConfig))
	{
		return 1;
	}

	if (!FetchAndWait(GameplayEffectConfig)
		|| !FetchAndWait(GameplayEffectParamConfig)
		|| !FetchAndWait(TriggerConfig)
		|| !FetchAndWait(EffectConfig)
		|| !FetchAndWait(BumperConfig)
		|| !RefreshCollectionBumperMetadata())
	{
		return 1;
	}

	if (!SaveApprovedDirtyPackages(bMigrateLegacyAssets))
	{
		return 1;
	}

	UE_LOG(LogTemp, Display, TEXT("[BumperSync] Approved Bumper data synchronization completed."));
	return 0;
}
