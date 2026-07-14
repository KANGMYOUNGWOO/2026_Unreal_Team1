#pragma once

#include "CoreMinimal.h"
#include "Bumper/PBSheetParserPreset.h"
#include "PBTableParserBase.h"
#include "PBSkillTableParser.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBSkillTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

public:
	UPBSkillTableParser();

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Sheet")
	FPBSheetAssetPathPreset SkillActorClassPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Sheet")
	FPBSheetAssetPathPreset BallDataAssetPreset;

	void UpdateLinkedBallDataAssets(FName SkillId) const;
};
