#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "PinBallLike/Actor/Bumper/Summon/PBBumperSummonActor.h"
#include "PinBallLike/Struct/Bumper/PBBumperRewardTypes.h"
#include "PBGateSupportField.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UPrimitiveComponent;
class USphereComponent;
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
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void StartActionForActor(
		APBModularBumperBase* Bumper,
		AActor* InteractionActor) override;
	virtual void DeactivateSummon() override;

	void ConfigureField(
		EPBBumperRewardType InRewardType,
		FName InResourceName,
		FName InStatusEffectId,
		FName InTimedEffectSourceId,
		FName InTimedStatName,
		float InRewardPower,
		int32 InTriggerCount,
		float InActiveDuration,
		const FLinearColor& InFieldColor);

	UFUNCTION(BlueprintPure, Category = "Bumper|Support Field")
	float GetFieldRadius() const { return FieldRadius; }

	/** 디버그 표시에서 영역이 어느 Trigger를 기준으로 배치됐는지 보여줍니다. */
	void SetDebugTriggerOrigin(const FVector& InTriggerOrigin);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<UBoxComponent> FieldArea;

	/** 기존 FieldArea Box는 BP 호환용으로 보존하고, 실제 원형 판정은 이 컴포넌트가 담당합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bumper|Support Field")
	TObjectPtr<USphereComponent> RadialFieldArea;

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
	FName TimedEffectSourceId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	FName TimedStatName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	float RewardPower = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	int32 TriggerCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Support Field")
	float ActiveDuration = 5.0f;

	/** 회복·반응형 수리·마나 영역이 공유하는 실제 효과 반경입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Support Field",
		meta = (ClampMin = "50.0", ClampMax = "800.0", Units = "cm"))
	float FieldRadius = PBGateFieldTuning::DefaultRadius;

private:
	TSet<TWeakObjectPtr<AActor>> RewardedActors;
	FTimerHandle ActiveDurationTimerHandle;
	FVector DebugTriggerOrigin = FVector::ZeroVector;
	FColor DebugFieldColor = FColor::Green;
	bool bHasDebugTriggerOrigin = false;

	void SetFieldActive(bool bNewActive);
	void RefreshFieldGeometry();
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
