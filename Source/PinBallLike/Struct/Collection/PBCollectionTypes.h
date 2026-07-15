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
	// 기존 Widget Blueprint와 저장 데이터의 직렬화 호환을 위해 유지합니다.
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
	// 이전 UI 호환용 값입니다. 항상 공개형 도감에서는 기본 정렬과 동일하게 처리합니다.
	StateDesc UMETA(DisplayName = "State Descending")
};

/**
 * DT_Collection에서 읽은 도감 고정 데이터입니다.
 * 모든 항목은 플레이어 상태와 무관하게 처음부터 공개됩니다.
 */
USTRUCT(BlueprintType)
struct FPBCollectionEntryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FName CollectionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	EPBCollectionCategory Category = EPBCollectionCategory::Ball;

	/** 원본 DataAsset 또는 외부 시스템에서 사용하는 식별자입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceId = NAME_None;

	/** 원본 데이터가 속한 테이블 또는 데이터 종류입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceTableName = NAME_None;

	/** 원본 DataTable의 RowName 또는 원본 시스템 식별자입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText DisplayName;

	/** 이전 잠금 표시 호환용 필드입니다. 런타임 카탈로그에서는 사용하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText LockedName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText ShortDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText DetailDescription;

	/** 이전 해금 조건 호환용 필드입니다. 런타임 카탈로그에서는 사용하지 않습니다. */
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

	/** 이전 발견 상태 호환용 필드입니다. 런타임 카탈로그에서는 사용하지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Visibility")
	bool bHiddenUntilDiscovered = false;

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
 * 이전 도감 진행 저장 형식과 Blueprint 직렬화 호환을 위해 유지하는 구조체입니다.
 * 항상 공개형 카탈로그에서는 새 데이터를 저장하거나 복원하지 않습니다.
 */
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceTableName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Source")
	FName SourceRowName = NAME_None;

	/** 이전 UI 바인딩 호환용입니다. 카탈로그에서는 항상 Unlocked로 채웁니다. */
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
