#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBBossStatusViewModel.generated.h"

class APBBossBase;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBossStatusViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Status ViewModel")
	void SetBoss(APBBossBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Status ViewModel")
	void ClearBoss();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Status ViewModel")
	FText BossNameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Status ViewModel")
	FText HPText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Status ViewModel")
	float HPPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Status ViewModel")
	float GroggyPercent = 0.0f;

protected:
	virtual void BeginDestroy() override;

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
