#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Actor/Boss/Pattern/PBBossPatternBase.h"
#include "PBOctopusBindingPattern.generated.h"

class APBBossMoveArea;
class APBOctopusBindingZone;

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBOctopusBindingPattern : public UPBBossPatternBase
{
	GENERATED_BODY()

public:
	UPBOctopusBindingPattern();

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const override;
	virtual void ExecutePattern_Implementation(APBBossBase* Boss) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Binding")
	TSubclassOf<APBOctopusBindingZone> BindingZoneClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Binding", meta = (ClampMin = "0.0"))
	float ZoneRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Binding")
	float ZoneHeightOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Binding", meta = (ClampMin = "0.0"))
	float ZoneDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Binding", meta = (ClampMin = "0.0"))
	float BindDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Binding", meta = (ClampMin = "0"))
	int32 BindDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Octopus Pattern|Binding", meta = (ClampMin = "0.0"))
	float ZoneEffectScale = 1.0f;

private:
	APBBossMoveArea* FindNearestMoveArea(const APBBossBase* Boss) const;
	FVector SelectZoneLocation(const APBBossMoveArea* MoveArea) const;
};
