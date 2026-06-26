#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternBase.h"
#include "TimerManager.h"
#include "PBBossChargePattern.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossChargePattern : public UPBBossPatternBase
{
	GENERATED_BODY()

protected:
	// 돌진 패턴을 실행할 수 있는지 검사합니다.
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	// 돌진 전 경고 표시와 준비 단계를 시작합니다.
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
	// 보스의 돌진 방향과 시작 위치를 계산하고 돌진을 실행합니다.
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	// 돌진 패턴 취소 시 타이머와 충돌 상태를 정리합니다.
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;
	// C++에서 돌진 패턴 실행 흐름을 직접 시작합니다.
	virtual void ExecuteNativePattern(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ChargeMaxDistance = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReboundDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReboundSeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float GroggySeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReturnSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0"))
	float ReturnAcceptanceRadius = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Charge", meta = (ClampMin = "0.001"))
	float UpdateIntervalSeconds = 0.016f;

private:
	// 돌진 시작 위치, 방향, 타겟 정보를 미리 계산합니다.
	void PrepareCharge(APBBossBase* Boss);
	// 돌진 전방 경고용 텔레그래프 액터를 생성합니다.
	void SpawnChargeTelegraph(APBBossBase* Boss);
	// 경고 표시가 끝난 뒤 실제 돌진 실행을 시작합니다.
	void StartExecuteChargePattern();
	// 돌진 타이머를 시작하고 핀볼 충돌 처리를 조정합니다.
	void StartCharge();
	// 돌진 중 보스를 이동시키고 충돌 또는 최대 거리 도달을 확인합니다.
	void UpdateCharge();
	// 돌진이 막히거나 끝났을 때 반동 단계로 전환합니다.
	void HandleChargeBlocked(const FHitResult& Hit);
	// 충돌 후 보스를 반대 방향으로 살짝 밀어냅니다.
	void StartRebound();
	// 반동 단계를 끝내고 그로기 단계로 전환합니다.
	void FinishRebound();
	// 돌진 후 그로기 대기 타이머를 시작합니다.
	void StartGroggy();
	// 그로기 단계를 끝내고 복귀 단계로 전환합니다.
	void FinishGroggy();
	// 보스를 돌진 시작 위치로 되돌리는 복귀 타이머를 시작합니다.
	void StartReturn();
	// 복귀 중 보스를 시작 위치 방향으로 이동시킵니다.
	void UpdateReturn();
	// 복귀를 완료하고 패턴 상태를 정리합니다.
	void FinishReturn();
	// 바닥 충돌은 무시하면서 보스를 지정 오프셋만큼 이동시킵니다.
	bool MoveBossWithFloorIgnored(APBBossBase* Boss, const FVector& MoveOffset, FHitResult& OutHit) const;
	// 충돌 결과가 바닥 충돌인지 판별합니다.
	bool IsFloorHit(const FHitResult& Hit) const;
	// 돌진 패턴에서 사용하는 모든 타이머를 해제합니다.
	void ClearPatternTimers();
	// 돌진 경고 표시 대기 타이머를 해제합니다.
	void ClearChargeTelegraphTimer();
	// 생성된 돌진 경고 텔레그래프를 제거합니다.
	void DestroyChargeTelegraph();
	// 돌진 중 보스가 핀볼 이동을 막지 않도록 무시 상태를 설정합니다.
	void SetPinballMoveIgnored(bool IsIgnored) const;
	// 돌진 단계에서 핀볼 충돌 데미지 차단 여부를 설정합니다.
	void SetPinballCollisionDamageBlocked(bool IsBlocked) const;
	// 월드에서 핀볼 액터를 찾아 반환합니다.
	AActor* FindPinballActor() const;

	FVector ChargeStartLocation = FVector::ZeroVector;
	FRotator ChargeStartRotation = FRotator::ZeroRotator;
	FVector ChargeDirection = FVector::ForwardVector;
	float ChargedDistance = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetPinballActor;

	UPROPERTY(Transient)
	TObjectPtr<class APBBossChargeTelegraph> SpawnedChargeTelegraph;

	FTimerHandle ChargeTelegraphTimerHandle;
	FTimerHandle ChargeTimerHandle;
	FTimerHandle ReboundTimerHandle;
	FTimerHandle GroggyTimerHandle;
	FTimerHandle ReturnTimerHandle;
};
