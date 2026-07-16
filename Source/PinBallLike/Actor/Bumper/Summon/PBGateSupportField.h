#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBGateSupportField.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

/**
 * 관문 범퍼가 일정 시간 활성화하는 공통 지원 영역입니다.
 * 한 번의 활성화 동안 각 Actor에는 보상을 한 번만 적용해 중복 Overlap 악용을 막습니다.
 */
UCLASS(Blueprintable)
class PINBALLLIKE_API APBGateSupportField : public APBBumperSummonActor
{
	GENERATED_BODY()

public:
	APBGateSupportField();

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

	void ConfigureField(
		EPBBumperRewardType InRewardType,
		FName InResourceName,
		FName InStatusEffectId,
		float InRewardPower,
		float InActiveDuration,
		const FLinearColor& InFieldColor);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<UBoxComponent> FieldArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<UStaticMeshComponent> FieldVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<UPointLightComponent> FieldLight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	EPBBumperRewardType RewardType = EPBBumperRewardType::Resource;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	FName ResourceName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	FName StatusEffectId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	float RewardPower = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	float ActiveDuration = 5.0f;

private:
	TSet<TWeakObjectPtr<AActor>> RewardedActors;
	FTimerHandle ActiveDurationTimerHandle;

	void SetFieldActive(bool bNewActive);
	bool TryApplyReward(AActor* TargetActor);
	void HandleActiveDurationFinished();

	UFUNCTION()
	void HandleFieldBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
