// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossHitPartComponent.h"
#include "PBBossHitPointTableParser.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBBossHitPointTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBBossHitPointTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual void OnParseComplete() override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;

private:
	void ClearBossHitPointData(FName BossRowName);
	void UpdateLinkedBossDataAsset(
		FName BossRowName,
		FName HitPointName,
		EPBBossHitPartType HitPartType,
		int32 HPDamageMultiplierPercent,
		int32 GroggyMultiplierPercent,
		bool IsWeaknessPoint);

	FPBSheetAssetPathPreset BossDataAssetPreset;
	TSet<FName> ClearedBossRowNames;
};
