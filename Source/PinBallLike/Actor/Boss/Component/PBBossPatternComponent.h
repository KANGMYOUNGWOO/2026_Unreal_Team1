#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "PBBossPatternComponent.generated.h"

class APBBossBase;
class UPBBossPatternBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBBossPatternChangedSignature, UPBBossPatternBase*, Pattern);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 보스 패턴 컴포넌트의 기본 값을 초기화합니다.
	UPBBossPatternComponent();

	// 게임 시작 시 패턴 인스턴스를 준비합니다.
	virtual void BeginPlay() override;
	// 컴포넌트 종료 시 타이머와 실행 중인 패턴을 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 보스 패턴 시스템을 시작하고 다음 패턴 검사를 예약합니다.
	void StartPatternSystem();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 보스 패턴 시스템을 중지하고 실행 중인 패턴을 취소합니다.
	void StopPatternSystem();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 패턴 시스템을 일시정지하고 남은 쿨다운 시간을 보존합니다.
	bool PausePatternSystem();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 일시정지된 패턴 시스템을 재개합니다.
	bool ResumePatternSystem();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 실행 가능한 다음 패턴을 찾아 시작합니다.
	void TryStartNextPattern();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 현재 실행 중인 패턴을 취소합니다.
	void CancelCurrentPattern();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 패턴 완료 알림을 받아 상태와 쿨다운을 갱신합니다.
	void NotifyPatternFinished(UPBBossPatternBase* FinishedPattern);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void NotifyEnragedPhaseStarted();

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	// 현재 보스가 새 패턴을 시작할 수 있는지 확인합니다.
	bool CanStartPattern() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	// 현재 실행 중인 패턴 인스턴스를 반환합니다.
	UPBBossPatternBase* GetCurrentPattern() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (DisplayName = "Normal Pattern Classes"))
	TArray<TSubclassOf<UPBBossPatternBase>> PatternClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<TSubclassOf<UPBBossPatternBase>> EnragedPatternClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<TSubclassOf<UPBBossPatternBase>> EnragedEntryPatternClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (ClampMin = "0"))
	float MinPatternIntervalSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (ClampMin = "0.1"))
	float PatternCheckIntervalSeconds = 0.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool IsPatternSystemActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool IsPatternRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool IsPatternSystemPaused = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool IsEnragedEntryPatternPending = false;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FPBBossPatternChangedSignature OnPatternStarted;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FPBBossPatternChangedSignature OnPatternFinished;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FPBBossPatternChangedSignature OnPatternCancelled;

private:
	// 설정된 패턴 클래스들로 패턴 인스턴스를 생성합니다.
	void InitializePatterns();
	void ResetPatternStartTime();
	void InitializePatternClasses(
		const TArray<TSubclassOf<UPBBossPatternBase>>& PatternClassList,
		TArray<TObjectPtr<UPBBossPatternBase>>& PatternInstanceList);
	// 다음 패턴 시작 가능 여부를 확인할 타이머를 예약합니다.
	void ScheduleNextPatternCheck();
	// 패턴 검사 타이머를 해제합니다.
	void ClearPatternCheckTimer();
	// 패턴 시스템을 비활성화하고 필요하면 현재 패턴 쿨다운을 적용합니다.
	void DeactivatePatternSystem(bool IsApplyCurrentPatternCooldown);
	// 지정 패턴의 쿨다운 종료 시간을 기록합니다.
	void SetPatternCooldown(UPBBossPatternBase* Pattern);
	// 현재 패턴을 내부적으로 취소하고 필요하면 쿨다운을 적용합니다.
	void CancelCurrentPatternInternal(bool IsApplyCooldown);
	// 현재 패턴 참조와 실행 상태를 초기화합니다.
	void ClearCurrentPattern();
	// 보스가 패턴 상태라면 Idle 상태로 되돌립니다.
	void SetOwnerBossIdleIfPatternState() const;
	// 일시정지 시간만큼 패턴 관련 타이머 기준 시간을 이동합니다.
	void ShiftPatternTimers(float DeltaSeconds);
	// 현재 월드 시간을 초 단위로 반환합니다.
	float GetCurrentTimeSeconds() const;
	// 지정 패턴의 쿨다운 종료 시간을 반환합니다.
	float GetPatternCooldownEndTime(const UPBBossPatternBase* Pattern) const;
	// 지정 패턴의 쿨다운이 끝났는지 확인합니다.
	bool IsPatternCooldownReady(const UPBBossPatternBase* Pattern) const;
	// 현재 실행 가능한 패턴 중 하나를 선택합니다.
	UPBBossPatternBase* SelectExecutablePattern() const;
	UPBBossPatternBase* SelectExecutablePatternFromList(const TArray<TObjectPtr<UPBBossPatternBase>>& PatternInstanceList) const;
	const TArray<TObjectPtr<UPBBossPatternBase>>& GetCurrentPhasePatternInstances() const;
	bool IsEnragedEntryPattern(const UPBBossPatternBase* Pattern) const;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossPatternBase> CurrentPattern;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBossPatternBase>> PatternInstances;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBossPatternBase>> EnragedPatternInstances;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBossPatternBase>> EnragedEntryPatternInstances;

	float NextPatternAllowedTime = 0.0f;
	float PatternSystemPausedTime = 0.0f;
	FTimerHandle PatternCheckTimerHandle;
	TMap<const UPBBossPatternBase*, float> CooldownEndTimeMap;
};
