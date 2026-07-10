#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PBShopTableRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBShopTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// BallTable의 RowName을 가리키는 키
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FName BallKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	int32 BuyPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	int32 SellPrice = 0;

	// 상점 등장 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	int32 ShopWeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	bool bCanAppearInShop = true;
};