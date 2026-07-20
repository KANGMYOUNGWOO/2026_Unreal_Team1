#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionTableRow.generated.h"

USTRUCT(BlueprintType)
struct PINBALLLIKE_API FPBCollectionTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Table")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Table")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceTableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Text")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Text")
	FText LockedName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Text")
	FText ShortDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Text")
	FText DetailDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Text")
	FText UnlockConditionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName AttackTypeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName RoleId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName AttributeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter", meta = (ClampMin = "0"))
	int32 StarGrade = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FString Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Sort")
	int32 SortOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName IconAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName PreviewAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName AssetBundleName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Visibility")
	bool bHiddenUntilDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Visual")
	FLinearColor AccentColor = FLinearColor::White;

	FPBCollectionEntryData ToEntryData() const
	{
		FPBCollectionEntryData EntryData;
		EntryData.CollectionId = CollectionId;
		EntryData.Category = Category;
		EntryData.SourceId = SourceId;
		EntryData.SourceTableName = SourceTableName;
		EntryData.SourceRowName = SourceRowName;
		EntryData.DisplayName = DisplayName;
		EntryData.LockedName = DisplayName;
		EntryData.ShortDescription = ShortDescription;
		EntryData.DetailDescription = DetailDescription;
		EntryData.UnlockConditionText = FText::GetEmpty();
		EntryData.AttackTypeId = AttackTypeId;
		EntryData.RoleId = RoleId;
		EntryData.AttributeId = AttributeId;
		EntryData.StarGrade = StarGrade;
		EntryData.SortOrder = SortOrder;
		EntryData.IconAssetKey = IconAssetKey;
		EntryData.PreviewAssetKey = PreviewAssetKey;
		EntryData.AssetBundleName = AssetBundleName;
		EntryData.bHiddenUntilDiscovered = false;
		EntryData.AccentColor = AccentColor;

		TArray<FString> TagTokens;
		Tags.ParseIntoArray(TagTokens, TEXT(";"), true);
		for (FString& TagToken : TagTokens)
		{
			TagToken.TrimStartAndEndInline();
			if (!TagToken.IsEmpty())
			{
				EntryData.Tags.Add(FName(*TagToken));
			}
		}

		return EntryData;
	}
};
