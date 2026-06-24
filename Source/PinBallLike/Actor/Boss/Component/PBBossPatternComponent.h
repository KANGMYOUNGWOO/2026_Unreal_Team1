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
	UPBBossPatternComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void StartPatternSystem();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void StopPatternSystem();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void TryStartNextPattern();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void CancelCurrentPattern();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void NotifyPatternFinished(UPBBossPatternBase* FinishedPattern);

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	bool CanStartPattern() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	UPBBossPatternBase* GetCurrentPattern() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<TSubclassOf<UPBBossPatternBase>> PatternClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (ClampMin = "0"))
	float MinPatternIntervalSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (ClampMin = "0.1"))
	float PatternCheckIntervalSeconds = 0.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool IsPatternSystemActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
	bool IsPatternRunning = false;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FPBBossPatternChangedSignature OnPatternStarted;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FPBBossPatternChangedSignature OnPatternFinished;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Pattern")
	FPBBossPatternChangedSignature OnPatternCancelled;

private:
	void InitializePatterns();
	void ScheduleNextPatternCheck();
	void ClearPatternCheckTimer();
	void SetPatternCooldown(UPBBossPatternBase* Pattern);
	void ClearCurrentPattern();
	void SetOwnerBossIdleIfPatternState() const;
	float GetCurrentTimeSeconds() const;
	float GetPatternCooldownEndTime(const UPBBossPatternBase* Pattern) const;
	bool IsPatternCooldownReady(const UPBBossPatternBase* Pattern) const;
	UPBBossPatternBase* SelectExecutablePattern() const;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossPatternBase> CurrentPattern;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBossPatternBase>> PatternInstances;

	float NextPatternAllowedTime = 0.0f;
	FTimerHandle PatternCheckTimerHandle;
	TMap<const UPBBossPatternBase*, float> CooldownEndTimeMap;
};
