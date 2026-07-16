#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBBossTurtleSpinPattern.generated.h"

class APBBallBase;
class APBTurtleBoss;
class UAnimSequence;
class UNiagaraComponent;
class UNiagaraSystem;
class UPrimitiveComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossTurtleSpinPattern : public UPBBossPatternBase
{
	GENERATED_BODY()

public:
	UPBBossTurtleSpinPattern();

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UFUNCTION()
	void HandleSpinBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	void StartSpin();
	void UpdateSpin();
	void StopSpin();
	void CompletePattern();
	void CleanupSpin();
	void ApplySpinDamage(APBBallBase* Ball);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin")
	TObjectPtr<UAnimSequence> HideAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin")
	TObjectPtr<UNiagaraSystem> SpinEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin", meta = (ClampMin = "0.0"))
	float SpinDurationSeconds = 3.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin", meta = (ClampMin = "0"))
	int32 SpinDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin", meta = (ClampMin = "0.0"))
	float SpinBounceVelocity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin", meta = (ClampMin = "0.0"))
	float SpinCollisionRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin", meta = (ClampMin = "0.0"))
	float SpinEffectScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin", meta = (ClampMin = "0.01"))
	float SpinEffectRadiusAtScaleOne = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin")
	float SpinRotationSpeed = 720.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin|Debug")
	bool IsDrawSpinDamageRange = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Turtle|Spin|Debug", meta = (ClampMin = "0.0"))
	float SpinDamageRangeLineThickness = 5.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> SpinCollision;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpinEffectComponent;

	TWeakObjectPtr<APBTurtleBoss> TurtleBoss;
	TSet<TObjectKey<APBBallBase>> DamagedBalls;
	FTimerHandle SpinStartTimerHandle;
	FTimerHandle SpinUpdateTimerHandle;
	FTimerHandle SpinStopTimerHandle;
	FTimerHandle PatternCompleteTimerHandle;
};
