#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBSkillFeedbackComponent.generated.h"

class UCameraShakeBase;
class UAudioComponent;
class USoundBase;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBSkillFeedbackSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Feedback|Sound")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Feedback|Sound", meta = (ClampMin = "0.01"))
	float MinimumPitch = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Feedback|Sound", meta = (ClampMin = "0.01"))
	float MaximumPitch = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Feedback|Camera")
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Feedback|Camera", meta = (ClampMin = "0.0"))
	float CameraShakeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Feedback|Camera",
		meta = (ClampMin = "0.0", Units = "s"))
	float MinimumCameraShakeInterval = 0.08f;
};

UCLASS(ClassGroup = (PinBall), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBSkillFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBSkillFeedbackComponent();

	UFUNCTION(BlueprintCallable, Category = "Skill|Feedback")
	void PlayCastFeedback();

	UFUNCTION(BlueprintCallable, Category = "Skill|Feedback")
	void PlayHitFeedback();

	UFUNCTION(BlueprintCallable, Category = "Skill|Feedback|Sound")
	void PlaySkillSound(USoundBase* Sound, float Volume = 1.0f, float Pitch = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Skill|Feedback|Sound")
	void StopSkillSound();

	UFUNCTION(BlueprintCallable, Category = "Skill|Feedback|Camera")
	bool PlayCameraShake(
		TSubclassOf<UCameraShakeBase> CameraShakeClass,
		float Scale = 1.0f,
		float MinimumInterval = 0.0f);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Feedback")
	FPBSkillFeedbackSettings CastFeedback;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Feedback")
	FPBSkillFeedbackSettings HitFeedback;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Feedback|Sound",
		meta = (ClampMin = "0.0", Units = "s"))
	float SkillSoundFadeOutDuration = 0.2f;

private:
	void PlayFeedback(const FPBSkillFeedbackSettings& FeedbackSettings);

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveSkillAudioComponent;

	double LastCameraShakeTime = -1.0;
};
