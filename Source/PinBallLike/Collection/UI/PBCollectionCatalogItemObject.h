#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTabTypes.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "UObject/Object.h"
#include "PBCollectionCatalogItemObject.generated.h"

UCLASS(BlueprintType)
class PINBALLLIKE_API UPBCollectionCatalogItemObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Collection|Catalog")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Catalog")
	FPBCollectionItemSummary Summary;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Catalog")
	int32 DataIndex = INDEX_NONE;
};
