#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PinBallLike/Struct/Relic/PBRelicTriggerContext.h"
#include "PBRelicTriggerService.generated.h"

class UPBRelicSubsystem;
class UPBTableDataSubsystem;
class UPBStatusEffectComponent;

UCLASS()
class PINBALLLIKE_API UPBRelicTriggerService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(
		UPBRelicSubsystem* InRelicSubsystem,
		UPBTableDataSubsystem* InTableSubsystem);

	void NotifyRelicEvent(
		FName TriggerEvent,
		const FPBRelicTriggerContext& Context);

private:
	void HandleRelicTrigger(
		FName RelicId,
		FName TriggerEvent,
		const FPBRelicTriggerContext& Context);

	UPBStatusEffectComponent* FindStatusEffectComponent(
		AActor* TargetActor) const;

private:
	UPROPERTY()
	TObjectPtr<UPBRelicSubsystem> RelicSubsystem;

	UPROPERTY()
	TObjectPtr<UPBTableDataSubsystem> TableSubsystem;
};