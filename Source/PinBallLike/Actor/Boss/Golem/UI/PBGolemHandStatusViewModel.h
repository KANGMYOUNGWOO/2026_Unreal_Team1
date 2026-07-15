#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBGolemHandStatusViewModel.generated.h"

class APBGolemBoss;
class APBGolemBossHand;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBGolemHandStatusViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand ViewModel")
	void SetGolemBoss(APBGolemBoss* NewGolemBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Golem Hand ViewModel")
	void ClearGolemBoss();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Golem Hand ViewModel")
	float LeftHandHPPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Golem Hand ViewModel")
	float RightHandHPPercent = 0.0f;

protected:
	virtual void BeginDestroy() override;

private:
	UFUNCTION()
	void HandleGolemHandsChanged();

	UFUNCTION()
	void HandleLeftHandHPChanged(int32 CurrentHandHP, int32 MaxHandHP);

	UFUNCTION()
	void HandleRightHandHPChanged(int32 CurrentHandHP, int32 MaxHandHP);

	void BindGolemHandEvents();
	void UnbindGolemHandEvents();
	void RefreshGolemHandStatus();
	static float CalculateHPPercent(int32 CurrentHP, int32 MaxHP);

	UPROPERTY(Transient)
	TObjectPtr<APBGolemBoss> GolemBoss;

	UPROPERTY(Transient)
	TObjectPtr<APBGolemBossHand> LeftGolemHand;

	UPROPERTY(Transient)
	TObjectPtr<APBGolemBossHand> RightGolemHand;
};
