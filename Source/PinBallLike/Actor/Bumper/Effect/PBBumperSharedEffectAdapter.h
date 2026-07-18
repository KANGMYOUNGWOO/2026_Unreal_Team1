#pragma once

#include "CoreMinimal.h"

struct FPBGameplayEffectRow;

/** 범퍼가 공용 효과 시트에서 읽어 온 검증 완료 정의입니다. */
struct FPBBumperSharedEffectDefinition
{
	FName EffectId = NAME_None;
	FName EffectType = NAME_None;
	FName TargetType = NAME_None;
	FName TargetFilter = NAME_None;
	TMap<FName, FString> Parameters;

	bool TryGetFloat(FName Key, float& OutValue) const;
	bool TryGetInt(FName Key, int32& OutValue) const;
	bool TryGetName(FName Key, FName& OutValue) const;
};

/**
 * 공용 효과 시트와 범퍼 전용 Effect 클래스 사이의 명시적 경계입니다.
 * 지원하지 않는 타입, 누락 파라미터, 알 수 없는 Resource/Stat 이름은 기본값으로 숨기지 않고 실패시킵니다.
 */
namespace PBBumperSharedEffectAdapter
{
	/** 시트 Row가 범퍼 Effect 클래스가 선언한 실행 대상 계약과 일치하는지 검증합니다. */
	bool ValidateContract(
		const FPBGameplayEffectRow& EffectRow,
		FName ExpectedEffectType,
		FName ExpectedTargetType,
		FName ExpectedTargetFilter,
		FString& OutError);

	bool Resolve(
		const UObject* WorldContext,
		FName SharedEffectId,
		FName ExpectedEffectType,
		FName ExpectedTargetType,
		FName ExpectedTargetFilter,
		TConstArrayView<FName> RequiredParameters,
		FPBBumperSharedEffectDefinition& OutDefinition,
		FString& OutError);

	bool ResolveResourceName(FName SheetName, FName& OutRuntimeName);
	bool ResolveStatName(FName SheetName, FName& OutRuntimeName);
}
