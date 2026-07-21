#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PBBumperVfxGenerationCommandlet.generated.h"

enum class EPBBumperVfxStage : uint8
{
	Activation,
	Delivery,
	Impact,
	Status
};

namespace PBBumperVfxCatalog
{
	PINBALLLIKEEDITOR_API FName CanonicalizeKnownVfxId(
		FName EffectId,
		EPBBumperVfxStage Stage,
		FName VfxId);
}

UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperVfxGenerationCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UPBBumperVfxGenerationCommandlet();
	virtual int32 Main(const FString& Params) override;
};
