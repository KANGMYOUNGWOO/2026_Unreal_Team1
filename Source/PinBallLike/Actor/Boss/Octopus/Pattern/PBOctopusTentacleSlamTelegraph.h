#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternTelegraph.h"
#include "PBOctopusTentacleSlamTelegraph.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBOctopusTentacleSlamTelegraph : public APBBossPatternTelegraph
{
	GENERATED_BODY()

public:
	APBOctopusTentacleSlamTelegraph();
	virtual void Tick(float DeltaSeconds) override;

	void InitSlamTelegraph(
		USceneComponent* NewStartComponent,
		AActor* NewTargetActor,
		double Length,
		double Width,
		float DurationSeconds);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus Pattern|Slam Telegraph")
	TObjectPtr<UStaticMeshComponent> TelegraphMesh;

private:
	void UpdateTrackedTransform();

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> StartComponent;

	double TelegraphLength = 0.0;
};
