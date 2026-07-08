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
	// 텔레그래프 액터의 기본 컴포넌트를 생성합니다.
	APBBossPatternTelegraph();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	// 지정된 시간과 스케일로 텔레그래프를 표시합니다.
	void InitTelegraph(float InDurationSeconds, const FVector& InScale);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Telegraph")
	// 텔레그래프 액터를 제거합니다.
	void DestroyTelegraph();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Pattern|Telegraph")
	TObjectPtr<USceneComponent> RootSceneComponent;
};
