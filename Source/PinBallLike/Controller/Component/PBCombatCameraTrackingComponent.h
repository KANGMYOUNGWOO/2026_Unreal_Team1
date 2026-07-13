#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBCombatCameraTrackingComponent.generated.h"

class APBBallBase;
class APBBossBase;
class APBCombatPartyController;

/** 현재 카메라가 어떤 전투 대상을 기준으로 움직이는지 나타냅니다. */
UENUM(BlueprintType)
enum class EPBCombatCameraTrackingMode : uint8
{
	None,
	Ball,
	LaunchReady,
	BallAndBoss
};

/**
 * 전투 카메라 소유 액터를 지정 축으로만 이동시킵니다.
 * 보이는 리더 Ball을 추적하고, Ball이 없을 때는 발사 준비를 위해 최소 좌표로 복귀합니다.
 * Ball과 살아 있는 Boss가 함께 있으면 두 대상의 중간점을 부드럽게 추적합니다.
 */
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBCombatCameraTrackingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBCombatCameraTrackingComponent();

	/** 자동 추적을 켜거나 끕니다. 끄면 현재 카메라 위치를 유지합니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat Camera|Tracking")
	void SetTrackingEnabled(bool bEnabled);

	/** 현재 맵에서 카메라 중심이 이동할 수 있는 최소/최대 좌표를 설정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat Camera|Bounds")
	void SetTrackingBounds(float NewMinimumCoordinate, float NewMaximumCoordinate);

	/** 런타임에 생성되거나 교체된 파티와 Boss를 즉시 다시 찾습니다. */
	UFUNCTION(BlueprintCallable, Category = "Combat Camera|Tracking")
	void RefreshTrackingSources();

	UFUNCTION(BlueprintPure, Category = "Combat Camera|Tracking")
	EPBCombatCameraTrackingMode GetTrackingMode() const { return CurrentTrackingMode; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ResolveDesiredCoordinate(float& OutCoordinate, EPBCombatCameraTrackingMode& OutMode) const;
	APBBallBase* GetTrackableBall() const;
	APBBossBase* GetTrackableBoss() const;
	bool IsTrackableBall(const APBBallBase* Ball) const;
	bool IsTrackableBoss(const APBBossBase* Boss) const;
	void ApplyTrackingCoordinate(float NewCoordinate) const;
	void NormalizeConfiguration();
	void SetCurrentTrackingMode(EPBCombatCameraTrackingMode NewMode);

	/** 현재 맵의 화면상 세로 방향입니다. Lv_Battle에서는 월드 X축입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Tracking",
		meta = (AllowPrivateAccess = "true"))
	FVector TrackingAxis = FVector::ForwardVector;

	/** 카메라 중심이 이동할 수 있는 최소 축 좌표입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Bounds",
		meta = (AllowPrivateAccess = "true"))
	float MinimumTrackingCoordinate = 950.0f;

	/** 카메라 중심이 이동할 수 있는 최대 축 좌표입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Bounds",
		meta = (AllowPrivateAccess = "true"))
	float MaximumTrackingCoordinate = 2000.0f;

	/** Ball과 Boss가 함께 있을 때 Boss 쪽으로 치우치는 비율입니다. 0.5는 정확한 중간점입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Tracking",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float BallBossBlendAlpha = 0.5f;

	/** 목표가 이 거리 안에서 움직일 때 카메라가 반응하지 않도록 하는 완충 영역입니다. 발사 준비 복귀에는 적용하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Smoothing",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm"))
	float TrackingDeadZone = 140.0f;

	/** 목표 좌표로 가까워지는 정도입니다. 값이 클수록 빠르게 반응합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Smoothing",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FollowInterpSpeed = 3.5f;

	/** 급격한 대상 전환에서도 초당 이동량이 이 값을 넘지 않도록 제한합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Smoothing",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm/s"))
	float MaximumFollowSpeed = 800.0f;

	/** 동적 생성 대상을 다시 검색하는 주기입니다. 검색은 매 프레임 실행하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Tracking",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.05", Units = "s"))
	float SourceRefreshInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Tracking",
		meta = (AllowPrivateAccess = "true"))
	bool bTrackingEnabled = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat Camera|Debug",
		meta = (AllowPrivateAccess = "true"))
	EPBCombatCameraTrackingMode CurrentTrackingMode = EPBCombatCameraTrackingMode::None;

	TWeakObjectPtr<APBCombatPartyController> CachedPartyController;
	TWeakObjectPtr<APBBallBase> CachedFallbackBall;
	TWeakObjectPtr<APBBossBase> CachedBoss;

	FVector TrackingOrigin = FVector::ZeroVector;
	FVector NormalizedTrackingAxis = FVector::ForwardVector;
	float TrackingOriginCoordinate = 0.0f;
	float SourceRefreshElapsedTime = 0.0f;
};
