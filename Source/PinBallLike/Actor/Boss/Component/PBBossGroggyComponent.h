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

	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy")
	void ApplyGroggyDamage(FName GroggyPointName);

	UFUNCTION(BlueprintCallable, Category = "Boss|Groggy")
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
	int32 GetGroggyAmount(FName GroggyPointName) const;
	bool CanNotifyOwner() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwnerActor;
};
