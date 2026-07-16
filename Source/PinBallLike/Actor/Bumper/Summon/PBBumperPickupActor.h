#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBBumperPickupActor.generated.h"

class UPointLightComponent;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * 생성형 범퍼가 필드에 잠시 배치하는 공통 획득 아이템입니다.
 * 실제 보상 종류와 수치는 Effect Blueprint와 Effect 시트가 주입합니다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API APBBumperPickupActor : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBBumperPickupActor();

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

	void ConfigurePickup(
		EPBBumperRewardType InRewardType,
		FName InResourceName,
		FName InStatusEffectId,
		float InRewardPower,
		float InLifetime,
		const FLinearColor& InRewardColor);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Pickup")
	TObjectPtr<USphereComponent> CollectionArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Pickup")
	TObjectPtr<UStaticMeshComponent> PickupVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Pickup")
	TObjectPtr<UPointLightComponent> PickupLight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Pickup")
	EPBBumperRewardType RewardType = EPBBumperRewardType::Resource;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Pickup")
	FName ResourceName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Pickup")
	FName StatusEffectId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Pickup")
	float RewardPower = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Pickup")
	float Lifetime = 8.0f;

private:
	FTimerHandle LifetimeTimerHandle;
	bool bIsPickupActive = false;

	void SetPickupActive(bool bNewActive);
	void HandleLifetimeExpired();

	UFUNCTION()
	void HandleCollectionAreaBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
