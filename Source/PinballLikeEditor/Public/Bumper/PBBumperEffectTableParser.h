// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoogleSheetParserBase.h"
#include "PBBumperEffectTableParser.generated.h"

class UDataTable;
class UPBBumperEffectDataAsset;

UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperEffectTableParser : public UGoogleSheetParserBase
{
	GENERATED_BODY()

public:
	UPBBumperEffectTableParser();

	virtual void OnParseComplete() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	TObjectPtr<UDataTable> TargetTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Asset", meta = (ContentDir))
	FDirectoryPath DataAssetFolderPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Asset")
	FString AssetNameFormat = TEXT("DA_Effect_{0}");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Class", meta = (ContentDir))
	FDirectoryPath EffectClassFolderPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet|Class")
	FString EffectClassNameFormat = TEXT("BP_{0}");

	UPBBumperEffectDataAsset* SetupEffectDataAsset(FName RowName) const;
};
