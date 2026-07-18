// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PBBumperEffectTableParser.generated.h"

/** Effect 시트 행을 파싱하고 연결된 범퍼 DataAsset의 클래스와 VFX 참조를 갱신합니다. */
UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperEffectTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBBumperEffectTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset EffectClassPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset BumperDataAssetPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Sheet")
	FPBSheetAssetPathPreset ActivationVfxPreset;

	void UpdateLinkedBumperDataAssets(FName EffectId, const struct FPBBumperEffectRow& EffectRow) const;
};
