#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "PBShopTableParser.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBShopTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
	

};