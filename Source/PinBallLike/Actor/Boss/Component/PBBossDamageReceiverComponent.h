#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossDamageReceiverComponent.generated.h"

class APBBossBase;

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossDamageReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossDamageReceiverComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleBossDamageApplied(FName HitPointName, int32 DamageAmount);

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;
};
