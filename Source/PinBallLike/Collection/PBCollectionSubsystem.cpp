#include "PBCollectionSubsystem.h"

#include "PBCollectionNotificationRouter.h"
#include "Internationalization/Text.h"
#include "Misc/DateTime.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"

namespace
{
bool ContainsSearchText(const FString& Source, const FString& SearchText)
{
	return Source.Contains(SearchText, ESearchCase::IgnoreCase, ESearchDir::FromStart);
}

bool ContainsSearchText(const FText& Source, const FString& SearchText)
{
	return ContainsSearchText(Source.ToString(), SearchText);
}
}

void UPBCollectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UPBTableDataSubsystem::StaticClass());

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBTableDataSubsystem* TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>())
		{
			TableDataSubsystem->OnStartupGameDataLoaded.AddUniqueDynamic(
				this,
				&UPBCollectionSubsystem::HandleStartupGameDataLoaded);
		}
	}

	ReloadCollectionData();
}

void UPBCollectionSubsystem::Deinitialize()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBTableDataSubsystem* TableDataSubsystem = GameInstance->GetSubsystem<UPBTableDataSubsystem>())
		{
			TableDataSubsystem->OnStartupGameDataLoaded.RemoveDynamic(
				this,
				&UPBCollectionSubsystem::HandleStartupGameDataLoaded);
		}
	}

	Super::Deinitialize();
}

void UPBCollectionSubsystem::HandleStartupGameDataLoaded()
{
	ReloadCollectionData();
}

bool UPBCollectionSubsystem::ReloadCollectionData()
{
	TArray<FPBCollectionEntryData> LoadedEntries;
	if (!BuildEntriesFromCollectionTable(LoadedEntries))
	{
		// 일시적인 재로딩 실패가 이미 표시 중인 정상 데이터를 지우지 않게 합니다.
		bIsDataReady = !Entries.IsEmpty();
		return false;
	}

	Entries = MoveTemp(LoadedEntries);
	ReconcileProgressWithEntries();
	bIsDataReady = true;
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Collection] Data reload completed. EntryCount=%d ProgressCount=%d"),
		Entries.Num(),
		ProgressMap.Num());

	OnCollectionDataReady.Broadcast(true);
	OnCollectionEntryChanged.Broadcast(NAME_None);
	return true;
}

TArray<FPBCollectionDisplayData> UPBCollectionSubsystem::GetDisplayEntries(EPBCollectionCategory Category) const
{
	FPBCollectionQuery Query;
	Query.Category = Category;
	return GetDisplayEntriesByQuery(Query);
}

TArray<FPBCollectionDisplayData> UPBCollectionSubsystem::GetDisplayEntriesByQuery(const FPBCollectionQuery& Query) const
{
	TArray<FPBCollectionDisplayData> Result;

	for (const FPBCollectionEntryData& EntryData : Entries)
	{
		const FPBCollectionProgressData* ProgressData = FindProgressData(EntryData.CollectionId);
		if (!ProgressData)
		{
			continue;
		}
		if (EntryData.bHiddenUntilDiscovered && ProgressData->State == EPBCollectionState::Locked)
		{
			continue;
		}

		if (!DoesEntryMatchQuery(EntryData, *ProgressData, Query))
		{
			continue;
		}

		Result.Add(MakeDisplayData(EntryData, *ProgressData));
	}

	Result.Sort([Query](const FPBCollectionDisplayData& A, const FPBCollectionDisplayData& B)
	{
		const auto CompareBySortOrderThenId = [](const FPBCollectionDisplayData& Left, const FPBCollectionDisplayData& Right)
		{
			return Left.SortOrder != Right.SortOrder
				? Left.SortOrder < Right.SortOrder
				: Left.CollectionId.ToString() < Right.CollectionId.ToString();
		};

		switch (Query.SortMode)
		{
		case EPBCollectionSortMode::NameAsc:
			return A.DisplayName.EqualTo(B.DisplayName)
				? A.CollectionId.ToString() < B.CollectionId.ToString()
				: A.DisplayName.ToString() < B.DisplayName.ToString();
		case EPBCollectionSortMode::StarGradeDesc:
			return A.StarGrade != B.StarGrade
				? A.StarGrade > B.StarGrade
				: CompareBySortOrderThenId(A, B);
		case EPBCollectionSortMode::StarGradeAsc:
			return A.StarGrade != B.StarGrade
				? A.StarGrade < B.StarGrade
				: CompareBySortOrderThenId(A, B);
		case EPBCollectionSortMode::StateDesc:
			return A.State != B.State
				? static_cast<uint8>(A.State) > static_cast<uint8>(B.State)
				: CompareBySortOrderThenId(A, B);
		case EPBCollectionSortMode::SortOrder:
		default:
			return CompareBySortOrderThenId(A, B);
		}
	});

	return Result;
}

bool UPBCollectionSubsystem::GetDisplayEntry(FName CollectionId, FPBCollectionDisplayData& OutDisplayData) const
{
	const FPBCollectionEntryData* EntryData = FindEntryData(CollectionId);
	const FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!EntryData || !ProgressData
		|| (EntryData->bHiddenUntilDiscovered && ProgressData->State == EPBCollectionState::Locked))
	{
		return false;
	}

	OutDisplayData = MakeDisplayData(*EntryData, *ProgressData);
	return true;
}

TArray<FName> UPBCollectionSubsystem::GetAvailableMetadataIds(EPBCollectionFilterField Field) const
{
	TSet<FName> UniqueIds;
	for (const FPBCollectionEntryData& EntryData : Entries)
	{
		const FPBCollectionProgressData* ProgressData = FindProgressData(EntryData.CollectionId);
		if (!ProgressData
			|| (EntryData.bHiddenUntilDiscovered && ProgressData->State == EPBCollectionState::Locked))
		{
			continue;
		}

		FName MetadataId = NAME_None;
		switch (Field)
		{
		case EPBCollectionFilterField::AttackType:
			MetadataId = EntryData.AttackTypeId;
			break;
		case EPBCollectionFilterField::Role:
			MetadataId = EntryData.RoleId;
			break;
		case EPBCollectionFilterField::Attribute:
			MetadataId = EntryData.AttributeId;
			break;
		default:
			break;
		}

		if (!MetadataId.IsNone())
		{
			UniqueIds.Add(MetadataId);
		}
	}

	TArray<FName> Result = UniqueIds.Array();
	Result.Sort([](const FName& A, const FName& B)
	{
		return GetMetadataDisplayText(A).ToString() < GetMetadataDisplayText(B).ToString();
	});
	return Result;
}

TArray<int32> UPBCollectionSubsystem::GetAvailableStarGrades() const
{
	TSet<int32> UniqueGrades;
	for (const FPBCollectionEntryData& EntryData : Entries)
	{
		const FPBCollectionProgressData* ProgressData = FindProgressData(EntryData.CollectionId);
		if (!ProgressData
			|| (EntryData.bHiddenUntilDiscovered && ProgressData->State == EPBCollectionState::Locked))
		{
			continue;
		}

		if (EntryData.StarGrade > 0)
		{
			UniqueGrades.Add(EntryData.StarGrade);
		}
	}

	TArray<int32> Result = UniqueGrades.Array();
	Result.Sort([](int32 A, int32 B)
	{
		return A > B;
	});
	return Result;
}

TArray<FName> UPBCollectionSubsystem::FindCollectionIdsBySourceId(FName SourceId) const
{
	TArray<FName> Result;
	if (SourceId.IsNone())
	{
		return Result;
	}

	for (const FPBCollectionEntryData& EntryData : Entries)
	{
		if (EntryData.SourceId == SourceId)
		{
			Result.Add(EntryData.CollectionId);
		}
	}
	return Result;
}

TArray<FName> UPBCollectionSubsystem::FindCollectionIdsBySourceRow(
	FName SourceTableName,
	FName SourceRowName) const
{
	TArray<FName> Result;
	if (SourceTableName.IsNone() || SourceRowName.IsNone())
	{
		return Result;
	}

	for (const FPBCollectionEntryData& EntryData : Entries)
	{
		if (EntryData.SourceTableName == SourceTableName && EntryData.SourceRowName == SourceRowName)
		{
			Result.Add(EntryData.CollectionId);
		}
	}
	return Result;
}

bool UPBCollectionSubsystem::DiscoverEntry(FName CollectionId)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	const FPBCollectionEntryData* EntryData = FindEntryData(CollectionId);
	if (!ProgressData || !EntryData)
	{
		return false;
	}

	const EPBCollectionState PreviousState = ProgressData->State;
	if (ProgressData->State == EPBCollectionState::Locked)
	{
		ProgressData->State = EPBCollectionState::Discovered;
		if (!HasTimestamp(ProgressData->FirstDiscoveredAt))
		{
			ProgressData->FirstDiscoveredAt = MakeNow();
		}
		ProgressData->bIsNew = true;
		NotifyProgressChanged(CollectionId);
		BroadcastProgressNotification(*EntryData, PreviousState, ProgressData->State);
	}

	return true;
}

bool UPBCollectionSubsystem::UnlockEntry(FName CollectionId)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	const FPBCollectionEntryData* EntryData = FindEntryData(CollectionId);
	if (!ProgressData || !EntryData)
	{
		return false;
	}

	const EPBCollectionState PreviousState = ProgressData->State;
	const FDateTime Now = MakeNow();
	if (ProgressData->State == EPBCollectionState::Locked
		&& !HasTimestamp(ProgressData->FirstDiscoveredAt))
	{
		ProgressData->FirstDiscoveredAt = Now;
	}

	if (ProgressData->State == EPBCollectionState::Locked || ProgressData->State == EPBCollectionState::Discovered)
	{
		ProgressData->State = EPBCollectionState::Unlocked;
		if (!HasTimestamp(ProgressData->FirstUnlockedAt))
		{
			ProgressData->FirstUnlockedAt = Now;
		}
		ProgressData->bIsNew = true;
		NotifyProgressChanged(CollectionId);
		BroadcastProgressNotification(*EntryData, PreviousState, ProgressData->State);
	}

	return true;
}

bool UPBCollectionSubsystem::CompleteEntry(FName CollectionId, const FString& CompletedByCharacterName)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	const FPBCollectionEntryData* EntryData = FindEntryData(CollectionId);
	if (!ProgressData || !EntryData)
	{
		return false;
	}

	const EPBCollectionState PreviousState = ProgressData->State;
	const bool bWasCompleted = ProgressData->State == EPBCollectionState::Completed;
	bool bChanged = false;
	if (!bWasCompleted)
	{
		const FDateTime Now = MakeNow();
		if (!HasTimestamp(ProgressData->FirstDiscoveredAt))
		{
			ProgressData->FirstDiscoveredAt = Now;
		}
		if (!HasTimestamp(ProgressData->FirstUnlockedAt))
		{
			ProgressData->FirstUnlockedAt = Now;
		}
		if (!HasTimestamp(ProgressData->CompletedAt))
		{
			ProgressData->CompletedAt = Now;
		}

		ProgressData->State = EPBCollectionState::Completed;
		ProgressData->bIsNew = true;
		bChanged = true;
	}

	if (!CompletedByCharacterName.IsEmpty()
		&& ProgressData->CompletedByCharacterName.IsEmpty())
	{
		ProgressData->CompletedByCharacterName = CompletedByCharacterName;
		bChanged = true;
	}

	if (bChanged)
	{
		NotifyProgressChanged(CollectionId);
	}
	if (!bWasCompleted)
	{
		BroadcastProgressNotification(*EntryData, PreviousState, ProgressData->State);
	}
	return true;
}

bool UPBCollectionSubsystem::MarkEntryAsSeen(FName CollectionId)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!ProgressData || !FindEntryData(CollectionId))
	{
		return false;
	}

	if (ProgressData->bIsNew)
	{
		ProgressData->bIsNew = false;
		NotifyProgressChanged(CollectionId);
	}
	return true;
}

bool UPBCollectionSubsystem::UpdateEntryStatistics(
	FName CollectionId,
	int32 AcquireDelta,
	int32 UseDelta,
	int32 DefeatDelta,
	int32 BestComboCandidate,
	int32 TotalDamageDelta)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!ProgressData || !FindEntryData(CollectionId))
	{
		return false;
	}

	const auto AddClamped = [](int32 CurrentValue, int32 Delta)
	{
		return static_cast<int32>(FMath::Min<int64>(
			static_cast<int64>(MAX_int32),
			static_cast<int64>(CurrentValue) + FMath::Max(0, Delta)));
	};

	const int32 NewAcquireCount = AddClamped(ProgressData->AcquireCount, AcquireDelta);
	const int32 NewUseCount = AddClamped(ProgressData->UseCount, UseDelta);
	const int32 NewDefeatCount = AddClamped(ProgressData->DefeatCount, DefeatDelta);
	const int32 NewBestCombo = FMath::Max(ProgressData->BestCombo, FMath::Max(0, BestComboCandidate));
	const int32 NewTotalDamage = AddClamped(ProgressData->TotalDamage, TotalDamageDelta);

	const bool bChanged = NewAcquireCount != ProgressData->AcquireCount
		|| NewUseCount != ProgressData->UseCount
		|| NewDefeatCount != ProgressData->DefeatCount
		|| NewBestCombo != ProgressData->BestCombo
		|| NewTotalDamage != ProgressData->TotalDamage;

	ProgressData->AcquireCount = NewAcquireCount;
	ProgressData->UseCount = NewUseCount;
	ProgressData->DefeatCount = NewDefeatCount;
	ProgressData->BestCombo = NewBestCombo;
	ProgressData->TotalDamage = NewTotalDamage;

	if (bChanged)
	{
		NotifyProgressChanged(CollectionId);
	}
	return true;
}

TArray<FPBCollectionProgressData> UPBCollectionSubsystem::GetProgressSnapshot() const
{
	TArray<FPBCollectionProgressData> Result;
	ProgressMap.GenerateValueArray(Result);
	Result.Sort([](const FPBCollectionProgressData& A, const FPBCollectionProgressData& B)
	{
		return A.CollectionId.ToString() < B.CollectionId.ToString();
	});
	return Result;
}

void UPBCollectionSubsystem::ApplyProgressSnapshot(
	const TArray<FPBCollectionProgressData>& ProgressSnapshot)
{
	ProgressMap.Reset();
	for (FPBCollectionProgressData ProgressData : ProgressSnapshot)
	{
		if (ProgressData.CollectionId.IsNone())
		{
			continue;
		}

		SanitizeProgressData(ProgressData);
		ProgressMap.Add(ProgressData.CollectionId, MoveTemp(ProgressData));
	}

	ReconcileProgressWithEntries();
	OnCollectionProgressChanged.Broadcast(NAME_None);
	OnCollectionEntryChanged.Broadcast(NAME_None);
}

FText UPBCollectionSubsystem::GetCategoryDisplayText(EPBCollectionCategory Category)
{
	switch (Category)
	{
	case EPBCollectionCategory::All:
		return NSLOCTEXT("PBCollection", "CategoryAll", "전체");
	case EPBCollectionCategory::Ball:
		return NSLOCTEXT("PBCollection", "CategoryBall", "Ball");
	case EPBCollectionCategory::Bumper:
		return NSLOCTEXT("PBCollection", "CategoryBumper", "Bumper");
	case EPBCollectionCategory::Boss:
		return NSLOCTEXT("PBCollection", "CategoryBoss", "Boss");
	case EPBCollectionCategory::Relic:
		return NSLOCTEXT("PBCollection", "CategoryRelic", "Relic");
	case EPBCollectionCategory::Achievement:
		return NSLOCTEXT("PBCollection", "CategoryAchievement", "업적");
	default:
		return FText::GetEmpty();
	}
}

FText UPBCollectionSubsystem::GetStateDisplayText(EPBCollectionState State)
{
	switch (State)
	{
	case EPBCollectionState::Locked:
		return NSLOCTEXT("PBCollection", "StateLocked", "잠김");
	case EPBCollectionState::Discovered:
		return NSLOCTEXT("PBCollection", "StateDiscovered", "발견");
	case EPBCollectionState::Unlocked:
		return NSLOCTEXT("PBCollection", "StateUnlocked", "해금");
	case EPBCollectionState::Completed:
		return NSLOCTEXT("PBCollection", "StateCompleted", "완료");
	default:
		return FText::GetEmpty();
	}
}

FText UPBCollectionSubsystem::GetMetadataDisplayText(FName MetadataId)
{
	if (MetadataId == TEXT("Melee"))
	{
		return NSLOCTEXT("PBCollection", "MetadataMelee", "근접");
	}
	if (MetadataId == TEXT("PowerFlip"))
	{
		return NSLOCTEXT("PBCollection", "MetadataPowerFlip", "파워플립");
	}
	if (MetadataId == TEXT("Projectile"))
	{
		return NSLOCTEXT("PBCollection", "MetadataProjectile", "투사체");
	}
	if (MetadataId == TEXT("Collision"))
	{
		return NSLOCTEXT("PBCollection", "MetadataCollision", "충돌");
	}
	if (MetadataId == TEXT("Summon"))
	{
		return NSLOCTEXT("PBCollection", "MetadataSummon", "소환");
	}
	if (MetadataId == TEXT("Support"))
	{
		return NSLOCTEXT("PBCollection", "MetadataSupport", "지원");
	}
	if (MetadataId == TEXT("Passive"))
	{
		return NSLOCTEXT("PBCollection", "MetadataPassive", "패시브");
	}
	if (MetadataId == TEXT("Pattern"))
	{
		return NSLOCTEXT("PBCollection", "MetadataPattern", "패턴");
	}
	if (MetadataId == TEXT("Leader"))
	{
		return NSLOCTEXT("PBCollection", "MetadataLeader", "리더");
	}
	if (MetadataId == TEXT("Damage"))
	{
		return NSLOCTEXT("PBCollection", "MetadataDamage", "딜러");
	}
	if (MetadataId == TEXT("Groggy"))
	{
		return NSLOCTEXT("PBCollection", "MetadataGroggy", "그로기");
	}
	if (MetadataId == TEXT("Controller"))
	{
		return NSLOCTEXT("PBCollection", "MetadataController", "제어");
	}
	if (MetadataId == TEXT("Tank"))
	{
		return NSLOCTEXT("PBCollection", "MetadataTank", "방어");
	}
	if (MetadataId == TEXT("Boss"))
	{
		return NSLOCTEXT("PBCollection", "MetadataBoss", "보스");
	}
	if (MetadataId == TEXT("Resource"))
	{
		return NSLOCTEXT("PBCollection", "MetadataResource", "자원");
	}
	if (MetadataId == TEXT("Neutral"))
	{
		return NSLOCTEXT("PBCollection", "MetadataNeutral", "무속성");
	}
	if (MetadataId == TEXT("Fire"))
	{
		return NSLOCTEXT("PBCollection", "MetadataFire", "화염");
	}
	if (MetadataId == TEXT("Water"))
	{
		return NSLOCTEXT("PBCollection", "MetadataWater", "물");
	}
	if (MetadataId == TEXT("Nature"))
	{
		return NSLOCTEXT("PBCollection", "MetadataNature", "자연");
	}
	if (MetadataId == TEXT("Light"))
	{
		return NSLOCTEXT("PBCollection", "MetadataLight", "빛");
	}
	if (MetadataId == TEXT("Dark"))
	{
		return NSLOCTEXT("PBCollection", "MetadataDark", "어둠");
	}
	if (MetadataId == TEXT("Physical"))
	{
		return NSLOCTEXT("PBCollection", "MetadataPhysical", "물리");
	}
	if (MetadataId == TEXT("Arcane"))
	{
		return NSLOCTEXT("PBCollection", "MetadataArcane", "비전");
	}
	if (MetadataId == TEXT("Achievement"))
	{
		return NSLOCTEXT("PBCollection", "MetadataAchievement", "업적");
	}
	if (MetadataId == TEXT("Objective"))
	{
		return NSLOCTEXT("PBCollection", "MetadataObjective", "목표");
	}
	if (MetadataId == TEXT("Account"))
	{
		return NSLOCTEXT("PBCollection", "MetadataAccount", "계정");
	}
	if (MetadataId == TEXT("Challenge"))
	{
		return NSLOCTEXT("PBCollection", "MetadataChallenge", "도전");
	}

	return FText::FromName(MetadataId);
}

bool UPBCollectionSubsystem::BuildEntriesFromCollectionTable(
	TArray<FPBCollectionEntryData>& OutEntries) const
{
	OutEntries.Reset();

	const UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return false;
	}

	const UPBTableDataSubsystem* TableDataSubsystem =
		GameInstance->GetSubsystem<UPBTableDataSubsystem>();
	if (!IsValid(TableDataSubsystem) || !TableDataSubsystem->IsCollectionTableReady())
	{
		return false;
	}

	TArray<FPBCollectionTableRow> CollectionRows;
	TableDataSubsystem->GetAllCollectionRows(CollectionRows);
	if (CollectionRows.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DT_Collection이 준비되었지만 유효한 행이 없습니다."));
		return false;
	}

	TSet<FName> AddedCollectionIds;
	for (const FPBCollectionTableRow& Row : CollectionRows)
	{
		FPBCollectionEntryData EntryData = Row.ToEntryData();
		if (EntryData.CollectionId.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("CollectionId가 비어 있는 DT_Collection 행을 건너뜁니다."));
			continue;
		}
		if (AddedCollectionIds.Contains(EntryData.CollectionId))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("중복 CollectionId '%s'를 건너뜁니다."),
				*EntryData.CollectionId.ToString());
			continue;
		}

		AddedCollectionIds.Add(EntryData.CollectionId);
		OutEntries.Add(MoveTemp(EntryData));
	}

	OutEntries.Sort([](const FPBCollectionEntryData& A, const FPBCollectionEntryData& B)
	{
		return A.SortOrder != B.SortOrder
			? A.SortOrder < B.SortOrder
			: A.CollectionId.ToString() < B.CollectionId.ToString();
	});

	return !OutEntries.IsEmpty();
}

void UPBCollectionSubsystem::ReconcileProgressWithEntries()
{
	for (const FPBCollectionEntryData& EntryData : Entries)
	{
		FPBCollectionProgressData& ProgressData = ProgressMap.FindOrAdd(EntryData.CollectionId);
		ProgressData.CollectionId = EntryData.CollectionId;
		SanitizeProgressData(ProgressData);
	}
}

void UPBCollectionSubsystem::NotifyProgressChanged(FName CollectionId)
{
	OnCollectionProgressChanged.Broadcast(CollectionId);
	OnCollectionEntryChanged.Broadcast(CollectionId);
}

void UPBCollectionSubsystem::SanitizeProgressData(FPBCollectionProgressData& ProgressData)
{
	if (static_cast<uint8>(ProgressData.State) > static_cast<uint8>(EPBCollectionState::Completed))
	{
		ProgressData.State = EPBCollectionState::Locked;
	}

	ProgressData.AcquireCount = FMath::Max(0, ProgressData.AcquireCount);
	ProgressData.UseCount = FMath::Max(0, ProgressData.UseCount);
	ProgressData.DefeatCount = FMath::Max(0, ProgressData.DefeatCount);
	ProgressData.BestCombo = FMath::Max(0, ProgressData.BestCombo);
	ProgressData.TotalDamage = FMath::Max(0, ProgressData.TotalDamage);
}

const FPBCollectionEntryData* UPBCollectionSubsystem::FindEntryData(FName CollectionId) const
{
	return Entries.FindByPredicate([CollectionId](const FPBCollectionEntryData& EntryData)
	{
		return EntryData.CollectionId == CollectionId;
	});
}

FPBCollectionProgressData* UPBCollectionSubsystem::FindProgressData(FName CollectionId)
{
	return ProgressMap.Find(CollectionId);
}

const FPBCollectionProgressData* UPBCollectionSubsystem::FindProgressData(FName CollectionId) const
{
	return ProgressMap.Find(CollectionId);
}

FPBCollectionDisplayData UPBCollectionSubsystem::MakeDisplayData(
	const FPBCollectionEntryData& EntryData,
	const FPBCollectionProgressData& ProgressData) const
{
	FPBCollectionDisplayData DisplayData;
	DisplayData.CollectionId = EntryData.CollectionId;
	DisplayData.Category = EntryData.Category;
	DisplayData.SourceId = EntryData.SourceId;
	DisplayData.SourceTableName = EntryData.SourceTableName;
	DisplayData.SourceRowName = EntryData.SourceRowName;
	DisplayData.State = ProgressData.State;
	DisplayData.CategoryText = GetCategoryDisplayText(EntryData.Category);
	DisplayData.StateText = GetStateDisplayText(ProgressData.State);
	DisplayData.AttackTypeId = EntryData.AttackTypeId;
	DisplayData.RoleId = EntryData.RoleId;
	DisplayData.AttributeId = EntryData.AttributeId;
	DisplayData.AttackTypeText = GetMetadataDisplayText(EntryData.AttackTypeId);
	DisplayData.RoleText = GetMetadataDisplayText(EntryData.RoleId);
	DisplayData.AttributeText = GetMetadataDisplayText(EntryData.AttributeId);
	DisplayData.StarGrade = EntryData.StarGrade;
	DisplayData.SortOrder = EntryData.SortOrder;
	DisplayData.UnlockConditionText = EntryData.UnlockConditionText;
	DisplayData.RecordText = BuildRecordText(ProgressData);
	DisplayData.bIsNew = ProgressData.bIsNew;
	DisplayData.bCanShowFullData = ProgressData.State == EPBCollectionState::Unlocked
		|| ProgressData.State == EPBCollectionState::Completed;
	DisplayData.IconAssetKey = EntryData.IconAssetKey;
	DisplayData.PreviewAssetKey = EntryData.PreviewAssetKey;
	DisplayData.AssetBundleName = EntryData.AssetBundleName;
	DisplayData.AccentColor = EntryData.AccentColor;

	if (ProgressData.State == EPBCollectionState::Locked)
	{
		DisplayData.DisplayName = EntryData.LockedName;
		DisplayData.ShortDescription = NSLOCTEXT("PBCollection", "LockedShortDescription", "아직 정보가 공개되지 않았습니다.");
		DisplayData.DetailDescription = NSLOCTEXT("PBCollection", "LockedDetailDescription", "해금 조건을 달성하면 상세 정보가 표시됩니다.");
		return DisplayData;
	}

	if (ProgressData.State == EPBCollectionState::Discovered)
	{
		DisplayData.DisplayName = EntryData.DisplayName;
		DisplayData.ShortDescription = EntryData.ShortDescription;
		DisplayData.DetailDescription = NSLOCTEXT("PBCollection", "DiscoveredDetailDescription", "발견한 항목입니다. 해금하면 전체 설명과 기록을 확인할 수 있습니다.");
		return DisplayData;
	}

	DisplayData.DisplayName = EntryData.DisplayName;
	DisplayData.ShortDescription = EntryData.ShortDescription;
	DisplayData.DetailDescription = EntryData.DetailDescription;
	return DisplayData;
}

void UPBCollectionSubsystem::BroadcastProgressNotification(
	const FPBCollectionEntryData& EntryData,
	EPBCollectionState PreviousState,
	EPBCollectionState NewState)
{
	if (PreviousState == NewState)
	{
		return;
	}

	FPBCollectionNotificationMessage Message;
	Message.CollectionId = EntryData.CollectionId;
	Message.Category = EntryData.Category;
	Message.PreviousState = PreviousState;
	Message.NewState = NewState;
	Message.DisplayName = EntryData.DisplayName;
	Message.MessageText = FPBCollectionNotificationRouter::BuildNotificationText(Message);

	OnCollectionNotificationRequested.Broadcast(Message);
	FPBCollectionNotificationRouter::Broadcast(this, Message);
}

bool UPBCollectionSubsystem::DoesEntryMatchQuery(
	const FPBCollectionEntryData& EntryData,
	const FPBCollectionProgressData& ProgressData,
	const FPBCollectionQuery& Query) const
{
	if (Query.Category != EPBCollectionCategory::All && EntryData.Category != Query.Category)
	{
		return false;
	}

	if (!Query.AttackTypeId.IsNone() && EntryData.AttackTypeId != Query.AttackTypeId)
	{
		return false;
	}

	if (!Query.RoleId.IsNone() && EntryData.RoleId != Query.RoleId)
	{
		return false;
	}

	if (!Query.AttributeId.IsNone() && EntryData.AttributeId != Query.AttributeId)
	{
		return false;
	}

	if (Query.StarGrade > 0 && EntryData.StarGrade != Query.StarGrade)
	{
		return false;
	}

	const FString SearchText = Query.SearchText.TrimStartAndEnd();
	if (SearchText.IsEmpty())
	{
		return true;
	}

	if (ContainsSearchText(EntryData.CollectionId.ToString(), SearchText)
		|| ContainsSearchText(EntryData.DisplayName, SearchText)
		|| ContainsSearchText(EntryData.LockedName, SearchText)
		|| ContainsSearchText(EntryData.ShortDescription, SearchText)
		|| ContainsSearchText(GetCategoryDisplayText(EntryData.Category), SearchText)
		|| ContainsSearchText(GetStateDisplayText(ProgressData.State), SearchText)
		|| ContainsSearchText(GetMetadataDisplayText(EntryData.AttackTypeId), SearchText)
		|| ContainsSearchText(GetMetadataDisplayText(EntryData.RoleId), SearchText)
		|| ContainsSearchText(GetMetadataDisplayText(EntryData.AttributeId), SearchText))
	{
		return true;
	}

	for (const FName& Tag : EntryData.Tags)
	{
		if (ContainsSearchText(Tag.ToString(), SearchText))
		{
			return true;
		}
	}

	return false;
}

FText UPBCollectionSubsystem::BuildRecordText(const FPBCollectionProgressData& ProgressData) const
{
	TArray<FString> RecordLines;

	if (HasTimestamp(ProgressData.FirstDiscoveredAt))
	{
		RecordLines.Add(FString::Printf(
			TEXT("최초 발견: %s"),
			*FormatTimestamp(ProgressData.FirstDiscoveredAt)));
	}
	if (HasTimestamp(ProgressData.FirstUnlockedAt))
	{
		RecordLines.Add(FString::Printf(
			TEXT("최초 해금: %s"),
			*FormatTimestamp(ProgressData.FirstUnlockedAt)));
	}
	if (HasTimestamp(ProgressData.CompletedAt))
	{
		RecordLines.Add(FString::Printf(
			TEXT("완료 시점: %s"),
			*FormatTimestamp(ProgressData.CompletedAt)));
	}
	if (!ProgressData.CompletedByCharacterName.IsEmpty())
	{
		RecordLines.Add(FString::Printf(
			TEXT("완료 캐릭터: %s"),
			*ProgressData.CompletedByCharacterName));
	}

	if (ProgressData.AcquireCount > 0 || ProgressData.UseCount > 0)
	{
		RecordLines.Add(FString::Printf(
			TEXT("획득/사용: %d / %d"),
			ProgressData.AcquireCount,
			ProgressData.UseCount));
	}
	if (ProgressData.DefeatCount > 0)
	{
		RecordLines.Add(FString::Printf(TEXT("처치 횟수: %d"), ProgressData.DefeatCount));
	}
	if (ProgressData.BestCombo > 0)
	{
		RecordLines.Add(FString::Printf(TEXT("최고 콤보: %d"), ProgressData.BestCombo));
	}
	if (ProgressData.TotalDamage > 0)
	{
		RecordLines.Add(FString::Printf(TEXT("누적 피해: %d"), ProgressData.TotalDamage));
	}

	return RecordLines.IsEmpty()
		? NSLOCTEXT("PBCollection", "NoProgressRecord", "아직 기록이 없습니다.")
		: FText::FromString(FString::Join(RecordLines, TEXT("\n")));
}

bool UPBCollectionSubsystem::HasTimestamp(const FDateTime& Timestamp)
{
	return Timestamp.GetTicks() > 0;
}

FDateTime UPBCollectionSubsystem::MakeNow()
{
	return FDateTime::Now();
}

FString UPBCollectionSubsystem::FormatTimestamp(const FDateTime& Timestamp)
{
	return Timestamp.ToString(TEXT("%Y-%m-%d %H:%M"));
}
