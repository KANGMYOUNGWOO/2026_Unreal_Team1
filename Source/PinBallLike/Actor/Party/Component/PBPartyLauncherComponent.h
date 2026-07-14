// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBPartyLauncherComponent.generated.h"

class APBBallBase;
class UStaticMeshComponent;

struct FPBPartyLauncherDependencies
{
	TFunction<const TArray<TObjectPtr<APBBallBase>>&()> GetPartyBalls;
	TFunction<APBBallBase*()> GetLeaderBall;
	TFunction<void()> RefreshPartyOrder;
	TFunction<void()> ResetPartyDeathState;
	TFunction<void()> BindPartyDeathEvents;
	TFunction<void(bool)> SetSnakeFormationActive;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBPartyLauncherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBPartyLauncherComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeDependencies(FPBPartyLauncherDependencies InDependencies);

	UFUNCTION(BlueprintCallable, Category = "Party|Launch")
	void SetLauncherVisualComponent(UStaticMeshComponent* InLauncherVisualComponent);

	void SetLauncherActive(bool bNewLauncherActive);
	bool IsLauncherActive() const { return bLauncherActive; }
	bool LaunchPartyFromReadyPosition();
	void HidePartyBallsForLaunchReady() const;
	void PlacePartyBallsAtLauncher() const;

private:
	const TArray<TObjectPtr<APBBallBase>>& GetPartyBalls() const;
	APBBallBase* GetLeaderBall() const;
	void ResetPartyDeathState() const;
	void BindPartyDeathEvents() const;
	void RefreshPartyOrder() const;
	void SetSnakeFormationActive(bool bActive) const;
	void CacheLauncherBaseLocation();
	void SetLauncherVisualActive(bool bVisible) const;
	FVector ResolveHorizontalMoveAxis() const;
	FVector ResolveHorizontalSpacingAxis() const;

	FPBPartyLauncherDependencies Dependencies;

	UPROPERTY(EditAnywhere, Category = "Party|Launch")
	bool bLauncherActive = true;

	UPROPERTY(EditAnywhere, Category = "Party|Launch", meta = (ClampMin = "0", ClampMax = "200"))
	float LauncherMoveHalfRange = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Launch", meta = (ClampMin = "0", ClampMax = "1"))
	float LauncherMoveSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Party|Launch")
	FVector LauncherMoveLocalAxis = FVector::RightVector;

	UPROPERTY(EditAnywhere, Category = "Party|Launch", meta = (ClampMin = "0"))
	float ReadyBallSpacing = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Party|Launch", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> LauncherVisualComponent;

	TArray<TObjectPtr<APBBallBase>> EmptyPartyBalls;
	FVector LauncherBaseLocation = FVector::ZeroVector;
	float LauncherElapsedTime = 0.0f;
};
