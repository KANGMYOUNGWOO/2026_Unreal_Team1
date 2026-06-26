#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "TimerManager.h"
#include "UObject/ObjectKey.h"
#include "PBBossBase.generated.h"

class UPBBossGroggyComponent;
class UPBBossPatternComponent;
class UPBBossStatComponent;
class UPBBossStatusWidget;
class UPBBossWeaknessComponent;
class UStateTreeComponent;
class UPrimitiveComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EPBBossState : uint8
{
	Idle,
	Pattern,
	Groggy,
	Enraged,
	Dead
};

UCLASS()
class PINBALLLIKE_API APBBossBase : public APawn, public IBossInterface
{
	GENERATED_BODY()

public:
	APBBossBase();

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	UPBBossStatComponent* GetBossStatComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	UPBBossGroggyComponent* GetBossGroggyComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	UPBBossPatternComponent* GetBossPatternComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	UPBBossWeaknessComponent* GetBossWeaknessComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	UStateTreeComponent* GetBossStateTreeComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void SetBossState(EPBBossState NewBossState);

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	EPBBossState GetBossState() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Profile")
	FText GetBossName() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	void SetPinballCollisionDamageBlocked(bool IsBlocked);

	UFUNCTION(BlueprintPure, Category = "Boss|Damage")
	bool IsPinballCollisionDamageBlocked() const;

	virtual void TakeBossDamage_Implementation(FName GroggyPointName, int32 DamageAmount) override;
	virtual void OnGroggyTriggered_Implementation() override;
	virtual void OnEnragedTriggered_Implementation() override;
	virtual void OnDeadTriggered_Implementation() override;

	bool IsDead() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	void BindBossCollisionEvents();
	void CreateBossStatusWidget();
	void RemoveBossStatusWidget();
	void StartGroggyResetTimer();
	void ClearGroggyResetTimer();
	void HandleGroggyDurationFinished();
	void SetWeaknessState(bool IsOpen);
	void SetWeaknessCollisionEnabled(bool IsEnabled);
	bool IsWeaknessCollisionComponent(const UPrimitiveComponent* PrimitiveComponent) const;
	bool IsWeaknessHitBlocked(FName HitPointName) const;
	bool CanApplyBossDamage(FName GroggyPointName, int32 DamageAmount) const;
	bool CanApplyDamageRateLimit(AActor* OtherActor) const;
	bool IsValidDamageSource(AActor* OtherActor, UPrimitiveComponent* OtherComponent) const;
	int32 GetPinballHitDamage(AActor* OtherActor) const;
	void RecordDamageRateLimit(AActor* OtherActor);
	void ApplyPinballHitImpulse(AActor* OtherActor, const FHitResult& Hit) const;
	void AddPinballCombo(AActor* OtherActor) const;
	FName ResolveGroggyPointName(UPrimitiveComponent* HitComponent) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossStatComponent> BossStatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossGroggyComponent> BossGroggyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossPatternComponent> BossPatternComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossWeaknessComponent> BossWeaknessComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UStateTreeComponent> BossStateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	EPBBossState BossState = EPBBossState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Profile")
	FText BossName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy")
	FName DefaultGroggyPointName = TEXT("Normal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy", meta = (ClampMin = "0.1"))
	float GroggyDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage")
	FName DamageSourceTagName = TEXT("pinball");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Damage")
	bool IsPinballCollisionDamageBlockedValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	int32 MaxDamageCountPerFrame = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	float SameSourceHitCooldownSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	float PinballHitImpulseStrength = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	TSubclassOf<UPBBossStatusWidget> BossStatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI", meta = (ClampMin = "0"))
	int32 BossStatusWidgetZOrder = 0;

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	void BP_OnDamaged(FName GroggyPointName, int32 DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	void BP_OnGroggyStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	void BP_OnEnragedStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	void BP_OnDead();

private:
	FTimerHandle GroggyResetTimerHandle;
	TMap<UPrimitiveComponent*, ECollisionEnabled::Type> WeaknessCollisionEnabledMap;
	TMap<TObjectKey<AActor>, float> LastDamageTimeMap;
	uint64 LastDamageFrameNumber = 0;
	int32 CurrentFrameDamageCount = 0;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossStatusWidget> BossStatusWidget;
};
