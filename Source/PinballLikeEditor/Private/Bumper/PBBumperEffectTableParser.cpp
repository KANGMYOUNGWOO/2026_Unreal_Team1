// Fill out your copyright notice in the Description page of Project Settings.


#include "Bumper/PBBumperEffectTableParser.h"

#include "PBSheetParserUtils.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperEffectBase.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Table/Bumper/DataAsset/PBBumperDataAsset.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"

using namespace PBSheetParserUtils;

UPBBumperEffectTableParser::UPBBumperEffectTableParser()
{
	EffectClassPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Effect");
	EffectClassPreset.NameFormat = TEXT("BP_{0}");
	BumperDataAssetPreset.FolderPath.Path = TEXT("/Game/Data/DataAssets/Bumper");
	BumperDataAssetPreset.NameFormat = TEXT("DA_Bumper_{0}");
	ActivationVfxPreset.FolderPath.Path = TEXT("/Game/Blueprints/Bumper/Effect/VFX");
	ActivationVfxPreset.NameFormat = TEXT("{0}");
}

const TCHAR* UPBBumperEffectTableParser::GetParserName() const
{
	return TEXT("Effect");
}

UScriptStruct* UPBBumperEffectTableParser::GetRowStruct() const
{
	return FPBBumperEffectRow::StaticStruct();
}

bool UPBBumperEffectTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBBumperEffectRow NewRow;
	NewRow.EffectType = ParseEnumValue(RowData.FindRef(TEXT("EffectType")), EPBBumperEffectType::Instant);
	NewRow.ExecutionPolicy = ParseEnumValue(
		RowData.FindRef(TEXT("ExecutionPolicy")),
		EPBBumperEffectExecutionPolicy::Immediate);
	NewRow.Power = ParseFloatValue(RowData.FindRef(TEXT("Power")), 0.0f);
	NewRow.SecondaryPower = ParseFloatValue(RowData.FindRef(TEXT("SecondaryPower")), 0.0f);
	NewRow.Duration = FMath::Max(ParseFloatValue(RowData.FindRef(TEXT("Duration")), 0.0f), 0.0f);
	NewRow.Count = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("Count")), 0), 0);
	NewRow.SharedEffectId = ParseNameValue(RowData.FindRef(TEXT("SharedEffectId")));
	NewRow.ActivationVfxId = ParseNameValue(RowData.FindRef(TEXT("ActivationVfxId")));
	NewRow.DeliveryVfxId = ParseNameValue(RowData.FindRef(TEXT("DeliveryVfxId")));
	NewRow.ImpactVfxId = ParseNameValue(RowData.FindRef(TEXT("ImpactVfxId")));
	NewRow.StatusVfxId = ParseNameValue(RowData.FindRef(TEXT("StatusVfxId")));
	NewRow.Description = FText::FromString(RowData.FindRef(TEXT("Description")));

	TargetTable->AddRow(RowName, NewRow);
	UpdateLinkedBumperDataAssets(RowName, NewRow);
	return true;
}

void UPBBumperEffectTableParser::UpdateLinkedBumperDataAssets(
	const FName EffectId,
	const FPBBumperEffectRow& EffectRow) const
{
	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	UDataTable* BumperTable = Settings ? Settings->BumperTable.LoadSynchronous() : nullptr;
	if (!IsValid(BumperTable) || EffectId.IsNone())
	{
		return;
	}

	const TSoftClassPtr<UPBBumperEffectBase> EffectClass =
		FindBlueprintClass<UPBBumperEffectBase>(EffectClassPreset, EffectId);
	const auto ResolveVfx = [this](const FName VfxId)
	{
		return VfxId.IsNone()
			? TSoftObjectPtr<UNiagaraSystem>()
			: FindObject<UNiagaraSystem>(ActivationVfxPreset, VfxId);
	};
	const TSoftObjectPtr<UNiagaraSystem> ActivationVfx = ResolveVfx(EffectRow.ActivationVfxId);
	const TSoftObjectPtr<UNiagaraSystem> DeliveryVfx = ResolveVfx(EffectRow.DeliveryVfxId);
	const TSoftObjectPtr<UNiagaraSystem> ImpactVfx = ResolveVfx(EffectRow.ImpactVfxId);
	const TSoftObjectPtr<UNiagaraSystem> StatusVfx = ResolveVfx(EffectRow.StatusVfxId);
	for (const TPair<FName, uint8*>& RowPair : BumperTable->GetRowMap())
	{
		const FPBBumperTableRow* BumperRow = reinterpret_cast<FPBBumperTableRow*>(RowPair.Value);
		if (!BumperRow || BumperRow->EffectID != EffectId)
		{
			continue;
		}

		UPBBumperDataAsset* BumperDataAsset =
			FindObject<UPBBumperDataAsset>(BumperDataAssetPreset, RowPair.Key).LoadSynchronous();
		if (IsValid(BumperDataAsset))
		{
			BumperDataAsset->EffectClass = EffectClass;
			BumperDataAsset->ActivationVfx = ActivationVfx;
			BumperDataAsset->DeliveryVfx = DeliveryVfx;
			BumperDataAsset->ImpactVfx = ImpactVfx;
			BumperDataAsset->StatusVfx = StatusVfx;
			(void)BumperDataAsset->MarkPackageDirty();
		}
	}
}
