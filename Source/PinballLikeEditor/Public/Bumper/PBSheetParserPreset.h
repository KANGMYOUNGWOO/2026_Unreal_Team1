// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBSheetParserPreset.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKEEDITOR_API FPBSheetAssetPathPreset
{
	GENERATED_BODY()

	/** 생성하거나 검색할 에셋이 위치한 Content 폴더 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheet|Asset", meta = (ContentDir))
	FDirectoryPath FolderPath;

	/** RowName을 넣어 에셋명을 만드는 포맷. 예: DA_Bumper_{0} */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheet|Asset")
	FString NameFormat = TEXT("{0}");

	/** 파서가 사용할 수 있는 경로/이름 포맷인지 확인함 */
	bool IsValid() const
	{
		const FString TrimmedFolderPath = FolderPath.Path.TrimStartAndEnd();
		const FString TrimmedNameFormat = NameFormat.TrimStartAndEnd();
		return !TrimmedFolderPath.IsEmpty()
			&& !TrimmedFolderPath.Equals(TEXT("None"), ESearchCase::IgnoreCase)
			&& !TrimmedNameFormat.IsEmpty()
			&& !TrimmedNameFormat.Equals(TEXT("None"), ESearchCase::IgnoreCase);
	}

	/** RowName을 기준으로 실제 에셋명을 생성함 */
	FString MakeAssetName(const FName RowName) const
	{
		return FString::Format(*NameFormat, { RowName.ToString() });
	}
};
