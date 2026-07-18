#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PBBumperVfxGenerationCommandlet.generated.h"

/**
 * 범퍼 효과별 Stateless Niagara 자산을 동일한 시각 규칙으로 생성합니다.
 * 기본 실행은 기존 시스템을 보존하는 create-only 모드입니다.
 * -UpdateExisting은 예상 템플릿의 기존 시스템을 갱신하고, -ForceReplace는 예상 템플릿과
 * 다른 기존 시스템까지 삭제 후 재생성하므로 수동 검토 후에만 사용합니다.
 * 사용자 Mesh/Level/Texture는 수정하지 않습니다.
 */
UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperVfxGenerationCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UPBBumperVfxGenerationCommandlet();
	virtual int32 Main(const FString& Params) override;
};
