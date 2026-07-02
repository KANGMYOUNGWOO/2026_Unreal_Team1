#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternTelegraphData.h"
#include "UObject/Object.h"
#include "PBBossPatternBase.generated.h"

class APBBossBase;
class APBBossPatternTelegraph;
class AActor;
class UPBBossPatternComponent;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBBossPatternBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 패턴이 소유될 패턴 컴포넌트를 초기화합니다.
	void InitializePattern(UPBBossPatternComponent* NewOwnerPatternComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	// 현재 보스 상태에서 이 패턴을 실행할 수 있는지 확인합니다.
	bool CanExecute(APBBossBase* Boss) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	// 패턴 시작 단계와 텔레그래프 표시를 처리합니다.
	void StartPattern(APBBossBase* Boss);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	// 실행 중인 패턴을 외부 요청으로 취소합니다.
	void CancelPattern(APBBossBase* Boss);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	// 패턴의 실제 동작을 실행합니다.
	void ExecutePattern(APBBossBase* Boss);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	// 자식 패턴에서 필요한 내부 취소 정리를 수행합니다.
	void CancelPatternInternal(APBBossBase* Boss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	// 패턴 완료를 소유 패턴 컴포넌트에 알립니다.
	void FinishPattern();

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	// 이 패턴을 소유한 패턴 컴포넌트를 반환합니다.
	UPBBossPatternComponent* GetOwnerPatternComponent() const;

	virtual bool PausePatternForExternalGroggy(APBBossBase* Boss);
	virtual bool ResumePatternAfterExternalGroggy(APBBossBase* Boss);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	FName PatternName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (ClampMin = "0"))
	float CooldownSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	bool IsEnabled = true;

protected:
	// 기본 패턴 실행 가능 조건을 검사합니다.
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const;
	// 기본 패턴 시작 흐름을 실행합니다.
	virtual void StartPattern_Implementation(APBBossBase* Boss);
	// 기본 패턴 취소 흐름을 실행합니다.
	virtual void CancelPattern_Implementation(APBBossBase* Boss);
	// 기본 패턴 실행 진입점을 제공합니다.
	virtual void ExecutePattern_Implementation(APBBossBase* Boss);
	// 기본 내부 취소 정리 진입점을 제공합니다.
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss);
	// C++ 전용 패턴 실행 진입점을 제공합니다.
	virtual void ExecuteNativePattern(APBBossBase* Boss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	// 설정된 텔레그래프 액터들을 보스 기준으로 생성합니다.
	TArray<APBBossPatternTelegraph*> SpawnTelegraph(APBBossBase* Boss);

	// 현재 생성된 텔레그래프 액터들을 제거합니다.
	void DestroySpawnedTelegraphs();
	// 소유 패턴 컴포넌트에서 보스 액터를 가져옵니다.
	APBBossBase* GetOwnerBoss() const;
	// 패턴 실행 중 참조할 소유 보스 액터를 저장합니다.
	void SetOwnerBoss(APBBossBase* Boss);
	AActor* FindPinballActor() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	TArray<FPBBossPatternTelegraphData> TelegraphDataList;

private:
	// 텔레그래프 시간이 끝난 뒤 실제 패턴 실행을 시작합니다.
	void StartExecutePattern();
	// 텔레그래프 대기 타이머를 해제합니다.
	void ClearTelegraphTimer();
	// 설정된 텔레그래프들 중 가장 긴 지속 시간을 반환합니다.
	float GetMaxTelegraphDurationSeconds() const;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossPatternComponent> OwnerPatternComponent;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APBBossPatternTelegraph>> SpawnedTelegraphs;

	FTimerHandle TelegraphTimerHandle;
};
