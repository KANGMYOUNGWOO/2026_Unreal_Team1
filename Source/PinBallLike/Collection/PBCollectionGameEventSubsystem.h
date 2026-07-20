#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBCollectionGameEventSubsystem.generated.h"

UCLASS()
class PINBALLLIKE_API UPBCollectionGameEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Collection|Integration", meta = (DeprecatedFunction, DeprecationMessage = "도감은 Ball 소유 여부와 무관하게 모든 항목을 표시합니다."))
	void SynchronizeCurrentBallOwnership();
};
