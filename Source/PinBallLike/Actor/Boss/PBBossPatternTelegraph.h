#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBossPatternTelegraph.generated.h"

class USceneComponent;

UCLASS(Blueprintable, BlueprintType)
class PINBALLLIKE_API APBBossPatternTelegraph : public AActor
{
	GENERATED_BODY()

public:
	APBBossPatternTelegraph();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	void InitTelegraph(float InDurationSeconds, const FVector& InScale);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	void DestroyTelegraph();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	TObjectPtr<USceneComponent> RootSceneComponent;

private:
	FTimerHandle DestroyTimerHandle;
};
