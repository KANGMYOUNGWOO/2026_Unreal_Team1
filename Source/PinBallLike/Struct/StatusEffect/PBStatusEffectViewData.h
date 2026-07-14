#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "PBStatusEffectViewData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBStatusEffectViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|View")
	FName StatusEffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|View", meta = (ClampMin = "0"))
	int32 StackCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect|View")
	TObjectPtr<UTexture2D> Icon = nullptr;

	bool IsValid() const
	{
		return !StatusEffectId.IsNone();
	}
};
