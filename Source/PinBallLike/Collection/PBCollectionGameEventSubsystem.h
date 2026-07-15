#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBCollectionGameEventSubsystem.generated.h"

/**
 * 이전 Ball 소유 기반 도감 해금 연동 클래스입니다.
 * Blueprint 직렬화 호환을 위해 클래스를 유지하지만 항상 공개형 도감에서는 이벤트를 구독하지 않습니다.
 */
UCLASS()
class PINBALLLIKE_API UPBCollectionGameEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 항상 공개형 도감에서는 아무 작업도 하지 않는 이전 호환 API입니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Integration", meta = (DeprecatedFunction, DeprecationMessage = "도감은 Ball 소유 여부와 무관하게 모든 항목을 표시합니다."))
	void SynchronizeCurrentBallOwnership();
};
