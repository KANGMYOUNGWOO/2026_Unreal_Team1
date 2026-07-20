#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PBBumperDataSyncCommandlet.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBBumperDataSyncCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UPBBumperDataSyncCommandlet();
	virtual int32 Main(const FString& Params) override;
};
