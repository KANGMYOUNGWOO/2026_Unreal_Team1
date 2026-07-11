#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionMessage.h"
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
 * DT_Collection의 고정 데이터와 플레이어별 도감 진행도의 런타임 사본을 관리합니다.
 * 영구 저장과 전투 판정은 소유하지 않으며, 저장 계층과 게임플레이 시스템이 사용할 API만 제공합니다.
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

	/** 저장 계층이 변경된 진행도를 감지할 때 사용합니다. NAME_None은 전체 스냅샷 변경을 뜻합니다. */
	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionProgressChangedSignature OnCollectionProgressChanged;

	/** DT_Collection을 읽어 실제 도감 항목을 준비한 뒤 발생합니다. */
	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionDataReadySignature OnCollectionDataReady;

	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionNotificationRequestedSignature OnCollectionNotificationRequested;

	/** 준비된 DT_Collection을 다시 읽습니다. 기존 플레이어 진행도는 유지합니다. */
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

	/** 원본 시스템 식별자와 연결된 모든 도감 ID를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Source")
	TArray<FName> FindCollectionIdsBySourceId(FName SourceId) const;

	/** 원본 테이블 종류와 RowName이 모두 일치하는 도감 ID를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Source")
	TArray<FName> FindCollectionIdsBySourceRow(FName SourceTableName, FName SourceRowName) const;

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool DiscoverEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool UnlockEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool CompleteEntry(FName CollectionId, const FString& CompletedByCharacterName);

	/** 도감의 NEW 표시를 해제합니다. 상태 자체는 변경하지 않습니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool MarkEntryAsSeen(FName CollectionId);

	/** 게임플레이에서 발생한 누적 기록을 한 번에 반영합니다. 모든 증분 값은 0 이상으로 보정됩니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool UpdateEntryStatistics(
		FName CollectionId,
		int32 AcquireDelta,
		int32 UseDelta,
		int32 DefeatDelta,
		int32 BestComboCandidate,
		int32 TotalDamageDelta);

	/** SaveGame 또는 PlayerData에 저장할 현재 진행도 사본을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Collection|Persistence")
	TArray<FPBCollectionProgressData> GetProgressSnapshot() const;

	/** 저장 계층에서 불러온 진행도를 적용합니다. 이 과정에서는 해금 알림을 발생시키지 않습니다. */
	UFUNCTION(BlueprintCallable, Category = "Collection|Persistence")
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
	void ReconcileProgressWithEntries();
	void NotifyProgressChanged(FName CollectionId);
	static void SanitizeProgressData(FPBCollectionProgressData& ProgressData);

	const FPBCollectionEntryData* FindEntryData(FName CollectionId) const;
	FPBCollectionProgressData* FindProgressData(FName CollectionId);
	const FPBCollectionProgressData* FindProgressData(FName CollectionId) const;

	FPBCollectionDisplayData MakeDisplayData(
		const FPBCollectionEntryData& EntryData,
		const FPBCollectionProgressData& ProgressData) const;
	void BroadcastProgressNotification(
		const FPBCollectionEntryData& EntryData,
		EPBCollectionState PreviousState,
		EPBCollectionState NewState);
	bool DoesEntryMatchQuery(
		const FPBCollectionEntryData& EntryData,
		const FPBCollectionProgressData& ProgressData,
		const FPBCollectionQuery& Query) const;
	FText BuildRecordText(const FPBCollectionProgressData& ProgressData) const;

	static bool HasTimestamp(const FDateTime& Timestamp);
	static FDateTime MakeNow();
	static FString FormatTimestamp(const FDateTime& Timestamp);

	UPROPERTY()
	TArray<FPBCollectionEntryData> Entries;

	UPROPERTY()
	TMap<FName, FPBCollectionProgressData> ProgressMap;

	bool bIsDataReady = false;
};
