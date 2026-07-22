#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBumperSoundComponent.generated.h"

class USoundBase;

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBumperSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBumperSoundComponent();

	UFUNCTION(BlueprintCallable, Category = "Bumper|Feedback|Sound")
	void PlayTriggerCountSound(int32 CurrentCount, int32 RequiredCount);

	UFUNCTION(BlueprintCallable, Category = "Bumper|Feedback|Sound")
	void PlayActivationSound();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Sound")
	TObjectPtr<USoundBase> TriggerCountSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Sound")
	TObjectPtr<USoundBase> ActivationSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Sound",
		meta = (ClampMin = "0.01"))
	float MinimumTriggerPitch = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Sound",
		meta = (ClampMin = "0.01"))
	float MaximumTriggerPitch = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Sound",
		meta = (ClampMin = "0.0"))
	float TriggerSoundVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bumper|Feedback|Sound",
		meta = (ClampMin = "0.0"))
	float ActivationSoundVolume = 1.0f;
};
