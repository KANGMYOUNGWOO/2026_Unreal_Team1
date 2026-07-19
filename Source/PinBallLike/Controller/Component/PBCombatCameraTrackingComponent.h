#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBCombatCameraTrackingComponent.generated.h"

class APBBallBase;
class APBBossBase;
class APBCombatPartyController;

UENUM(BlueprintType)
enum class EPBCombatCameraTrackingMode : uint8
{
	None,
	Ball,
	LaunchReady,
	BallAndBoss
};

UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBCombatCameraTrackingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBCombatCameraTrackingComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat Camera|Tracking")
	void SetTrackingEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Combat Camera|Bounds")
	void SetTrackingBounds(float NewMinimumCoordinate, float NewMaximumCoordinate);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Tracking",
		meta = (AllowPrivateAccess = "true"))
	FVector TrackingAxis = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Bounds",
		meta = (AllowPrivateAccess = "true"))
	float MinimumTrackingCoordinate = 950.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Bounds",
		meta = (AllowPrivateAccess = "true"))
	float MaximumTrackingCoordinate = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Tracking",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float BallBossBlendAlpha = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Smoothing",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm"))
	float TrackingDeadZone = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Smoothing",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float FollowInterpSpeed = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Smoothing",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm/s"))
	float MaximumFollowSpeed = 800.0f;

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
