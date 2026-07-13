#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBCollectionGameEventSubsystem.generated.h"

class UPBBallDeckSubsystem;
class UPBCollectionSubsystem;

/**
 * 게임 시스템의 안정적인 식별자 이벤트를 도감 진행도 API로 변환합니다.
 * 도감 데이터나 진행도를 직접 소유하지 않고, 팀원 시스템과 도감 사이의 연결만 담당합니다.
 */
UCLASS()
class PINBALLLIKE_API UPBCollectionGameEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 현재 덱에 배치된 Ball 소유 정보를 다시 읽어 도감 해금 상태와 맞춥니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Integration")
	void SynchronizeCurrentBallOwnership();

private:
	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleBenchSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleCollectionDataReady(bool bIsReady);

	void UnlockBallByInstanceId(int32 BallInstanceId);
	void UnlockBallBySourceId(FName BallId);

	UPROPERTY(Transient)
	TObjectPtr<UPBCollectionSubsystem> CollectionSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> BallDeckSubsystem;

	/** 도감 데이터 준비 전에 획득한 Ball을 로드 완료 후 처리하기 위한 대기 집합입니다. */
	TSet<FName> PendingBallIds;
};
