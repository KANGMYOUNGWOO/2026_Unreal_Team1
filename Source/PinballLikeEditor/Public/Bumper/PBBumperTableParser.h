// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoogleSheetParserBase.h"
#include "PBBumperTableParser.generated.h"

class UDataTable;
class UPBBumperDataAsset;

UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperTableParser : public UGoogleSheetParserBase
{
	GENERATED_BODY()

public:
	UPBBumperTableParser();

	virtual void OnParseComplete() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	TObjectPtr<UDataTable> TargetTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Asset", meta = (ContentDir))
	FDirectoryPath DataAssetFolderPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Asset")
	FString AssetNameFormat = TEXT("DA_Bumper_{0}");

	UPBBumperDataAsset* SetupBumperDataAsset(FName RowName) const;
};
