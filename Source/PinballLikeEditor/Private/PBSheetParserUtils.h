// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "DataTableEditorUtils.h"
#include "Engine/DataTable.h"
#include "PathDataLoadHelper.h"
#include "PinBallLike/Struct/Bumper/PBBumperTriggerSpawnInfo.h"
#include "UObject/Package.h"

namespace PBSheetParserUtils
{
	// 빈 셀과 "None"은 미설정 값으로 처리.
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

	// 대상 테이블의 RowStruct가 파서와 맞는지 확인.
	inline bool ValidateTargetTable(const UDataTable* TargetTable, const UScriptStruct* ExpectedRowStruct)
	{
		return IsValid(TargetTable)
			&& IsValid(ExpectedRowStruct)
			&& TargetTable->GetRowStruct() == ExpectedRowStruct;
	}

	// 파서 공용 로그 헬퍼.
	inline void LogTargetTableError(const TCHAR* ParserName, const UDataTable* TargetTable, const UScriptStruct* ExpectedRowStruct)
	{
		UE_LOG(LogTemp, Error, TEXT("[Sheet][%s] TargetTable is invalid. Table=%s ExpectedRow=%s ActualRow=%s"),
			ParserName,
			*GetNameSafe(TargetTable),
			*GetNameSafe(ExpectedRowStruct),
			TargetTable ? *GetNameSafe(TargetTable->GetRowStruct()) : TEXT("None"));
	}

	inline void LogHeaders(const TCHAR* ParserName, const TArray<FString>& Headers)
	{
		UE_LOG(LogTemp, Log, TEXT("[Sheet][%s] Headers=%s"),
			ParserName,
			*FString::Join(Headers, TEXT(", ")));
	}

	// 에디터 갱신을 위해 변경 알림과 함께 테이블 초기화.
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

	// enum 파싱
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

	// 숫자 셀 파싱.
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

	// 쉼표로 구분된 PositionId 목록 파싱.
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

	inline FString MakeGeneratedAssetName(const FString& NameFormat, const FName RowName)
	{
		return FString::Format(*NameFormat, { RowName.ToString() });
	}

	// RowName 기준으로 데이터 에셋을 찾거나 생성.
	template <typename TDataAsset>
	TDataAsset* GetOrCreateDataAsset(
		const FPBSheetAssetPathPreset& Preset,
		const FName RowName,
		const TCHAR* ParserName)
	{
		const FString RowNameString = RowName.ToString();
		if (IsUnsetValue(RowNameString) || !Preset.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Sheet][%s] Skip DataAsset setup because asset preset is invalid. RowName=%s"),
				ParserName,
				*RowNameString);
			return nullptr;
		}

		const FString AssetName = Preset.MakeAssetName(RowName);
		TDataAsset* DataAsset =
			UPathDataLoadHelper::GetOrCreateAsset<TDataAsset>(Preset.FolderPath.Path, AssetName);
		if (!IsValid(DataAsset))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Sheet][%s] Failed to get or create DataAsset. RowName=%s AssetName=%s"),
				ParserName,
				*RowNameString,
				*AssetName);
			return nullptr;
		}

		// PrimaryAssetId가 에셋명이 아닌 시트 RowName 기준이 되도록 저장.
		DataAsset->RowName = RowName;
		(void)DataAsset->MarkPackageDirty();
		UE_LOG(LogTemp, Log, TEXT("[Sheet][%s] Linked DataAsset. RowName=%s Asset=%s PrimaryAssetId=%s"),
			ParserName,
			*RowNameString,
			*GetNameSafe(DataAsset),
			*DataAsset->GetPrimaryAssetId().ToString());

		return DataAsset;
	}

	// 생성된 블루프린트 클래스를 찾아 _C 클래스 참조 경로 반환.
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
			UE_LOG(LogTemp, Warning, TEXT("[Sheet] Class asset not found. FolderPath=%s ClassName=%s"),
				*FolderPath,
				*ClassName);
			return FString();
		}

		const FString AssetReferencePath = AssetDatas[0].GetSoftObjectPath().ToString();
		return FString::Printf(TEXT("%s_C"), *AssetReferencePath);
	}

	// 생성된 에셋 오브젝트를 찾아 소프트 오브젝트 경로 반환.
	inline FString FindObjectReferencePath(
		const FString& FolderPath,
		const FString& ObjectNameFormat,
		const FName RowName)
	{
		if (IsUnsetValue(FolderPath) || IsUnsetValue(ObjectNameFormat) || IsUnsetValue(RowName.ToString()))
		{
			return FString();
		}

		const FString ObjectName = MakeGeneratedAssetName(ObjectNameFormat, RowName);
		const TArray<FAssetData> AssetDatas = UPathDataLoadHelper::GetAssetsByPathFilter(FolderPath, ObjectName);
		if (AssetDatas.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Sheet] Object asset not found. FolderPath=%s ObjectName=%s"),
				*FolderPath,
				*ObjectName);
			return FString();
		}

		return AssetDatas[0].GetSoftObjectPath().ToString();
	}

	// 파서 프리셋용 편의 래퍼.
	template <typename TClass>
	TSoftClassPtr<TClass> FindBlueprintClass(
		const FPBSheetAssetPathPreset& Preset,
		const FName RowName)
	{
		const FString ClassPath = FindBlueprintClassReferencePath(
			Preset.FolderPath.Path,
			Preset.NameFormat,
			RowName);
		if (ClassPath.IsEmpty())
		{
			return nullptr;
		}

		return TSoftClassPtr<TClass>(FSoftObjectPath(ClassPath));
	}

	template <typename TObjectType>
	TSoftObjectPtr<TObjectType> FindObject(
		const FPBSheetAssetPathPreset& Preset,
		const FName RowName)
	{
		const FString ObjectPath = FindObjectReferencePath(
			Preset.FolderPath.Path,
			Preset.NameFormat,
			RowName);
		if (ObjectPath.IsEmpty())
		{
			return nullptr;
		}

		return TSoftObjectPtr<TObjectType>(FSoftObjectPath(ObjectPath));
	}
}
