#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossHitPartComponent.generated.h"

class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EPBBossHitPartType : uint8
{
	Body,
	GroggyPoint,
	WeakPoint
};

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossHitPartComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 보스 피격 부위 컴포넌트의 기본 값을 초기화합니다.
	UPBBossHitPartComponent();

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	// 이 컴포넌트가 나타내는 피격 부위 타입을 반환합니다.
	EPBBossHitPartType GetHitPartType() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	// 이 부위가 연결된 데미지/그로기 포인트 이름을 반환합니다.
	FName GetHitPointName() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	// 이 피격 부위가 사용하는 충돌 컴포넌트를 반환합니다.
	UPrimitiveComponent* GetHitCollisionComponent() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Hit")
	// 전달된 충돌 컴포넌트가 이 피격 부위의 대상인지 확인합니다.
	bool IsTargetHitComponent(const UPrimitiveComponent* PrimitiveComponent) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	EPBBossHitPartType HitPartType = EPBBossHitPartType::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	FName HitPointName = TEXT("Normal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Hit")
	TObjectPtr<UPrimitiveComponent> HitCollisionComponent;
};
