#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectKey.h"
#include "PBBossHitPartComponent.h"
#include "PBBossDamageComponent.generated.h"

class APBBossBase;
class UPrimitiveComponent;

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 보스 데미지 컴포넌트의 기본 값을 초기화합니다.
	UPBBossDamageComponent();

	// 게임 시작 시 소유 보스 참조를 캐싱합니다.
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	// 범퍼 스킬 등 외부 데미지 소스로 보스에게 데미지를 적용합니다. //볼은 제외
	void DamageToBose(AActor* DamageSource, int32 DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	// 지정한 히트 포인트 이름으로 보스 데미지를 적용합니다.
	void ApplyPointDamage(FName HitPointName, int32 DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	// 맞은 부위 컴포넌트를 해석해 보스에게 데미지를 적용합니다.
	void ApplyHitPartDamage(AActor* DamageSource, UPrimitiveComponent* HitComponent, int32 DamageAmount, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	// 핀볼 충돌 데미지를 임시로 차단할지 설정합니다.
	void SetPinballCollisionDamageBlocked(bool IsBlocked);

	UFUNCTION(BlueprintPure, Category = "Boss|Damage")
	// 핀볼 충돌 데미지가 현재 차단되어 있는지 반환합니다.
	bool IsPinballCollisionDamageBlocked() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Damage")
	// 입력된 액터와 컴포넌트가 유효한 데미지 소스인지 확인합니다.
	bool IsValidDamageSource(AActor* DamageSource, UPrimitiveComponent* DamageSourceComponent) const;

	UFUNCTION(BlueprintPure, Category = "Boss|Damage")
	// 핀볼 액터가 충돌 시 줄 데미지 값을 반환합니다.
	int32 GetPinballHitDamage(AActor* DamageSource) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage")
	FName DefaultHitPointName = TEXT("Normal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage")
	FName DamageSourceTagName = TEXT("pinball");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Damage")
	bool IsPinballCollisionDamageBlockedValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	int32 MaxDamageCountPerFrame = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	float SameSourceHitCooldownSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Damage", meta = (ClampMin = "0"))
	float PinballHitImpulseStrength = 500.0f;

private:
	struct FPBBossHitPartInfo
	{
		EPBBossHitPartType HitPartType = EPBBossHitPartType::Body;
		FName HitPointName = TEXT("Normal");
	};

	// 충돌한 컴포넌트가 어떤 보스 피격 부위인지 해석합니다.
	FPBBossHitPartInfo ResolveHitPartInfo(const UPrimitiveComponent* HitComponent) const;
	// 충돌 컴포넌트 또는 부모에서 보스 피격 부위 컴포넌트를 찾습니다.
	const UPBBossHitPartComponent* FindHitPartComponent(const UPrimitiveComponent* HitComponent) const;
	// 데미지 이름과 수치가 실제 적용 가능한 값인지 확인합니다.
	bool CanApplyDamage(FName HitPointName, int32 DamageAmount) const;
	// 같은 프레임 또는 같은 소스의 과도한 데미지를 제한합니다.
	bool CanApplyDamageRateLimit(AActor* DamageSource) const;
	// 닫힌 약점 부위에 대한 데미지 적용을 막아야 하는지 확인합니다.
	bool IsWeakPointHitBlocked(const FPBBossHitPartInfo& HitPartInfo) const;
	// 해석된 피격 부위 정보를 바탕으로 최종 데미지 처리를 수행합니다.
	void ApplyResolvedDamage(AActor* DamageSource, const FPBBossHitPartInfo& HitPartInfo, int32 DamageAmount, const FHitResult& Hit);
	// 소유 보스에게 지정 히트 포인트 데미지를 전달합니다.
	void ApplyDamageToBoss(FName HitPointName, int32 DamageAmount);
	// 데미지 소스별 마지막 타격 시간을 기록합니다.
	void RecordDamageRateLimit(AActor* DamageSource);
	// 핀볼 타격 시 핀볼에 반발 임펄스를 적용합니다.
	void ApplyPinballHitImpulse(AActor* DamageSource, const FHitResult& Hit) const;
	// 핀볼 타격 성공 시 콤보 카운트를 증가시킵니다.
	void AddPinballCombo(AActor* DamageSource) const;

	UPROPERTY(Transient)
	TObjectPtr<APBBossBase> OwnerBoss;

	TMap<TObjectKey<AActor>, float> LastDamageTimeMap;
	uint64 LastDamageFrameNumber = 0;
	int32 CurrentFrameDamageCount = 0;
};
