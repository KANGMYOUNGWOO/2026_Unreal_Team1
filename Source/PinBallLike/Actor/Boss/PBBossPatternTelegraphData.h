#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternTelegraphData.generated.h"

class APBBossPatternTelegraph;

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBossPatternTelegraphData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	TSubclassOf<APBBossPatternTelegraph> TelegraphClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	float DurationSeconds = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	FVector Offset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	FRotator Rotation = FRotator::ZeroRotator;
};
