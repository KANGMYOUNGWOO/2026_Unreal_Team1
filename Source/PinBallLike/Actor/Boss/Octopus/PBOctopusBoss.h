#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PBOctopusBoss.generated.h"

class APBOctopusTentacle;
class UAnimationAsset;
class UAnimSequence;
class USceneComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBOctopusTentaclesChangedSignature);

UCLASS()
class PINBALLLIKE_API APBOctopusBoss : public APBBossBase
{
	GENERATED_BODY()

public:
	APBOctopusBoss();

	UFUNCTION(BlueprintCallable, Category = "Boss|Octopus")
	void SpawnTentacles();

	UFUNCTION(BlueprintPure, Category = "Boss|Octopus")
	APBOctopusTentacle* GetTentacle(int32 TentacleIndex) const;

	void PlayOctopusAnimation(UAnimationAsset* Animation, bool IsLooping = false);
	void RestoreOctopusAnimationMode();

	void HandleTentacleDestroyed(APBOctopusTentacle* DestroyedTentacle, int32 GroggyAmount);

	UPROPERTY(BlueprintAssignable, Category = "Boss|Octopus")
	FPBOctopusTentaclesChangedSignature OnTentaclesChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void StartIdleState_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Animation")
	TObjectPtr<UAnimSequence> IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus")
	TSubclassOf<APBOctopusTentacle> TentacleClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus")
	TObjectPtr<USkeletalMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus")
	TArray<TObjectPtr<USceneComponent>> TentacleSpawnPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Octopus")
	TArray<TObjectPtr<APBOctopusTentacle>> Tentacles;

private:
	APBOctopusTentacle* SpawnTentacle(USceneComponent* SpawnPoint);
	void DestroyTentacles();
};
