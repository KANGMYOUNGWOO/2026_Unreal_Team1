#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionMessage.h"
#include "PinBallLike/Struct/Collection/PBCollectionTabTypes.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBCollectionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBCollectionEntryChangedSignature, FName, CollectionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBCollectionProgressChangedSignature, FName, CollectionId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBCollectionDataReadySignature, bool, bIsReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBCollectionNotificationRequestedSignature,
	const FPBCollectionNotificationMessage&,
	Message);

/**
 * 팀의 원본 게임 테이블을 카테고리별 카탈로그 데이터로 조합합니다.
 * DT_Collection은 도감 전용 표시 메타데이터만 보충하며, 발견/해금/완료 상태는 사용하지 않습니다.
 */
UCLASS()
class PINBALLLIKE_API UPBCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 목록 또는 상세 화면을 다시 그려야 할 때 발생합니다. NAME_None은 전체 갱신을 뜻합니다. */
	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionEntryChangedSignature OnCollectionEntryChanged;

	/** 이전 도감 진행도 Blueprint와의 직렬화 호환을 위해 남겨 둔 이벤트입니다. 새 코드에서는 사용하지 않습니다. */
	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionProgressChangedSignature OnCollectionProgressChanged;

	/** 원본 게임 테이블을 조회할 준비가 끝난 뒤 발생합니다. DT_Collection 메타데이터는 선택 사항입니다. */
	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionDataReadySignature OnCollectionDataReady;

	/** 이전 해금 알림 Blueprint와의 직렬화 호환을 위해 남겨 둔 이벤트입니다. 새 코드에서는 사용하지 않습니다. */
	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionNotificationRequestedSignature OnCollectionNotificationRequested;

	/** DT_Collection 표시 메타데이터를 다시 읽고 조회 인덱스를 갱신합니다. 원본 카탈로그 준비 여부와는 독립적입니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Data")
	bool ReloadCollectionData();

	UFUNCTION(BlueprintPure, Category = "Collection|Data")
	bool IsDataReady() const { return bIsDataReady; }

	UFUNCTION(BlueprintPure, Category = "Collection|Query")
	TArray<FPBCollectionDisplayData> GetDisplayEntries(EPBCollectionCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Collection|Query")
	TArray<FPBCollectionDisplayData> GetDisplayEntriesByQuery(const FPBCollectionQuery& Query) const;

	UFUNCTION(BlueprintPure, Category = "Collection|Query")
	bool GetDisplayEntry(FName CollectionId, FPBCollectionDisplayData& OutDisplayData) const;

	UFUNCTION(BlueprintPure, Category = "Collection|Query")
	TArray<FName> GetAvailableMetadataIds(EPBCollectionFilterField Field) const;

	UFUNCTION(BlueprintPure, Category = "Collection|Query")
	TArray<int32> GetAvailableStarGrades() const;

	/** 원본 Ball/Skill 테이블을 결합한 볼 탭 전용 표시 데이터입니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionBallDisplayData> GetBallCatalogEntries() const;

	/** Synergy, Tier, Effect, Modifier, Trigger 테이블을 결합한 시너지 탭 전용 표시 데이터입니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionSynergyDisplayData> GetSynergyCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionRelicDisplayData> GetRelicCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionBumperDisplayData> GetBumperCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionBossDisplayData> GetBossCatalogEntries() const;

	/** 도감 화면을 열기 전에 확인할 수 있는 원본 테이블 참조 오류 목록입니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Validation")
	TArray<FPBCollectionValidationIssue> GetCatalogValidationIssues() const;

	/** 원본 시스템 식별자와 연결된 모든 도감 ID를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Source")
	TArray<FName> FindCollectionIdsBySourceId(FName SourceId) const;

	/** 원본 테이블 종류와 RowName이 모두 일치하는 도감 ID를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Source")
	TArray<FName> FindCollectionIdsBySourceRow(FName SourceTableName, FName SourceRowName) const;

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 모든 항목을 항상 공개하므로 진행도를 사용하지 않습니다."))
	bool DiscoverEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 모든 항목을 항상 공개하므로 해금 API를 사용하지 않습니다."))
	bool UnlockEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 완료 상태를 저장하지 않습니다."))
	bool CompleteEntry(FName CollectionId, const FString& CompletedByCharacterName);

	/** 이전 NEW 표시 API입니다. 항상 공개형 도감에서는 아무 작업도 하지 않습니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 NEW 상태를 저장하지 않습니다."))
	bool MarkEntryAsSeen(FName CollectionId);

	/** 이전 누적 기록 API입니다. 항상 공개형 도감에서는 아무 작업도 하지 않습니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 플레이 기록을 저장하지 않습니다."))
	bool UpdateEntryStatistics(
		FName CollectionId,
		int32 AcquireDelta,
		int32 UseDelta,
		int32 DefeatDelta,
		int32 BestComboCandidate,
		int32 TotalDamageDelta);

	/** 이전 저장 API입니다. 항상 빈 배열을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Persistence", meta = (DeprecatedFunction, DeprecationMessage = "도감 진행도 저장은 사용하지 않습니다."))
	TArray<FPBCollectionProgressData> GetProgressSnapshot() const;

	/** 이전 복원 API입니다. 전달된 진행도는 적용하지 않습니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Persistence", meta = (DeprecatedFunction, DeprecationMessage = "도감 진행도 저장은 사용하지 않습니다."))
	void ApplyProgressSnapshot(const TArray<FPBCollectionProgressData>& ProgressSnapshot);

	UFUNCTION(BlueprintPure, Category = "Collection|Text")
	static FText GetCategoryDisplayText(EPBCollectionCategory Category);

	UFUNCTION(BlueprintPure, Category = "Collection|Text")
	static FText GetStateDisplayText(EPBCollectionState State);

	UFUNCTION(BlueprintPure, Category = "Collection|Text")
	static FText GetMetadataDisplayText(FName MetadataId);

private:
	UFUNCTION()
	void HandleStartupGameDataLoaded();

	bool BuildEntriesFromCollectionTable(TArray<FPBCollectionEntryData>& OutEntries) const;
	void RebuildLookupIndexes();

	const FPBCollectionEntryData* FindEntryData(FName CollectionId) const;

	FPBCollectionDisplayData MakeDisplayData(const FPBCollectionEntryData& EntryData) const;
	const FPBCollectionEntryData* FindEntryDataBySourceRow(
		EPBCollectionCategory Category,
		FName SourceRowName) const;
	FPBCollectionItemSummary MakeItemSummary(
		EPBCollectionCategory Category,
		FName SourceRowName,
		const FText& SourceDisplayName,
		const FText& SourceDescription,
		int32 DefaultSortOrder = 0) const;
	bool DoesEntryMatchQuery(
		const FPBCollectionEntryData& EntryData,
		const FPBCollectionQuery& Query) const;

	UPROPERTY()
	TArray<FPBCollectionEntryData> Entries;

	/** Entries에서 재생성하는 런타임 전용 인덱스입니다. 저장 대상이 아닙니다. */
	TMap<FName, int32> EntryIndexByCollectionId;
	TMap<FName, TArray<FName>> CollectionIdsBySourceId;
	TMap<FName, TArray<FName>> CollectionIdsBySourceRowName;

	bool bIsDataReady = false;
};
