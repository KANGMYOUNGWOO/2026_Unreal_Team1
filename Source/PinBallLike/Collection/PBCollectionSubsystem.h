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

UCLASS()
class PINBALLLIKE_API UPBCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionEntryChangedSignature OnCollectionEntryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionProgressChangedSignature OnCollectionProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionDataReadySignature OnCollectionDataReady;

	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionNotificationRequestedSignature OnCollectionNotificationRequested;

	UFUNCTION(BlueprintCallable, Category = "Collection|Data")
	bool ReloadCollectionData();

	UFUNCTION(BlueprintPure, Category = "Collection|Data")
	bool IsDataReady() const { return bIsDataReady; }

	UFUNCTION(BlueprintPure, Category = "Collection|Data")
	bool HasDataLoadCompleted() const;

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

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionBallDisplayData> GetBallCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionSynergyDisplayData> GetSynergyCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionRelicDisplayData> GetRelicCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionBumperDisplayData> GetBumperCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Catalog")
	TArray<FPBCollectionBossDisplayData> GetBossCatalogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Validation")
	TArray<FPBCollectionValidationIssue> GetCatalogValidationIssues() const;

	UFUNCTION(BlueprintPure, Category = "Collection|Source")
	TArray<FName> FindCollectionIdsBySourceId(FName SourceId) const;

	UFUNCTION(BlueprintPure, Category = "Collection|Source")
	TArray<FName> FindCollectionIdsBySourceRow(FName SourceTableName, FName SourceRowName) const;

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 모든 항목을 항상 공개하므로 진행도를 사용하지 않습니다."))
	bool DiscoverEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 모든 항목을 항상 공개하므로 해금 API를 사용하지 않습니다."))
	bool UnlockEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 완료 상태를 저장하지 않습니다."))
	bool CompleteEntry(FName CollectionId, const FString& CompletedByCharacterName);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 NEW 상태를 저장하지 않습니다."))
	bool MarkEntryAsSeen(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress", meta = (DeprecatedFunction, DeprecationMessage = "도감은 플레이 기록을 저장하지 않습니다."))
	bool UpdateEntryStatistics(
		FName CollectionId,
		int32 AcquireDelta,
		int32 UseDelta,
		int32 DefeatDelta,
		int32 BestComboCandidate,
		int32 TotalDamageDelta);

	UFUNCTION(BlueprintPure, Category = "Collection|Persistence", meta = (DeprecatedFunction, DeprecationMessage = "도감 진행도 저장은 사용하지 않습니다."))
	TArray<FPBCollectionProgressData> GetProgressSnapshot() const;

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

	TMap<FName, int32> EntryIndexByCollectionId;
	TMap<FName, TArray<FName>> CollectionIdsBySourceId;
	TMap<FName, TArray<FName>> CollectionIdsBySourceRowName;

	bool bIsDataReady = false;
};
