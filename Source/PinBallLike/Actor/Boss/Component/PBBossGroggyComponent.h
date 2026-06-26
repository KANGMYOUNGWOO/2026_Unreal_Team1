#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PBBossGroggyComponent.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBBossGroggyGaugeChangedSignature, int32, GroggyGauge, int32, MaxGroggyGauge);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossGroggyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 보스 그로기 컴포넌트의 기본 값을 초기화합니다.
	UPBBossGroggyComponent();

	// 게임 시작 시 소유 액터 참조를 캐싱합니다.
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy")
	// 지정 그로기 포인트의 누적 그로기 데미지를 적용합니다.
	void ApplyGroggyDamage(FName GroggyPointName);

	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy")
	// 그로기 게이지와 그로기 상태를 초기화합니다.
	void ResetGroggy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy", meta = (ClampMin = "1"))
	int32 MaxGroggyGauge = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Groggy")
	int32 GroggyGauge = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool IsGroggy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy", meta = (ClampMin = "0"))
	int32 DefaultGroggyAmount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy")
	TMap<FName, FBossGroggyPointData> GroggyPointDataMap;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Groggy")
	FPBBossGroggyGaugeChangedSignature OnGroggyGaugeChanged;

private:
	// 그로기 포인트 이름에 해당하는 그로기 증가량을 반환합니다.
	int32 GetGroggyAmount(FName GroggyPointName) const;
	// 소유 액터에 그로기 이벤트를 알릴 수 있는지 확인합니다.
	bool CanNotifyOwner() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwnerActor;
};
