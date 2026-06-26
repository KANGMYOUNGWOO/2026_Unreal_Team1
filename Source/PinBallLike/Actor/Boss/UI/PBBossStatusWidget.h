#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBossStatusWidget.generated.h"

class APBBossBase;
class UProgressBar;
class UTextBlock;

UCLASS()
class PINBALLLIKE_API UPBBossStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|UI")
	// UI가 표시할 보스 참조를 설정하고 이벤트를 바인딩합니다.
	void SetBoss(APBBossBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|UI")
	// 현재 보스 참조와 이벤트 바인딩을 해제합니다.
	void ClearBoss();

protected:
	// 위젯 초기화 시 내부 상태를 준비합니다.
	virtual void NativeOnInitialized() override;
	// 위젯 제거 시 보스 이벤트 바인딩을 해제합니다.
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HPProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> GroggyProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HPText;

private:
	UFUNCTION()
	// 보스 체력 변경 이벤트를 받아 HP UI를 갱신합니다.
	void HandleHPChanged(int32 HP, int32 MaxHP);

	UFUNCTION()
	// 보스 그로기 게이지 변경 이벤트를 받아 그로기 UI를 갱신합니다.
	void HandleGroggyGaugeChanged(int32 GroggyGauge, int32 MaxGroggyGauge);

	// 현재 보스의 상태 변경 이벤트에 바인딩합니다.
	void BindBossEvents();
	// 현재 보스의 상태 변경 이벤트 바인딩을 해제합니다.
	void UnbindBossEvents();
	// 보스 이름 텍스트를 현재 보스 정보로 갱신합니다.
	void RefreshBossName();
	// 체력과 그로기 UI를 현재 보스 상태로 갱신합니다.
	void RefreshBossStatus();
	// 현재 값과 최대 값으로 프로그레스바 비율을 계산합니다.
	static float CalculateGaugePercent(int32 CurrentValue, int32 MaxValue);

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> Boss;
};
