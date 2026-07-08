#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BossInterface.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FBossGroggyPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy Point", meta = (ClampMin = "0"))
	int32 GroggyMultiplierPercent = 100;
};

UINTERFACE(BlueprintType)
class PINBALLLIKE_API UBossInterface : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IBossInterface
{
	GENERATED_BODY()

public:
	// DamageAmount: 보스에게 적용할 기본 데미지 값입니다.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void DamageToBoss(int32 DamageAmount);

	// GroggyAmount: 보스 그로기 게이지에 더할 기본 수치입니다.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void IncreaseGroggy(int32 GroggyAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnGroggyTriggered();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnEnragedTriggered();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnDeadTriggered();
};
