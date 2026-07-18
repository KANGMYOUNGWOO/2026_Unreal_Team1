// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PBBumperTriggerTableParser.generated.h"

/** Trigger 시트 행을 파싱하고 연결된 범퍼 DataAsset의 Trigger 클래스 참조를 갱신합니다. */
UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperTriggerTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBBumperTriggerTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset TriggerClassPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset BumperDataAssetPreset;

	void UpdateLinkedBumperDataAssets(FName TriggerId) const;
};
