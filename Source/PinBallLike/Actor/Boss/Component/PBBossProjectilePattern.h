#pragma once

#include "CoreMinimal.h"
#include "PBBossPatternBase.h"
#include "PBBossProjectilePattern.generated.h"

UCLASS(Blueprintable)
class PINBALLLIKE_API UPBBossProjectilePattern : public UPBBossPatternBase
{
	GENERATED_BODY()

protected:
	virtual void StartPattern_Implementation(APBBossBase* Boss) override;
};
