#include "Shop/PBShopTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Table/Shop/Struct/PBShopTableRow.h"

using namespace PBSheetParserUtils;

const TCHAR* UPBShopTableParser::GetParserName() const
{
	return TEXT("Shop");
}

UScriptStruct* UPBShopTableParser::GetRowStruct() const
{
	return FPBShopTableRow::StaticStruct();
}

bool UPBShopTableParser::ParseRow(
	const FName RowName,
	const TMap<FString, FString>& RowData)
{
	FPBShopTableRow NewRow;

	NewRow.BallKey = FName(*TrimCell(RowData.FindRef(TEXT("BallKey"))));

	NewRow.BuyPrice = FCString::Atoi(
		*TrimCell(RowData.FindRef(TEXT("BuyPrice"))));

	NewRow.SellPrice = FCString::Atoi(
		*TrimCell(RowData.FindRef(TEXT("SellPrice"))));

	NewRow.ShopWeight = FCString::Atoi(
		*TrimCell(RowData.FindRef(TEXT("ShopWeight"))));

	const FString CanAppearText =
		TrimCell(RowData.FindRef(TEXT("bCanAppearInShop")));

	NewRow.bCanAppearInShop =
		CanAppearText.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase) ||
		CanAppearText.Equals(TEXT("true"), ESearchCase::IgnoreCase) ||
		CanAppearText.Equals(TEXT("1"));

	TargetTable->AddRow(RowName, NewRow);
	return true;
}