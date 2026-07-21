#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperVfxRuntimeComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(Transient)
class PINBALLLIKE_API UPBBumperVfxRuntimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperVfxRuntimeComponent();

	static UPBBumperVfxRuntimeComponent* FindOrAddToActor(AActor* TargetActor);

	UNiagaraComponent* PlayAttached(
		FName Channel,
		UNiagaraSystem* System,
		float Duration,
		const FVector& RelativeLocation = FVector::ZeroVector,
		const FVector& Scale = FVector::OneVector);

	void StopChannel(FName Channel);
	void StopAll();

	static UNiagaraComponent* PlayOneShotAtLocation(
		const UObject* WorldContext,
		UNiagaraSystem* System,
		const FVector& WorldLocation,
		const FRotator& WorldRotation = FRotator::ZeroRotator,
		const FVector& Scale = FVector::OneVector,
		float SafetyLifetime = 2.0f);

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
