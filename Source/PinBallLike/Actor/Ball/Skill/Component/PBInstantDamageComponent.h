#pragma once

#include "CoreMinimal.h"
#include "PBDamageComponentBase.h"
#include "PBInstantDamageComponent.generated.h"

UCLASS(ClassGroup=(PinBall), meta=(BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBInstantDamageComponent : public UPBDamageComponentBase
{
	GENERATED_BODY()

public:
	UPBInstantDamageComponent();
};
