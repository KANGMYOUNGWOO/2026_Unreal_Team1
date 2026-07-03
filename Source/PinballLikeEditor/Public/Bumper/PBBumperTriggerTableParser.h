// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PBBumperTriggerTableParser.generated.h"

class UPBBumperTriggerDataAsset;

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
	FPBSheetAssetPathPreset DataAssetPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset TriggerClassPreset;

	UPBBumperTriggerDataAsset* SetupTriggerDataAsset(FName RowName) const;
};
