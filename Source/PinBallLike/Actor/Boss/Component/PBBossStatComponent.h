#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PBBossStatComponent.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPBBossHPChangedSignature, int32, HP, int32, MaxHP);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 보스 스탯 컴포넌트의 기본 값을 초기화합니다.
	UPBBossStatComponent();

	// 게임 시작 시 체력과 소유 액터 참조를 초기화합니다.
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	// 히트 포인트 배율을 반영해 보스 체력 데미지를 적용합니다.
	void ApplyBossDamage(FName HitPointName, int32 DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	// 현재 체력이 0 이하인지 확인합니다.
	bool IsDead() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stat", meta = (ClampMin = "1"))
	int32 MaxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stat")
	int32 HP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool IsEnraged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	int32 DefaultHPDamageMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage")
	TMap<FName, FBossGroggyPointData> HitPointDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|State", meta = (ClampMin = "0", ClampMax = "100"))
	int32 EnrageHPRatioPercent = 40;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Stat")
	FPBBossHPChangedSignature OnHPChanged;

private:
	// 히트 포인트 이름에 해당하는 체력 데미지 배율을 반환합니다.
	int32 GetHPDamageMultiplierPercent(FName HitPointName) const;
	// 현재 체력이 분노 상태 진입 기준에 도달했는지 확인합니다.
	bool IsEnrageThresholdReached() const;
	// 소유 액터에 상태 이벤트를 알릴 수 있는지 확인합니다.
	bool CanNotifyOwner() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwnerActor;
};
