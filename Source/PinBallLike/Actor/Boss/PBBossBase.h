#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PinBallLike/Interface/BossInterface.h"
#include "TimerManager.h"
#include "PBBossBase.generated.h"

class UPBBossGroggyComponent;
class UPBBossDamageComponent;
class UPBBossDamageReceiverComponent;
class UPBBossPatternComponent;
class UPBBossPinballReactionComponent;
class UPBBossStatComponent;
class UPBBossStatusWidget;
class UPBBossUIComponent;
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

UENUM(BlueprintType)
enum class EPBBossMovementType : uint8
{
	Fixed,
	Movable
};

UCLASS()
class PINBALLLIKE_API APBBossBase : public APawn, public IBossInterface
{
	GENERATED_BODY()

public:
	// 보스 기본 컴포넌트들을 생성하고 초기 상태를 설정합니다.
	APBBossBase();

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	// 보스의 체력/상태 수치 컴포넌트를 반환합니다.
	UPBBossStatComponent* GetBossStatComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	// 보스의 그로기 게이지 컴포넌트를 반환합니다.
	UPBBossGroggyComponent* GetBossGroggyComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	// 보스의 데미지 처리 컴포넌트를 반환합니다.
	UPBBossDamageComponent* GetBossDamageComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	// 보스의 패턴 실행 컴포넌트를 반환합니다.
	UPBBossPatternComponent* GetBossPatternComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	// 보스의 약점 상태 컴포넌트를 반환합니다.
	UPBBossWeaknessComponent* GetBossWeaknessComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	// 보스의 StateTree 컴포넌트를 반환합니다.
	UStateTreeComponent* GetBossStateTreeComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Component")
	UPBBossUIComponent* GetBossUIComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	// 보스의 현재 상태를 변경합니다.
	void SetBossState(EPBBossState NewBossState);

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	// 보스의 현재 상태를 반환합니다.
	EPBBossState GetBossState() const;

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsIdleState() const;

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsPatternState() const;

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsGroggyState() const;

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsEnragedState() const;

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsDeadState() const;

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsEnragedPhase() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Type")
	bool IsFixedBoss() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Type")
	bool IsMovableBoss() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|State")
	void StartIdleState();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void StartPatternState();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void StopPatternState();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void StartGroggyState();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void FinishGroggyState();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void StartEnragedState();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void StartDeadState();

	UFUNCTION(BlueprintPure, Category = "Boss|Profile")
	// UI 등에 표시할 보스 이름을 반환합니다.
	FText GetBossName() const;

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	// 핀볼 충돌 데미지를 임시로 막을지 설정합니다.
	void SetPinballCollisionDamageBlocked(bool IsBlocked);

	UFUNCTION(BlueprintPure, Category = "Boss|Damage")
	// 현재 핀볼 충돌 데미지가 차단되어 있는지 반환합니다.
	bool IsPinballCollisionDamageBlocked() const;

	// 보스가 데미지를 받았음을 내부 시스템과 블루프린트에 알립니다.
	void NotifyBossDamaged(FName HitPointName, int32 DamageAmount);

	// BossInterface를 통해 보스 데미지를 적용합니다.
	virtual void TakeBossDamage_Implementation(FName GroggyPointName, int32 DamageAmount) override;
	// BossInterface를 통해 그로기 진입 처리를 실행합니다.
	virtual void OnGroggyTriggered_Implementation() override;
	// BossInterface를 통해 분노 상태 진입 처리를 실행합니다.
	virtual void OnEnragedTriggered_Implementation() override;
	// BossInterface를 통해 사망 처리를 실행합니다.
	virtual void OnDeadTriggered_Implementation() override;

	// 보스가 사망 상태인지 반환합니다.
	bool IsDead() const;

protected:
	// 게임 시작 시 컴포넌트 바인딩과 UI 초기화를 수행합니다.
	virtual void BeginPlay() override;
	// 액터 종료 시 타이머와 UI를 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	// 보스 충돌체가 다른 액터와 충돌했을 때 데미지 처리를 시도합니다.
	void HandleCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	// 보스 충돌 이벤트 델리게이트를 바인딩합니다.
	void BindBossCollisionEvents();
	// 그로기 지속 시간이 끝나도록 타이머를 시작합니다.
	void StartGroggyResetTimer();
	// 그로기 종료 타이머를 해제합니다.
	void ClearGroggyResetTimer();
	// 그로기 지속 시간이 끝났을 때 보스를 정상 상태로 복귀시킵니다.
	void HandleGroggyDurationFinished();
	// 약점 개방 상태를 컴포넌트와 충돌체에 반영합니다.
	void SetWeaknessState(bool IsOpen);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossStatComponent> BossStatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossGroggyComponent> BossGroggyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossDamageComponent> BossDamageComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossDamageReceiverComponent> BossDamageReceiverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossPatternComponent> BossPatternComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossPinballReactionComponent> BossPinballReactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossWeaknessComponent> BossWeaknessComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UStateTreeComponent> BossStateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Component")
	TObjectPtr<UPBBossUIComponent> BossUIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	EPBBossState BossState = EPBBossState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Type")
	EPBBossMovementType BossMovementType = EPBBossMovementType::Fixed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Profile")
	FText BossName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy")
	FName DefaultGroggyPointName = TEXT("Normal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Groggy", meta = (ClampMin = "0.1"))
	float GroggyDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI")
	TSubclassOf<UPBBossStatusWidget> BossStatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI", meta = (ClampMin = "0"))
	int32 BossStatusWidgetZOrder = 0;

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	// 블루프린트에서 보스 피격 연출을 구현하는 이벤트입니다.
	void BP_OnDamaged(FName GroggyPointName, int32 DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	// 블루프린트에서 그로기 시작 연출을 구현하는 이벤트입니다.
	void BP_OnGroggyStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	// 블루프린트에서 분노 시작 연출을 구현하는 이벤트입니다.
	void BP_OnEnragedStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Blueprint Event")
	// 블루프린트에서 사망 연출을 구현하는 이벤트입니다.
	void BP_OnDead();

private:
	FTimerHandle GroggyResetTimerHandle;
};
