#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternTelegraph.h"
#include "PBBossChargeTelegraph.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PINBALLLIKE_API APBBossChargeTelegraph : public APBBossPatternTelegraph
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	// 돌진 방향과 길이에 맞춰 경고 텔레그래프를 초기화합니다.
	void InitChargeTelegraph(
		float InDurationSeconds,
		const FVector& StartLocation,
		const FVector& Direction,
		float Length,
		const FVector& InScale);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	void UpdateChargeTelegraphTransform(const FVector& StartLocation, const FVector& Direction, float Length);
};
