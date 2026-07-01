#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBCollectionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBCollectionEntryChangedSignature, FName, CollectionId);

/**
 * 도감 프로토타입의 데이터 조회와 상태 변경을 담당합니다.
 * 현재는 더미 데이터를 메모리에 만들고, 검색 / 필터 / 정렬 조회와 이후 SaveGame/DataAsset 연동 지점으로 확장합니다.
 */
UCLASS()
class PINBALLLIKE_API UPBCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Collection|Event")
	FPBCollectionEntryChangedSignature OnCollectionEntryChanged;

	UFUNCTION(BlueprintCallable, Category = "Collection|Demo")
	void ResetDemoData();

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	void ResetAllProgress();

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

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool DiscoverEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool UnlockEntry(FName CollectionId);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool CompleteEntry(FName CollectionId, const FString& CompletedByCharacterName);

	UFUNCTION(BlueprintCallable, Category = "Collection|Progress")
	bool AdvanceDemoState(FName CollectionId);

	UFUNCTION(BlueprintPure, Category = "Collection|Text")
	static FText GetCategoryDisplayText(EPBCollectionCategory Category);

	UFUNCTION(BlueprintPure, Category = "Collection|Text")
	static FText GetStateDisplayText(EPBCollectionState State);

	UFUNCTION(BlueprintPure, Category = "Collection|Text")
	static FText GetMetadataDisplayText(FName MetadataId);

private:
	void BuildDemoEntries();
	void BuildDemoProgress();
	void BuildLockedProgress();
	void AddDemoEntry(
		FName CollectionId,
		EPBCollectionCategory Category,
		const FText& DisplayName,
		const FText& ShortDescription,
		const FText& DetailDescription,
		const FText& UnlockConditionText,
		FName AttackTypeId,
		FName RoleId,
		FName AttributeId,
		int32 StarGrade,
		const TArray<FName>& Tags,
		int32 SortOrder,
		const FLinearColor& AccentColor);
	void SetInitialProgress(FName CollectionId, EPBCollectionState State);

	const FPBCollectionEntryData* FindEntryData(FName CollectionId) const;
	FPBCollectionProgressData* FindProgressData(FName CollectionId);
	const FPBCollectionProgressData* FindProgressData(FName CollectionId) const;

	FPBCollectionDisplayData MakeDisplayData(
		const FPBCollectionEntryData& EntryData,
		const FPBCollectionProgressData& ProgressData) const;
	bool DoesEntryMatchQuery(
		const FPBCollectionEntryData& EntryData,
		const FPBCollectionProgressData& ProgressData,
		const FPBCollectionQuery& Query) const;
	FText BuildRecordText(const FPBCollectionProgressData& ProgressData) const;

	static FString MakeNowText();

	UPROPERTY()
	TArray<FPBCollectionEntryData> DemoEntries;

	UPROPERTY()
	TMap<FName, FPBCollectionProgressData> ProgressMap;
};
