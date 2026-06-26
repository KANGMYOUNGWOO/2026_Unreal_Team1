#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossPinballReactionComponent.generated.h"

class AActor;

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossPinballReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossPinballReactionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Reaction", meta = (ClampMin = "0"))
	float PinballHitImpulseStrength = 500.0f;

private:
	void HandleDamageSourceHitApplied(AActor* DamageSource, const FHitResult& Hit);
	void ApplyPinballHitImpulse(AActor* DamageSource, const FHitResult& Hit) const;
	void AddPinballCombo(AActor* DamageSource) const;
};
