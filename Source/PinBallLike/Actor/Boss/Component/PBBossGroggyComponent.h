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
	UPBBossGroggyComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy Component")
	void ApplyGroggyDamage(int32 GroggyAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy Component")
	void ResetGroggy();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy Component", meta = (ClampMin = "1"))
	int32 MaxGroggyGauge = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Groggy Component")
	int32 GroggyGauge = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Groggy State")
	bool IsGroggy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy Component", meta = (ClampMin = "0"))
	int32 DefaultGroggyMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy Component")
	TMap<FName, FBossGroggyPointData> GroggyPointDataMap;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Groggy Component")
	FPBBossGroggyGaugeChangedSignature OnGroggyGaugeChanged;

private:
	int32 CalculateGroggyAmount(FName GroggyPointName, int32 GroggyAmount) const;
	int32 GetGroggyMultiplierPercent(FName GroggyPointName) const;
	int32 GetDisplayedGroggyGauge() const;
	void RefreshDisplayedGroggyGauge();
	bool CanNotifyOwner() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwnerActor;

	int32 GroggyGaugeRaw = 0;
	int32 MaxGroggyGaugeRaw = 0;
};
