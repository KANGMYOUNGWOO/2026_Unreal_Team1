// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PBBumperTableParser.generated.h"

class UPBBumperDataAsset;
class UNiagaraSystem;

UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBBumperTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset DataAssetPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset IconPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset TriggerClassPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset EffectClassPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset ActivationVfxPreset;

	UPBBumperDataAsset* SetupBumperDataAsset(FName RowName, FName TriggerId, FName EffectId) const;
	bool ResolveEffectVfx(FName EffectId, struct FPBBumperEffectRow& OutEffectRow) const;
	TSoftObjectPtr<UNiagaraSystem> ResolveVfx(FName VfxId) const;
};
