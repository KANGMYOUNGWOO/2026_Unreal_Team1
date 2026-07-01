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
	Relic UMETA(DisplayName = "Relic")
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

/**
 * 도감 항목의 고정 표시 데이터입니다.
 * 프로토타입에서는 C++ 더미 데이터로 채우고, 이후 DataAsset/DataTable로 옮길 수 있게 값 타입으로 둡니다.
 */
USTRUCT(BlueprintType)
struct FPBCollectionEntryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FLinearColor AccentColor = FLinearColor::White;
};

/**
 * 도감 목록을 조회할 때 적용할 검색 / 필터 / 정렬 조건입니다.
 * 비어 있는 FName과 0 성급은 해당 조건을 적용하지 않는다는 의미입니다.
 */
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

/**
 * 유저별 도감 진행 데이터입니다.
 * 실제 저장은 SaveGame으로 옮기기 전까지 메모리 상태로만 사용합니다.
 */
USTRUCT(BlueprintType)
struct FPBCollectionProgressData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionState State = EPBCollectionState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	bool bIsNew = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	bool bIsEnabledInRunPool = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FString FirstDiscoveredAtText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FString FirstUnlockedAtText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FString CompletedAtText;

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

/**
 * UI가 바로 표시할 수 있도록 고정 데이터와 진행 데이터를 합친 값입니다.
 */
USTRUCT(BlueprintType)
struct FPBCollectionDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionState State = EPBCollectionState::Locked;

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
	bool bCanShowFullData = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FLinearColor AccentColor = FLinearColor::White;
};
