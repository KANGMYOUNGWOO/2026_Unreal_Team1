#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PinBallLike/Struct/Ball/PBBallVoiceType.h"
#include "PBBallVoiceDataAsset.generated.h"

class USoundBase;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallVoiceSoundList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Voice")
	TArray<TObjectPtr<USoundBase>> DeathSounds;
};

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBBallVoiceDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USoundBase* GetRandomDeathSound(EPBBallVoiceType VoiceType) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Voice")
	TMap<EPBBallVoiceType, FPBBallVoiceSoundList> VoiceSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|Voice", meta = (ClampMin = "0.0"))
	float DeathSoundVolume = 1.0f;
};
