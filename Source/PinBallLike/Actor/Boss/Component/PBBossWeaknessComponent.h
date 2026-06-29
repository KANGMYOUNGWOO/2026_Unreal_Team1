#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectKey.h"
#include "PBBossWeaknessComponent.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBBossWeaknessOpenChangedSignature, bool, IsWeaknessOpen);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossWeaknessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossWeaknessComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Weakness")
	void OpenWeakness();

	UFUNCTION(BlueprintCallable, Category = "Boss|Weakness")
	void CloseWeakness();

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	bool IsWeaknessPoint(FName WeaknessPointName) const;

	UFUNCTION(BlueprintPure, Category = "Boss|Weakness")
	bool IsWeaknessPointOpen(FName WeaknessPointName) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Weakness")
	bool IsWeaknessOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Weakness")
	TSet<FName> WeaknessPointNames;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Weakness")
	FPBBossWeaknessOpenChangedSignature OnWeaknessOpenChanged;

private:
	void ApplyWeaknessCollisionState(bool IsEnabled);
	bool IsWeaknessCollisionComponent(const UPrimitiveComponent* PrimitiveComponent) const;

	TMap<TObjectKey<UPrimitiveComponent>, ECollisionEnabled::Type> WeaknessCollisionEnabledMap;
};
