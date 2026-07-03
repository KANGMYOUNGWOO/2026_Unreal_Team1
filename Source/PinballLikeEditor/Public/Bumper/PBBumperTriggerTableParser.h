// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoogleSheetParserBase.h"
#include "PBBumperTriggerTableParser.generated.h"

class UDataTable;
class UPBBumperTriggerDataAsset;

UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperTriggerTableParser : public UGoogleSheetParserBase
{
	GENERATED_BODY()

public:
	UPBBumperTriggerTableParser();

	virtual void OnParseComplete() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	TObjectPtr<UDataTable> TargetTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Asset", meta = (ContentDir))
	FDirectoryPath DataAssetFolderPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Asset")
	FString AssetNameFormat = TEXT("DA_Trigger_{0}");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Class", meta = (ContentDir))
	FDirectoryPath TriggerClassFolderPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Class")
	FString TriggerClassNameFormat = TEXT("BP_{0}");

	UPBBumperTriggerDataAsset* SetupTriggerDataAsset(FName RowName) const;
};
