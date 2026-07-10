#include "Collection/PBCollectionTableParser.h"

#include "PBSheetParserUtils.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"

using namespace PBSheetParserUtils;

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
