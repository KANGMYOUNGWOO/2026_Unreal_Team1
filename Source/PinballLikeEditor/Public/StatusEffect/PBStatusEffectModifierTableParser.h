// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "PBStatusEffectModifierTableParser.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBStatusEffectModifierTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
};
