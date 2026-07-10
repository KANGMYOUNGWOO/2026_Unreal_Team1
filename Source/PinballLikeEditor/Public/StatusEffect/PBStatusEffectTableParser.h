// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBStatusEffectTableParser.generated.h"

class UPBStatusEffectDataAsset;
class UTexture2D;

UCLASS()
class PINBALLLIKEEDITOR_API UPBStatusEffectTableParser : public UPBTableParserBase
{
	GENERATED_BODY()
	
public:
	UPBStatusEffectTableParser();
	
protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Sheet")
	FPBSheetAssetPathPreset DataAssetPreset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|Sheet")
	FPBSheetAssetPathPreset IconPreset;
	
	UPBStatusEffectDataAsset* SetupStatusEffectDataAsset(FName RowName, const TMap<FString, FString>& RowData) const;
};
