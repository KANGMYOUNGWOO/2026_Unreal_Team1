#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PinBallLike/Table/Synergy/DataAsset/PBSynergyDataAsset.h"
#include "PBSynergyTableParser.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBSynergyTableParser : public UPBTableParserBase
{
	GENERATED_BODY()
	
public:
	UPBSynergyTableParser();
	
protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|Sheet")
	FPBSheetAssetPathPreset DataAssetPreset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy|Sheet")
	FPBSheetAssetPathPreset IconPreset;
	
	UPBSynergyDataAsset* SetupSynergyDataAsset(FName RowName, const TMap<FString, FString>& RowData) const;
};
