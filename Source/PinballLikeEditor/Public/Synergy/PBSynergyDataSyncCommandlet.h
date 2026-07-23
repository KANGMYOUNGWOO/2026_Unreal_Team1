#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "PBSynergyDataSyncCommandlet.generated.h"

UCLASS()
class PINBALLLIKEEDITOR_API UPBSynergyDataSyncCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UPBSynergyDataSyncCommandlet();
	virtual int32 Main(const FString& Params) override;
};
