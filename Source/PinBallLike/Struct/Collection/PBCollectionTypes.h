#pragma once

#include "CoreMinimal.h"
#include "PBCollectionTypes.generated.h"

UENUM(BlueprintType)
enum class EPBCollectionCategory : uint8
{
	All UMETA(DisplayName = "All"),
	Ball UMETA(DisplayName = "Ball"),
	Bumper UMETA(DisplayName = "Bumper"),
	Boss UMETA(DisplayName = "Boss"),
	Relic UMETA(DisplayName = "Relic"),
	Achievement UMETA(DisplayName = "Achievement"),
	Synergy UMETA(DisplayName = "Synergy")
};

UENUM(BlueprintType)
enum class EPBCollectionState : uint8
{
	Locked UMETA(DisplayName = "Locked"),
	Discovered UMETA(DisplayName = "Discovered"),
	Unlocked UMETA(DisplayName = "Unlocked"),
	Completed UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class EPBCollectionFilterField : uint8
{
	AttackType UMETA(DisplayName = "Attack Type"),
	Role UMETA(DisplayName = "Role"),
	Attribute UMETA(DisplayName = "Attribute")
};

UENUM(BlueprintType)
enum class EPBCollectionSortMode : uint8
{
	SortOrder UMETA(DisplayName = "Default"),
	NameAsc UMETA(DisplayName = "Name Ascending"),
	StarGradeDesc UMETA(DisplayName = "Star Grade Descending"),
	StarGradeAsc UMETA(DisplayName = "Star Grade Ascending"),
	StateDesc UMETA(DisplayName = "State Descending")
};

USTRUCT(BlueprintType)
struct FPBCollectionEntryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceTableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText LockedName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText ShortDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText DetailDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText UnlockConditionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName AttackTypeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName RoleId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName AttributeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter", meta = (ClampMin = "0"))
	int32 StarGrade = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	TArray<FName> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 SortOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName IconAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName PreviewAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName AssetBundleName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Visibility")
	bool bHiddenUntilDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FLinearColor AccentColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FPBCollectionQuery
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Query")
	EPBCollectionCategory Category = EPBCollectionCategory::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Query")
	FString SearchText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Query")
	FName AttackTypeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Query")
	FName RoleId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Query")
	FName AttributeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Query", meta = (ClampMin = "0"))
	int32 StarGrade = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Query")
	EPBCollectionSortMode SortMode = EPBCollectionSortMode::SortOrder;
};

USTRUCT(BlueprintType)
struct FPBCollectionProgressData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionState State = EPBCollectionState::Unlocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	bool bIsNew = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	bool bIsEnabledInRunPool = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FDateTime FirstDiscoveredAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FDateTime FirstUnlockedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FDateTime CompletedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FString CompletedByCharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 AcquireCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 UseCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 DefeatCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 BestCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 TotalDamage = 0;
};

USTRUCT(BlueprintType)
struct FPBCollectionDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceTableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionState State = EPBCollectionState::Unlocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText CategoryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText StateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName AttackTypeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName RoleId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FName AttributeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FText AttackTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FText RoleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	FText AttributeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Filter")
	int32 StarGrade = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 SortOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText ShortDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText DetailDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText UnlockConditionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText RecordText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	bool bIsNew = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	bool bCanShowFullData = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName IconAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName PreviewAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Asset")
	FName AssetBundleName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FLinearColor AccentColor = FLinearColor::White;
};
