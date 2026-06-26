#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossWeaknessComponent.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FBossWeaknessData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Weakness", meta = (ClampMin = "0"))
	int32 HPDamageMultiplierPercent = 200;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBBossWeaknessOpenChangedSignature, bool, IsWeaknessOpen);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossWeaknessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 보스 약점 컴포넌트의 기본 값을 초기화합니다.
	UPBBossWeaknessComponent();

	UFUNCTION(BlueprintCallable, Category = "Boss|Weakness")
	// 약점 상태를 열고 변경 이벤트를 알립니다.
	void OpenWeakness();

	UFUNCTION(BlueprintCallable, Category = "Boss|Weakness")
	// 약점 상태를 닫고 변경 이벤트를 알립니다.
	void CloseWeakness();

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	// 지정한 이름이 등록된 약점 포인트인지 확인합니다.
	bool IsWeaknessPoint(FName WeaknessPointName) const;

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	// 지정한 약점 포인트가 현재 데미지를 받을 수 있는지 확인합니다.
	bool IsWeaknessPointOpen(FName WeaknessPointName) const;

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	// 약점 배율을 반영한 최종 데미지를 계산합니다.
	int32 CalculateWeaknessDamage(FName WeaknessPointName, int32 DamageAmount) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Weakness")
	bool IsWeaknessOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Weakness")
	TMap<FName, FBossWeaknessData> WeaknessPointDataMap;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Weakness")
	FPBBossWeaknessOpenChangedSignature OnWeaknessOpenChanged;
};
