#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "../Struct/GamePlayMessage/PBDragMessage.h"
#include "PBDragTest.generated.h"

UCLASS()
class PINBALLLIKE_API APBDragTest : public AActor
{
	GENERATED_BODY()

public:
	APBDragTest();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleDragStarted(
		FGameplayTag Channel,
		const FPBDragStartedMessage& Message);

	

private:
	FGameplayMessageListenerHandle DragStartedHandle;
	FGameplayMessageListenerHandle DragCancelledHandle;
};