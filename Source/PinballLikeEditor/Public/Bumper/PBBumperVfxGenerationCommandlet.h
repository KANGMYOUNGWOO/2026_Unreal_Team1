#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PBBumperVfxGenerationCommandlet.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperVfxGenerationCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UPBBumperVfxGenerationCommandlet();
	virtual int32 Main(const FString& Params) override;
};
