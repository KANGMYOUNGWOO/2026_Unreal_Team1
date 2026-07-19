#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PBOctopusTentacle.generated.h"

class APBOctopusBoss;
class UPBBossHitEffectComponent;
class USceneComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FPBOctopusTentacleHPChangedSignature,
	int32, CurrentTentacleHP,
	int32, MaxTentacleHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBOctopusTentacleStateChangedSignature);

UCLASS()
class PINBALLLIKE_API APBOctopusTentacle : public AActor, public IBossInterface
{
	GENERATED_BODY()

public:
	APBOctopusTentacle();

	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool DamageToBoss_Implementation(int32 DamageAmount) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle")
	void InitializeTentacle(APBOctopusBoss* NewOwnerBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle|HP")
	void ApplyTentacleDamage(int32 DamageAmount);

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle|HP")
	bool IsTentacleAvailable() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle|HP")
	int32 GetCurrentTentacleHP() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle|HP")
	int32 GetMaxTentacleHP() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle|Slam")
	void StartSlam(FVector NewSlamDirection, float SlamDuration);

	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus Tentacle|Slam")
	void FinishSlam();

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle|Slam")
	bool IsSlamActive() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle|Slam")
	FVector GetSlamDirection() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle|Slam")
	float GetSlamProgress() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle|Slam")
	float GetSlamControlRigWeight() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus Tentacle")
	USkeletalMeshComponent* GetTentacleMesh() const;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Octopus Tentacle|HP")
	FPBOctopusTentacleHPChangedSignature OnTentacleHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Octopus Tentacle|HP")
	FPBOctopusTentacleStateChangedSignature OnTentacleDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Octopus Tentacle|HP")
	FPBOctopusTentacleStateChangedSignature OnTentacleRegenerated;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Octopus Tentacle|HP")
	void BP_OnTentacleDestroyed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Octopus Tentacle|HP")
	void BP_OnTentacleRegenerated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Octopus Tentacle|Slam")
	void BP_OnSlamStarted(FVector NewSlamDirection, float SlamDuration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Octopus Tentacle|Slam")
	void BP_OnSlamFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle")
	TObjectPtr<USkeletalMeshComponent> TentacleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle")
	TObjectPtr<UPBBossHitEffectComponent> HitEffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Tentacle|HP", meta = (ClampMin = "1"))
	int32 MaxTentacleHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle|HP")
	int32 CurrentTentacleHP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Tentacle|HP", meta = (ClampMin = "0.0"))
	float RegenerationDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Tentacle|HP", meta = (ClampMin = "0"))
	int32 DestroyedGroggyAmount = 25;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle|HP")
	bool IsTentacleDestroyed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle|Slam")
	bool IsSlamActiveValue = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle|Slam")
	FVector SlamDirection = FVector::ForwardVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Tentacle|Slam")
	float SlamProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Tentacle|Slam", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxSlamControlRigWeight = 1.0f;

private:
	void DestroyTentacle();
	void RegenerateTentacle();
	void SetTentacleActive(bool IsActive);

	UPROPERTY(Transient)
	TObjectPtr<APBOctopusBoss> OwnerBoss;

	FTimerHandle RegenerationTimerHandle;
	float SlamStartTime = 0.0f;
	float SlamDurationSeconds = 0.0f;
};
