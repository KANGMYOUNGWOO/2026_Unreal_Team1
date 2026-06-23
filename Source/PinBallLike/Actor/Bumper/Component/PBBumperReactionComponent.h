// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "PBBumperReactionComponent.generated.h"

class UCurveFloat;
class USceneComponent;

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBumperReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperReactionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Bumper|Reaction")
	void PlayImpactReaction(const FHitResult& Hit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Reaction")
	TObjectPtr<USceneComponent> TargetComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Reaction")
	FName TargetComponentTag = TEXT("BumperVisual");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Reaction", meta = (ClampMin = "0.0", Units = "cm"))
	float PushDistance = 80.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Reaction|Curve")
	TObjectPtr<UCurveFloat> MoveCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bumper|Reaction|Curve")
	TObjectPtr<UCurveFloat> ScaleCurve;

private:
	void InitializeReactionTimeline();
	bool ResolveTargetComponent();
	void CacheBaseTransform();
	void ResetTargetTransform();

	UFUNCTION()
	void HandleMoveTimelineUpdate(float Alpha);

	UFUNCTION()
	void HandleScaleTimelineUpdate(float Alpha);

	UFUNCTION()
	void HandleReactionTimelineFinished();

	FTimeline ReactionTimeline;

	FVector BaseRelativeLocation = FVector::ZeroVector;
	FVector BaseRelativeScale = FVector::OneVector;
	FVector CurrentPushDirection = FVector::ZeroVector;
};
