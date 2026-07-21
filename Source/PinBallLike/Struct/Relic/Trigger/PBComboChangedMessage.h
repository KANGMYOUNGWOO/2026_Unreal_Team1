#pragma once

#include "CoreMinimal.h"
#include "PBComboChangedMessage.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBComboChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 PreviousCombo = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCombo = 0;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> SourceActor = nullptr;
};