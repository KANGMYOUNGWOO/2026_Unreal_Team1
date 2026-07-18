#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PBBumperDataSyncCommandlet.generated.h"

/**
 * 승인된 범퍼 시트 세대를 로컬 Unreal 에셋으로 동기화합니다.
 *
 * 마우스 편집을 대신하는 에디터 전용 명령이며 다음 작업만 수행합니다.
 * - 공유 효과 DataTable/Loader 생성 및 시트 로드
 * - 범퍼 Trigger -> Effect -> Bumper 순서 로드
 *
 * -MigrateLegacyAssets를 명시한 경우에만 승인된 Blueprint/DataAsset/Icon 이름 변경과
 * Effect Blueprint 재부모화를 수행합니다. 기본 실행은 시트 동기화만 수행합니다.
 * 저장 범위는 명시한 범퍼 테이블/로더와 범퍼 DataAsset으로 제한합니다.
 *
 * 사용자 제작 Mesh, Texture, Level은 수정하지 않습니다.
 */
UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperDataSyncCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UPBBumperDataSyncCommandlet();
	virtual int32 Main(const FString& Params) override;
};
