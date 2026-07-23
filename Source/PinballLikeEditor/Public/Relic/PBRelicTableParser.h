
#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PinBallLike/Table/Relic/DataAsset/PBRelicDataAsset.h"
#include  "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "PBRelicTableParser.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBRelicTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

protected:
protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(
		FName RowName,
		const TMap<FString, FString>& RowData) override;

private:
	UPBRelicDataAsset* SetupRelicDataAsset(
		const FName RowName,
		const FPBRelicTableRow& RelicRow) const;
	
	UPBRelicTableParser();	

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|Sheet")
	FPBSheetAssetPathPreset DataAssetPreset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relic|Sheet")
	FPBSheetAssetPathPreset RelicIconPreset;
};