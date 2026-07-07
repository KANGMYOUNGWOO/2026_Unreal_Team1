#pragma once

#include "CoreMinimal.h"
#include "PBTableParserBase.h"
#include "PBCollectionTableParser.generated.h"

/**
 * DT_Collection_Export 시트를 도감 DataTable 행으로 변환하는 에디터 전용 파서입니다.
 * Collection_Master는 사람이 관리하는 허브 탭이고, 이 파서는 언리얼로 내보낼 최종 헤더만 읽습니다.
 * 런타임 도감은 이 파서가 채운 FPBCollectionTableRow 데이터를 읽어 목록 UI를 구성합니다.
 */
UCLASS()
class PINBALLLIKEEDITOR_API UPBCollectionTableParser : public UPBTableParserBase
{
	GENERATED_BODY()

protected:
	virtual const TCHAR* GetParserName() const override;
	virtual UScriptStruct* GetRowStruct() const override;
	virtual bool ParseRow(FName RowName, const TMap<FString, FString>& RowData) override;
};
