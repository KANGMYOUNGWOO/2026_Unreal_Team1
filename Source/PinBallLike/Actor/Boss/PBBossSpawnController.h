// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBossSpawnController.generated.h"

class APBBossBase;
class USceneComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBossSpawnController : public AActor
{
	GENERATED_BODY()

public:
	APBBossSpawnController();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Spawn")
	void SpawnBossAsync();

	UFUNCTION(BlueprintCallable, Category = "Boss|Spawn")
	void ClearSpawnedBoss();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void CompleteBossPreparation(bool IsSuccess) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Spawn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Spawn", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APBBossBase> BossClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Spawn", meta = (AllowPrivateAccess = "true"))
	bool IsSpawnOnBeginPlay = false;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> SpawnedBoss;
};
