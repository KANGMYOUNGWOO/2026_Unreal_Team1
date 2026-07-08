// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PinBallLike/Table/Boss/Struct/PBBossPatternPhaseType.h"
#include "PBBossPatternTableParser.generated.h"

class UPBBossDataAsset;
class UPBBossPatternBase;

UCLASS()
class PINBALLLIKEEDITOR_API UPBBossPatternTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBBossPatternTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
	virtual void OnParseComplete() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sheet")
	FPBSheetAssetPathPreset BossDataAssetPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Sheet")
	TArray<FPBSheetAssetPathPreset> PatternClassPresets;

	TSet<FName> ClearedBossRowNames;

	void UpdateLinkedBossDataAsset(
		FName BossRowName,
		FName PatternClassId,
		EPBBossPatternPhaseType PatternPhaseType,
		FName PatternName,
		float CooldownSeconds,
		bool IsEnabled);
	TSoftClassPtr<UPBBossPatternBase> FindPatternClass(FName PatternClassId) const;
	void ClearBossPatternData(FName BossRowName);
};
