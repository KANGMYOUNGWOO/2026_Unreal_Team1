#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "PBSynergyEffectModifierTableParser.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBSynergyEffectModifierTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
};
