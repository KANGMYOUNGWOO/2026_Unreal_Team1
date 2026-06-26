#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BossInterface.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FBossGroggyPointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy Point", meta = (ClampMin = "0"))
	int32 HPDamageMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy Point", meta = (ClampMin = "0"))
	int32 GroggyAmount = 10;
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void TakeBossDamage(FName GroggyPointName, int32 DamageAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnGroggyTriggered();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnEnragedTriggered();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnDeadTriggered();
};
