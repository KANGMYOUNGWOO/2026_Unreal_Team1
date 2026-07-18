#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperVfxRuntimeComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 대상 Actor에 부착되는 범퍼 지속 VFX의 생성·교체·종료만 담당합니다.
 * 실제 효과 수치와 판정은 기존 Effect/Component가 소유하고, 이 컴포넌트는 시각 수명만 따라갑니다.
 */
UCLASS(Transient)
class PINBALLLIKE_API UPBBumperVfxRuntimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperVfxRuntimeComponent();

	static UPBBumperVfxRuntimeComponent* FindOrAddToActor(AActor* TargetActor);

	/** 같은 Channel의 연출은 교체하며, Duration이 0이면 대상 Actor가 제거될 때까지 유지합니다. */
	UNiagaraComponent* PlayAttached(
		FName Channel,
		UNiagaraSystem* System,
		float Duration,
		const FVector& RelativeLocation = FVector::ZeroVector,
		const FVector& Scale = FVector::OneVector);

	void StopChannel(FName Channel);
	void StopAll();

	static void PlayImpact(
		const UObject* WorldContext,
		UNiagaraSystem* System,
		const AActor* TargetActor,
		const FVector& WorldOffset = FVector::ZeroVector,
		const FVector& Scale = FVector::OneVector);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	TMap<FName, TObjectPtr<UNiagaraComponent>> ActiveComponents;
	TMap<FName, FTimerHandle> ExpirationTimers;
};
