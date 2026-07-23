#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBCombatCameraTrackingComponent.generated.h"

class APBBallBase;
class APBBossBase;
class APBCombatPartyController;
class UCameraComponent;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSkillFocusCameraSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Focus",
		meta = (ClampMin = "0.0", Units = "s"))
	float BlendInDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Focus",
		meta = (ClampMin = "0.0", Units = "s"))
	float HoldDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Focus",
		meta = (ClampMin = "0.0", Units = "s"))
	float BlendOutDuration = 0.14f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Focus",
		meta = (ClampMin = "0.0"))
	float FieldOfViewOffset = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Focus",
		meta = (ClampMin = "0.0"))
	float FollowInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Focus",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float MaximumFollowSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Focus",
		meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float GlobalTimeDilation = 0.1f;
};

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

	bool PlaySkillFocus(APBBallBase* TargetBall);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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
	void UpdateSkillFocus();
	void BeginSkillFocusSlowMotion();
	void EndSkillFocusSlowMotion();
	void EndSkillFocus();
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Camera|Skill Focus",
		meta = (AllowPrivateAccess = "true"))
	FPBSkillFocusCameraSettings SkillFocusSettings;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat Camera|Debug",
		meta = (AllowPrivateAccess = "true"))
	EPBCombatCameraTrackingMode CurrentTrackingMode = EPBCombatCameraTrackingMode::None;

	TWeakObjectPtr<APBCombatPartyController> CachedPartyController;
	TWeakObjectPtr<APBBallBase> CachedFallbackBall;
	TWeakObjectPtr<APBBossBase> CachedBoss;
	TWeakObjectPtr<APBBallBase> SkillFocusBall;
	TWeakObjectPtr<UCameraComponent> CachedCameraComponent;

	FVector TrackingOrigin = FVector::ZeroVector;
	FVector NormalizedTrackingAxis = FVector::ForwardVector;
	float TrackingOriginCoordinate = 0.0f;
	float SourceRefreshElapsedTime = 0.0f;
	float SkillFocusElapsedTime = 0.0f;
	float SkillFocusBaseFieldOfView = 0.0f;
	float SkillFocusPreviousGlobalTimeDilation = 1.0f;
	double LastSkillFocusRealTime = -1.0;
	bool bSkillFocusActive = false;
	bool bSkillFocusSlowMotionActive = false;
};
