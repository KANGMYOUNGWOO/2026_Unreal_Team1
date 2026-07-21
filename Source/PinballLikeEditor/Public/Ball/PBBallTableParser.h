// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PBBallTableParser.generated.h"

class UPBBallDataAsset;
struct FPBBallTableRow;

UCLASS()
class PINBALLLIKEEDITOR_API UPBBallTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBBallTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Sheet")
	FPBSheetAssetPathPreset DataAssetPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Sheet")
	FPBSheetAssetPathPreset BallIconPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Sheet")
	FPBSheetAssetPathPreset BallSpritePreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Sheet")
	FPBSheetAssetPathPreset SynergyIconPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Sheet")
	FPBSheetAssetPathPreset PowerFlipIconPreset;

	UPBBallDataAsset* SetupBallDataAsset(FName RowName, const FPBBallTableRow& BallRow) const;
};
