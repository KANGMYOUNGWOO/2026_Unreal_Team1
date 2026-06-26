#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternBase.h"
#include "TimerManager.h"
#include "PBBossProjectilePattern.generated.h"

class APBBossProjectile;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossProjectilePattern : public UPBBossPatternBase
{
	GENERATED_BODY()

public:
	// 투사체 패턴의 기본 값을 초기화합니다.
	UPBBossProjectilePattern();

protected:
	// 투사체 클래스와 실행 조건을 확인합니다.
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	// 투사체 발사 패턴을 시작하고 발사 타이머를 설정합니다.
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;
	// 투사체 패턴 취소 시 발사 타이머를 정리합니다.
	virtual void CancelPatternInternal_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile")
	TSubclassOf<APBBossProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float ProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	int32 ProjectileCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (ClampMin = "0"))
	float FireIntervalSeconds = 0.2f;

private:
	// 설정된 위치와 방향으로 투사체 하나를 발사합니다.
	void FireProjectile();
	// 반복 발사 타이머를 해제합니다.
	void ClearFireTimer();
	// 보스 위치와 오프셋을 기준으로 투사체 생성 위치를 계산합니다.
	FVector GetProjectileSpawnLocation() const;
	// 핀볼 방향을 바라보도록 투사체 생성 회전을 계산합니다.
	FRotator GetProjectileSpawnRotation(const FVector& SpawnLocation) const;
	// 월드에서 핀볼 액터를 찾아 반환합니다.
	AActor* FindPinballActor() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile", meta = (AllowPrivateAccess = "true"))
	FVector SpawnOffset = FVector::ZeroVector;

	int32 FiredProjectileCount = 0;
	FTimerHandle FireTimerHandle;
};
