#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BossInterface.generated.h"

class AActor;
class UPrimitiveComponent;

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
	// DamageSource: 보스에게 데미지를 발생시킨 액터입니다.
	// DamageAmount: 보스에게 적용할 기본 데미지 값입니다.
	// HitComponent: 보스 쪽에서 맞은 충돌 컴포넌트입니다.
	// Hit: 충돌 위치, 방향, 노멀 등의 피격 결과 정보입니다.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void DamageToBoss(AActor* DamageSource, int32 DamageAmount, UPrimitiveComponent* HitComponent, const FHitResult& Hit);

	// GroggyAmount: 보스 그로기 게이지에 더할 기본 수치입니다.
	// HitComponent: 그로기 배율을 판정할 보스 쪽 피격 충돌 컴포넌트입니다.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void IncreaseGroggy(int32 GroggyAmount, UPrimitiveComponent* HitComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnGroggyTriggered();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnEnragedTriggered();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss")
	void OnDeadTriggered();
};
