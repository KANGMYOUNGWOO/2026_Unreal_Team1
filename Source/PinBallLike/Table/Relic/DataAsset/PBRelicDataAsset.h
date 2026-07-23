#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PBRelicDataAsset.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBRelicDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Relic")
	FName RowName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Relic")
	TSoftObjectPtr<UTexture2D> RelicIcon;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};