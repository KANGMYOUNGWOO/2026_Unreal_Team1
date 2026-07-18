// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "PBBumperEffectRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBumperEffectRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	EPBBumperEffectType EffectType = EPBBumperEffectType::Instant;

	/** 효과의 완료 계약. Immediate도 잘못 장기 실행되면 공유 Effect 재진입을 막기 위해 안전하게 대기한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	EPBBumperEffectExecutionPolicy ExecutionPolicy = EPBBumperEffectExecutionPolicy::Immediate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	float Power = 0.0f;

	/** 범퍼 전용 주 수치 외에 필요한 두 번째 수치입니다. 0이면 사용하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	float SecondaryPower = 0.0f;

	/** 지속형 효과의 시간입니다. 0이면 해당 Effect 클래스의 기존 기본값을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	/** 발사 횟수처럼 정수 계약이 필요한 효과가 사용합니다. 0이면 기존 기본값을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect", meta = (ClampMin = "0"))
	int32 Count = 0;

	/** 공용 효과 시트의 Effect RowName입니다. 비어 있으면 기존 범퍼 전용 Power 계약을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName SharedEffectId = NAME_None;

	/** 효과가 실제 실행될 때 재생할 Niagara System의 논리 에셋 이름. None이면 연출을 생략한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName ActivationVfxId = NAME_None;

	/** 효과가 대상까지 이동하는 동안 재생할 Niagara System의 논리 에셋 이름. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName DeliveryVfxId = NAME_None;

	/** 효과가 실제 대상에 적용된 순간 재생할 Niagara System의 논리 에셋 이름. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName ImpactVfxId = NAME_None;

	/** 효과가 유지되는 동안 대상 또는 생성 Actor에 부착할 Niagara System의 논리 에셋 이름. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FName StatusVfxId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Effect")
	FText Description;

};
