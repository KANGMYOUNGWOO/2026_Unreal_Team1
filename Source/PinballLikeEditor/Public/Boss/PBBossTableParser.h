// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PBBossTableParser.generated.h"

class UPBBossDataAsset;

UCLASS()
class PINBALLLIKEEDITOR_API UPBBossTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBBossTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sheet")
	FPBSheetAssetPathPreset DataAssetPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sheet")
	FPBSheetAssetPathPreset IconPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sheet")
	FPBSheetAssetPathPreset BossUILayerClassPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sheet")
	FPBSheetAssetPathPreset EnrageCameraShakeClassPreset;

	UPBBossDataAsset* SetupBossDataAsset(FName RowName, const TMap<FString, FString>& RowData) const;
	TSoftClassPtr<class APBBossBase> FindBossClass(FName BossClassId) const;
	
private:
	FName GetClassIdOrRowName(const TMap<FString, FString>& RowData, const FString& ColumnName, const FName RowName) const;
	
};
