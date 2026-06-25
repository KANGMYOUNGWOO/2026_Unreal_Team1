#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "PinBallLike/Interface/Damageable.h"
#include "TimerManager.h"
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
class PINBALLLIKE_API APBBossBase : public APawn, public IBossInterface, public IDamageable
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

	virtual void TakeBossDamage_Implementation(FName GroggyPointName, int32 DamageAmount) override;
	virtual void OnGroggyTriggered_Implementation() override;
	virtual void OnEnragedTriggered_Implementation() override;
	virtual void OnDeadTriggered_Implementation() override;

	virtual void TakeDamage(int32 Damage) override;
	virtual bool IsDead() const override;

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
	void SetWeaknessCollisionEnabled(bool IsEnabled);
	bool IsWeaknessCollisionComponent(const UPrimitiveComponent* PrimitiveComponent) const;
	bool IsWeaknessHitBlocked(FName HitPointName) const;
	bool IsValidDamageSource(AActor* OtherActor, UPrimitiveComponent* OtherComponent) const;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	int32 PinballHitDamage = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy")
	FName DefaultGroggyPointName = TEXT("Normal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy", meta = (ClampMin = "0.1"))
	float GroggyDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage")
	FName DamageSourceTagName = TEXT("pinball");

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

	UPROPERTY(Transient)
	TObjectPtr<UPBBossStatusWidget> BossStatusWidget;
};
