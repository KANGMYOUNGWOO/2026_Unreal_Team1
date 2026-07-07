#include "Collection/PBCollectionTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"

using namespace PBSheetParserUtils;

namespace
{
FName ParseNameValue(const FString& Value)
{
	return IsUnsetValue(Value) ? NAME_None : FName(*TrimCell(Value));
}

bool ParseBoolValue(const FString& Value, const bool bDefaultValue)
{
	if (IsUnsetValue(Value))
	{
		return bDefaultValue;
	}

	const FString TrimmedValue = TrimCell(Value);
	return TrimmedValue.Equals(TEXT("true"), ESearchCase::IgnoreCase)
		|| TrimmedValue.Equals(TEXT("1"), ESearchCase::IgnoreCase)
		|| TrimmedValue.Equals(TEXT("yes"), ESearchCase::IgnoreCase);
}

FLinearColor ParseLinearColorValue(const FString& Value, const FLinearColor& DefaultValue)
{
	if (IsUnsetValue(Value))
	{
		return DefaultValue;
	}

	const FString TrimmedValue = TrimCell(Value);
	FLinearColor ParsedColor;
	if (ParsedColor.InitFromString(TrimmedValue))
	{
		return ParsedColor;
	}

	TArray<FString> Tokens;
	TrimmedValue.ParseIntoArray(Tokens, TEXT(","), true);
	if (Tokens.Num() < 3)
	{
		return DefaultValue;
	}

	const float R = ParseFloatValue(Tokens[0], DefaultValue.R);
	const float G = ParseFloatValue(Tokens[1], DefaultValue.G);
	const float B = ParseFloatValue(Tokens[2], DefaultValue.B);
	const float A = Tokens.IsValidIndex(3)
		? ParseFloatValue(Tokens[3], DefaultValue.A)
		: DefaultValue.A;

	return FLinearColor(R, G, B, A);
}
}

const TCHAR* UPBCollectionTableParser::GetParserName() const
{
	return TEXT("Collection");
}

UScriptStruct* UPBCollectionTableParser::GetRowStruct() const
{
	return FPBCollectionTableRow::StaticStruct();
}

bool UPBCollectionTableParser::ParseRow(const FName RowName, const TMap<FString, FString>& RowData)
{
	FPBCollectionTableRow NewRow;
	NewRow.CollectionId = ParseNameValue(RowData.FindRef(TEXT("CollectionId")));
	NewRow.Category = ParseEnumValue(RowData.FindRef(TEXT("Category")), EPBCollectionCategory::Ball);
	NewRow.SourceId = ParseNameValue(RowData.FindRef(TEXT("SourceId")));
	NewRow.SourceTableName = ParseNameValue(RowData.FindRef(TEXT("SourceTableName")));
	NewRow.SourceRowName = ParseNameValue(RowData.FindRef(TEXT("SourceRowName")));
	NewRow.DisplayName = FText::FromString(RowData.FindRef(TEXT("DisplayName")));
	NewRow.LockedName = FText::FromString(RowData.FindRef(TEXT("LockedName")));
	NewRow.ShortDescription = FText::FromString(RowData.FindRef(TEXT("ShortDescription")));
	NewRow.DetailDescription = FText::FromString(RowData.FindRef(TEXT("DetailDescription")));
	NewRow.UnlockConditionText = FText::FromString(RowData.FindRef(TEXT("UnlockConditionText")));
	NewRow.AttackTypeId = ParseNameValue(RowData.FindRef(TEXT("AttackTypeId")));
	NewRow.RoleId = ParseNameValue(RowData.FindRef(TEXT("RoleId")));
	NewRow.AttributeId = ParseNameValue(RowData.FindRef(TEXT("AttributeId")));
	NewRow.StarGrade = FMath::Max(ParseIntValue(RowData.FindRef(TEXT("StarGrade")), 0), 0);
	NewRow.Tags = TrimCell(RowData.FindRef(TEXT("Tags")));
	NewRow.SortOrder = ParseIntValue(RowData.FindRef(TEXT("SortOrder")), 0);
	NewRow.IconAssetKey = ParseNameValue(RowData.FindRef(TEXT("IconAssetKey")));
	NewRow.PreviewAssetKey = ParseNameValue(RowData.FindRef(TEXT("PreviewAssetKey")));
	NewRow.AssetBundleName = ParseNameValue(RowData.FindRef(TEXT("AssetBundleName")));
	NewRow.bHiddenUntilDiscovered = ParseBoolValue(RowData.FindRef(TEXT("bHiddenUntilDiscovered")), false);
	NewRow.AccentColor = ParseLinearColorValue(RowData.FindRef(TEXT("AccentColor")), FLinearColor::White);

	if (NewRow.CollectionId.IsNone())
	{
		NewRow.CollectionId = RowName;
	}

	TargetTable->AddRow(RowName, NewRow);
	return true;
}
