// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/Actor/Bumper/Trigger/PBBumperTriggerActorBase.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"

using namespace PBSheetParserUtils;

UPBBumperTableParser::UPBBumperTableParser()
{
	DataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Bumper");
	DataAssetPreset.NameFormat = TEXT("DA_Bumper_{0}");
	IconPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Art/Texture/Icon");
	IconPreset.NameFormat = TEXT("T_{0}");
	TriggerClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Trigger");
	TriggerClassPreset.NameFormat = TEXT("BP_{0}");
	EffectClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Effect");
	EffectClassPreset.NameFormat = TEXT("BP_{0}");
	ActivationVfxPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Effect/VFX");
	ActivationVfxPreset.NameFormat = TEXT("{0}");
}

const TCHAR* UPBBumperTableParser::GetParserName() const
{
	return TEXT("Bumper");
}

UScriptStruct* UPBBumperTableParser::GetRowStruct() const
{
	return FPBBumperTableRow::StaticStruct();
}

bool UPBBumperTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBumperTableRow NewRow;
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));
	NewRow.BumperType = ParseEnumValue(RowData.FindRef(TEXT("BumperType")), EPBBumperType::Rebound);
	NewRow.TriggerID = FName(*TrimCell(RowData.FindRef(TEXT("TriggerID"))));
	NewRow.RoleType = ParseEnumValue(RowData.FindRef(TEXT("RoleType")), EPBBumperRoleType::Attack);
	NewRow.EffectType = ParseEnumValue(RowData.FindRef(TEXT("EffectType")), EPBBumperEffectType::Instant);
	NewRow.RequiredTriggerCount = FMath::Max(
		ParseIntValue(RowData.FindRef(TEXT("RequireTriggerCount")), 1),
		1);
	NewRow.EffectID = FName(*TrimCell(RowData.FindRef(TEXT("EffectID"))));
	if (UPBBumperDataAsset* BumperDataAsset = SetupBumperDataAsset(
		RowName,
		NewRow.TriggerID,
		NewRow.EffectID))
	{
		NewRow.BumperDataAsset = TSoftObjectPtr<UPBBumperDataAsset>(BumperDataAsset);
	}

	TargetTable->AddRow(RowName, NewRow);
	return true;
}

UPBBumperDataAsset* UPBBumperTableParser::SetupBumperDataAsset(
	const FName RowName,
	const FName TriggerId,
	const FName EffectId) const
{
	UPBBumperDataAsset* BumperDataAsset =
		GetOrCreateDataAsset<UPBBumperDataAsset>(DataAssetPreset, RowName, TEXT("Bumper"));
	if (!IsValid(BumperDataAsset))
	{
		return nullptr;
	}

	if (IconPreset.IsValid())
	{
		BumperDataAsset->Icon = FindObject<UTexture2D>(IconPreset, RowName);
	}
	BumperDataAsset->TriggerClass =
		FindBlueprintClass<APBBumperTriggerActorBase>(TriggerClassPreset, TriggerId);
	BumperDataAsset->EffectClass =
		FindBlueprintClass<UPBBumperEffectBase>(EffectClassPreset, EffectId);
	BumperDataAsset->ActivationVfx = ResolveActivationVfx(EffectId);

	(void)BumperDataAsset->SetRowNameForImport(RowName);
	(void)BumperDataAsset->MarkPackageDirty();
	return BumperDataAsset;
}

TSoftObjectPtr<UNiagaraSystem> UPBBumperTableParser::ResolveActivationVfx(const FName EffectId) const
{
	if (EffectId.IsNone())
	{
		return nullptr;
	}

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* EffectTable = Settings ? Settings->BumperEffectTable.LoadSynchronous() : nullptr;
	if (!IsValid(EffectTable))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Sheet][Bumper] Activation VFX lookup skipped because Effect table is unavailable. EffectId=%s"),
			*EffectId.ToString());
		return nullptr;
	}

	const FPBBumperEffectRow* EffectRow = EffectTable->FindRow<FPBBumperEffectRow>(
		EffectId,
		TEXT("ResolveBumperActivationVfx"),
		false);
	if (!EffectRow)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Sheet][Bumper] Activation VFX lookup skipped because Effect row is missing. EffectId=%s"),
			*EffectId.ToString());
		return nullptr;
	}

	return EffectRow->ActivationVfxId.IsNone()
		? nullptr
		: FindObject<UNiagaraSystem>(ActivationVfxPreset, EffectRow->ActivationVfxId);
}
