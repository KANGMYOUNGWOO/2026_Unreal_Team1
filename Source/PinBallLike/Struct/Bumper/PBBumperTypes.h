// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBBumperTypes.generated.h"

UENUM(BlueprintType)
enum class EPBBumperType : uint8
{
	Rebound,
	Side,
	TopTarget,
	Gate
};

UENUM(BlueprintType)
enum class EPBBumperTriggerType : uint8
{
	HitCount,
	PassCount,
	RailEnter
};

UENUM(BlueprintType)
enum class EPBBumperRoleType : uint8
{
	Attack,
	Spawn,
	Support,
	Zone
};

UENUM(BlueprintType)
enum class EPBBumperEffectType : uint8
{
	Instant,
	Buff,
	Area,
	Summon
};

/**
 * 하나의 모듈 범퍼에서 이전 효과가 아직 끝나지 않았을 때 새 발동 요청을 처리하는 방식이다.
 * Instant와 StatusEffect 적용형 Buff는 실행 후 즉시 종료하는 것이 기본이며,
 * 지속 연출·소환처럼 실행 레인을 오래 점유하는 효과는 QueueIfBusy를 사용한다.
 */
UENUM(BlueprintType)
enum class EPBBumperEffectExecutionPolicy : uint8
{
	/** 같은 호출 흐름에서 FinishEffect까지 완료하는 즉시 효과용 계약이다. */
	Immediate,
	/** 연출·소환 완료 콜백까지 실행 레인을 점유하고 다음 요청을 FIFO로 기다리게 한다. */
	QueueIfBusy
};
