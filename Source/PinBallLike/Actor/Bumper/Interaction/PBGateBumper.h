// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Bumper/PBBumperBase.h"
#include "PBGateBumper.generated.h"

class APBBallBase;
class UPrimitiveComponent;

UCLASS()
class PINBALLLIKE_API APBGateBumper : public APBBumperBase
{
	GENERATED_BODY()

public:
	APBGateBumper();

protected:
	virtual void BeginPlay() override;

	// 이 태그를 가진 PrimitiveComponent를 통과 판정 영역으로 자동 등록한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bumper|Gate")
	FName GateAreaTag = TEXT("BumperTrigger");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bumper|Gate")
	TArray<TObjectPtr<UPrimitiveComponent>> GateAreas;

#pragma region Blueprint Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Bumper|Gate")
	void OnGateBumperPassed(APBBallBase* Ball);
#pragma endregion

private:
	TMap<TWeakObjectPtr<APBBallBase>, int32> PassingBallOverlapCounts;
	TMap<TWeakObjectPtr<APBBallBase>, float> PassingBallEntrySides;

	float CalculateGateSide(const UPrimitiveComponent* GateArea, const APBBallBase* Ball) const;
	
	void RegisterTaggedAreas();
	void SetupTriggerArea(UPrimitiveComponent* GateArea);
	
	UFUNCTION()
	void HandleGateBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool IsFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleGateEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
