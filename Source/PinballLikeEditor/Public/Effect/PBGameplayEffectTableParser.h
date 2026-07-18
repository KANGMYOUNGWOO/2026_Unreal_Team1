#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "PBGameplayEffectTableParser.generated.h"

/** 공용 Effect 시트를 DT_GameplayEffect로 변환합니다. */
UCLASS()
class PINBALLLIKEEDITOR_API UPBGameplayEffectTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
};
