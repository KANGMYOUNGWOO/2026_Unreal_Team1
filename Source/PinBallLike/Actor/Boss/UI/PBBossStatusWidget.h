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
	void SetBoss(APBBossBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|UI")
	void ClearBoss();

protected:
	virtual void NativeOnInitialized() override;
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
	void HandleHPChanged(int32 HP, int32 MaxHP);

	UFUNCTION()
	void HandleGroggyGaugeChanged(int32 GroggyGauge, int32 MaxGroggyGauge);

	void BindBossEvents();
	void UnbindBossEvents();
	void RefreshBossName();
	void RefreshBossStatus();
	static float CalculateGaugePercent(int32 CurrentValue, int32 MaxValue);

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> Boss;
};
