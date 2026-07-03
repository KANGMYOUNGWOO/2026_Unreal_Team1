// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataTableEditorUtils.h"
#include "Engine/DataTable.h"
#include "PathDataLoadHelper.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "UObject/Package.h"

namespace PBSheetParserUtils
{
	inline FString TrimCell(const FString& Value)
	{
		return Value.TrimStartAndEnd();
	}

	inline bool IsUnsetValue(const FString& Value)
	{
		const FString TrimmedValue = TrimCell(Value);
		return TrimmedValue.IsEmpty()
			|| TrimmedValue.Equals(TEXT("None"), ESearchCase::IgnoreCase);
	}

	inline bool ValidateTargetTable(const UDataTable* TargetTable, const UScriptStruct* ExpectedRowStruct)
	{
		return IsValid(TargetTable)
			&& IsValid(ExpectedRowStruct)
			&& TargetTable->GetRowStruct() == ExpectedRowStruct;
	}

	inline void LogTargetTableError(const TCHAR* ParserName, const UDataTable* TargetTable, const UScriptStruct* ExpectedRowStruct)
	{
		UE_LOG(LogTemp, Error, TEXT("[BumperSheet][%s] TargetTable is invalid. Table=%s ExpectedRow=%s ActualRow=%s"),
			ParserName,
			*GetNameSafe(TargetTable),
			*GetNameSafe(ExpectedRowStruct),
			TargetTable ? *GetNameSafe(TargetTable->GetRowStruct()) : TEXT("None"));
	}

	inline void LogHeaders(const TCHAR* ParserName, const TArray<FString>& Headers)
	{
		UE_LOG(LogTemp, Log, TEXT("[BumperSheet][%s] Headers=%s"),
			ParserName,
			*FString::Join(Headers, TEXT(", ")));
	}

	inline void ResetTargetTable(UDataTable* TargetTable)
	{
		FDataTableEditorUtils::BroadcastPreChange(
			TargetTable,
			FDataTableEditorUtils::EDataTableChangeInfo::RowList);
		TargetTable->Modify();
		TargetTable->EmptyTable();
	}

	inline void MarkTargetTableDirty(UDataTable* TargetTable)
	{
		FDataTableEditorUtils::BroadcastPostChange(
			TargetTable,
			FDataTableEditorUtils::EDataTableChangeInfo::RowList);
		(void)TargetTable->MarkPackageDirty();
		if (UPackage* Package = TargetTable->GetPackage())
		{
			(void)Package->MarkPackageDirty();
		}
	}

	template <typename TEnum>
	TEnum ParseEnumValue(const FString& Value, const TEnum DefaultValue)
	{
		const UEnum* Enum = StaticEnum<TEnum>();
		if (!Enum)
		{
			return DefaultValue;
		}

		const FString TrimmedValue = TrimCell(Value);
		const int64 EnumValue = Enum->GetValueByNameString(TrimmedValue);
		if (EnumValue == INDEX_NONE)
		{
			return DefaultValue;
		}

		return static_cast<TEnum>(EnumValue);
	}

	inline int32 ParseIntValue(const FString& Value, const int32 DefaultValue)
	{
		if (IsUnsetValue(Value))
		{
			return DefaultValue;
		}

		return FCString::Atoi(*Value);
	}

	inline float ParseFloatValue(const FString& Value, const float DefaultValue)
	{
		if (IsUnsetValue(Value))
		{
			return DefaultValue;
		}

		return FCString::Atof(*Value);
	}

	inline TArray<EPBBumperPositionId> ParsePositionIds(const FString& Value)
	{
		TArray<EPBBumperPositionId> PositionIds;
		TArray<FString> Tokens;
		Value.ParseIntoArray(Tokens, TEXT(","), true);

		for (const FString& Token : Tokens)
		{
			const EPBBumperPositionId PositionId = ParseEnumValue(Token, EPBBumperPositionId::None);
			if (PositionId != EPBBumperPositionId::None)
			{
				PositionIds.Add(PositionId);
			}
		}

		return PositionIds;
	}

	inline FString MakeGeneratedAssetName(const FString& AssetNameFormat, const FName RowName)
	{
		return FString::Format(*AssetNameFormat, { RowName.ToString() });
	}

	inline FString FindBlueprintClassReferencePath(
		const FString& FolderPath,
		const FString& ClassNameFormat,
		const FName RowName)
	{
		if (IsUnsetValue(FolderPath) || IsUnsetValue(ClassNameFormat) || IsUnsetValue(RowName.ToString()))
		{
			return FString();
		}

		const FString ClassName = MakeGeneratedAssetName(ClassNameFormat, RowName);
		const TArray<FAssetData> AssetDatas = UPathDataLoadHelper::GetAssetsByPathFilter(FolderPath, ClassName);
		if (AssetDatas.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[BumperSheet] Class asset not found. FolderPath=%s ClassName=%s"),
				*FolderPath,
				*ClassName);
			return FString();
		}

		const FString AssetReferencePath = AssetDatas[0].GetSoftObjectPath().ToString();
		return FString::Printf(TEXT("%s_C"), *AssetReferencePath);
	}
}
