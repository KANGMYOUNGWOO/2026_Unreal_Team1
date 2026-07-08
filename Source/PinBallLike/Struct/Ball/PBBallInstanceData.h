#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Common/PBResourceData.h"
#include "PinBallLike/Struct/Common/PBStatData.h"
#include "PBBallInstanceData.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBBallInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Instance")
	int32 InstanceId = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Instance")
	FName BallId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Instance", meta = (ClampMin = "1", UIMin = "1"))
	int32 StarLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Instance")
	TArray<FPBResourceData> BaseResources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball|Instance")
	TArray<FPBStatData> BaseStats;

	bool IsValid() const
	{
		return InstanceId != INDEX_NONE && !BallId.IsNone();
	}
};
