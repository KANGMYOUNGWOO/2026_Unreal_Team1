#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTabTypes.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "UObject/Object.h"
#include "PBCollectionCatalogItemObject.generated.h"

/** ListView와 TileView에 전달하는 도감 전용 일시 객체입니다. 저장 대상이 아닙니다. */
UCLASS(BlueprintType)
class PINBALLLIKE_API UPBCollectionCatalogItemObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Collection|Catalog")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Catalog")
	FPBCollectionItemSummary Summary;

	/** 탭이 보관한 원본 표시 배열의 위치입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Collection|Catalog")
	int32 DataIndex = INDEX_NONE;
};
