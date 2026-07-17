// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GoogleSheetParserBase.h"
#include "PBTableParserBase.generated.h"

class UDataTable;

UCLASS(Abstract)
class PINBALLLIKEEDITOR_API UPBTableParserBase : public UGoogleSheetParserBase
{
	GENERATED_BODY()

public:
	virtual void OnParseComplete() override;

	/** 에디터 자동화에서도 Details 패널과 같은 방식으로 출력 DataTable을 지정합니다. */
	void SetTargetTable(UDataTable* InTargetTable) { TargetTable = InTargetTable; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sheet")
	TObjectPtr<UDataTable> TargetTable;

	virtual const TCHAR* GetParserName() const;
	virtual UScriptStruct* GetRowStruct() const;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData);
};
