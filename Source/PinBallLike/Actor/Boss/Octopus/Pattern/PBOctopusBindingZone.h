#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBOctopusBindingZone.generated.h"

class APBBallBase;
class UNiagaraComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBOctopusBindingZone : public AActor
{
	GENERATED_BODY()

public:
	APBOctopusBindingZone();

	void InitializeZone(
		float NewZoneRadius,
		float NewZoneDuration,
		float NewBindDuration,
		int32 NewBindDamage,
		const FVector& NewEffectScale);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Pattern|Binding")
	TObjectPtr<USphereComponent> BindingArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Pattern|Binding")
	TObjectPtr<UNiagaraComponent> BindingEffect;

private:
	void ReleaseBinding();
	void RestoreBoundBallMovement();

	float ZoneDuration = 3.0f;
	float BindDuration = 1.0f;
	int32 BindDamage = 1;
	bool IsBoundBallPreviouslyPaused = false;

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> BoundBall;

	FTimerHandle ZoneDurationTimerHandle;
	FTimerHandle BindDurationTimerHandle;
};
