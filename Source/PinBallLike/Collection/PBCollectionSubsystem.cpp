#include "PBCollectionSubsystem.h"

#include "Internationalization/Text.h"
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
	const bool bMetadataLoaded = BuildEntriesFromCollectionTable(LoadedEntries);
	Entries = MoveTemp(LoadedEntries);
	RebuildLookupIndexes();

	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	bIsDataReady = IsValid(TableDataSubsystem) && TableDataSubsystem->IsCollectionCatalogDataReady();
	if (!bIsDataReady)
	{
		if (IsValid(TableDataSubsystem) && TableDataSubsystem->HasStartupGameDataLoadCompleted())
		{
			OnCollectionDataReady.Broadcast(false);
		}
		return false;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Collection] Catalog source data is ready. MetadataLoaded=%s MetadataEntryCount=%d"),
		bMetadataLoaded ? TEXT("true") : TEXT("false"),
		Entries.Num());

	OnCollectionDataReady.Broadcast(true);
	OnCollectionEntryChanged.Broadcast(NAME_None);
	return true;
}

bool UPBCollectionSubsystem::HasDataLoadCompleted() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	return IsValid(TableDataSubsystem) && TableDataSubsystem->HasStartupGameDataLoadCompleted();
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
		if (!DoesEntryMatchQuery(EntryData, Query))
		{
			continue;
		}

		Result.Add(MakeDisplayData(EntryData));
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
		case EPBCollectionSortMode::NameDesc:
			return A.DisplayName.EqualTo(B.DisplayName)
				? A.CollectionId.ToString() < B.CollectionId.ToString()
				: A.DisplayName.ToString() > B.DisplayName.ToString();
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
	if (!EntryData)
	{
		return false;
	}

	OutDisplayData = MakeDisplayData(*EntryData);
	return true;
}

TArray<FName> UPBCollectionSubsystem::GetAvailableMetadataIds(EPBCollectionFilterField Field) const
{
	TSet<FName> UniqueIds;
	for (const FPBCollectionEntryData& EntryData : Entries)
	{
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
	if (SourceId.IsNone())
	{
		return {};
	}

	if (const TArray<FName>* CollectionIds = CollectionIdsBySourceId.Find(SourceId))
	{
		return *CollectionIds;
	}

	return {};
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

	const TArray<FName>* CandidateIds = CollectionIdsBySourceRowName.Find(SourceRowName);
	if (!CandidateIds)
	{
		return Result;
	}

	Result.Reserve(CandidateIds->Num());
	for (const FName CollectionId : *CandidateIds)
	{
		const FPBCollectionEntryData* EntryData = FindEntryData(CollectionId);
		if (EntryData && EntryData->SourceTableName == SourceTableName)
		{
			Result.Add(CollectionId);
		}
	}
	return Result;
}

bool UPBCollectionSubsystem::DiscoverEntry(FName CollectionId)
{
	(void)CollectionId;
	return false;
}

bool UPBCollectionSubsystem::UnlockEntry(FName CollectionId)
{
	(void)CollectionId;
	return false;
}

bool UPBCollectionSubsystem::CompleteEntry(FName CollectionId, const FString& CompletedByCharacterName)
{
	(void)CollectionId;
	(void)CompletedByCharacterName;
	return false;
}

bool UPBCollectionSubsystem::MarkEntryAsSeen(FName CollectionId)
{
	(void)CollectionId;
	return false;
}

bool UPBCollectionSubsystem::UpdateEntryStatistics(
	FName CollectionId,
	int32 AcquireDelta,
	int32 UseDelta,
	int32 DefeatDelta,
	int32 BestComboCandidate,
	int32 TotalDamageDelta)
{
	(void)CollectionId;
	(void)AcquireDelta;
	(void)UseDelta;
	(void)DefeatDelta;
	(void)BestComboCandidate;
	(void)TotalDamageDelta;
	return false;
}

TArray<FPBCollectionProgressData> UPBCollectionSubsystem::GetProgressSnapshot() const
{
	return {};
}

void UPBCollectionSubsystem::ApplyProgressSnapshot(
	const TArray<FPBCollectionProgressData>& ProgressSnapshot)
{
	(void)ProgressSnapshot;
}

FText UPBCollectionSubsystem::GetCategoryDisplayText(EPBCollectionCategory Category)
{
	switch (Category)
	{
	case EPBCollectionCategory::All:
		return NSLOCTEXT("PBCollection", "CategoryAll", "전체");
	case EPBCollectionCategory::Ball:
		return NSLOCTEXT("PBCollection", "CategoryBall", "볼");
	case EPBCollectionCategory::Bumper:
		return NSLOCTEXT("PBCollection", "CategoryBumper", "범퍼");
	case EPBCollectionCategory::Boss:
		return NSLOCTEXT("PBCollection", "CategoryBoss", "보스");
	case EPBCollectionCategory::Relic:
		return NSLOCTEXT("PBCollection", "CategoryRelic", "유물");
	case EPBCollectionCategory::Achievement:
		return NSLOCTEXT("PBCollection", "CategoryAchievement", "업적");
	case EPBCollectionCategory::Synergy:
		return NSLOCTEXT("PBCollection", "CategorySynergy", "시너지");
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
	if (MetadataId.IsNone())
	{
		return FText::GetEmpty();
	}

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

void UPBCollectionSubsystem::RebuildLookupIndexes()
{
	EntryIndexByCollectionId.Reset();
	CollectionIdsBySourceId.Reset();
	CollectionIdsBySourceRowName.Reset();
	EntryIndexByCollectionId.Reserve(Entries.Num());

	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		const FPBCollectionEntryData& EntryData = Entries[EntryIndex];
		EntryIndexByCollectionId.Add(EntryData.CollectionId, EntryIndex);

		if (!EntryData.SourceId.IsNone())
		{
			CollectionIdsBySourceId.FindOrAdd(EntryData.SourceId).Add(EntryData.CollectionId);
		}
		if (!EntryData.SourceRowName.IsNone())
		{
			CollectionIdsBySourceRowName.FindOrAdd(EntryData.SourceRowName).Add(EntryData.CollectionId);
		}
	}
}

const FPBCollectionEntryData* UPBCollectionSubsystem::FindEntryData(FName CollectionId) const
{
	const int32* EntryIndex = EntryIndexByCollectionId.Find(CollectionId);
	if (!EntryIndex || !Entries.IsValidIndex(*EntryIndex))
	{
		return nullptr;
	}

	return &Entries[*EntryIndex];
}

FPBCollectionDisplayData UPBCollectionSubsystem::MakeDisplayData(
	const FPBCollectionEntryData& EntryData) const
{
	FPBCollectionDisplayData DisplayData;
	DisplayData.CollectionId = EntryData.CollectionId;
	DisplayData.Category = EntryData.Category;
	DisplayData.SourceId = EntryData.SourceId;
	DisplayData.SourceTableName = EntryData.SourceTableName;
	DisplayData.SourceRowName = EntryData.SourceRowName;
	DisplayData.State = EPBCollectionState::Unlocked;
	DisplayData.CategoryText = GetCategoryDisplayText(EntryData.Category);
	DisplayData.StateText = FText::GetEmpty();
	DisplayData.AttackTypeId = EntryData.AttackTypeId;
	DisplayData.RoleId = EntryData.RoleId;
	DisplayData.AttributeId = EntryData.AttributeId;
	DisplayData.AttackTypeText = GetMetadataDisplayText(EntryData.AttackTypeId);
	DisplayData.RoleText = GetMetadataDisplayText(EntryData.RoleId);
	DisplayData.AttributeText = GetMetadataDisplayText(EntryData.AttributeId);
	DisplayData.StarGrade = EntryData.StarGrade;
	DisplayData.SortOrder = EntryData.SortOrder;
	DisplayData.DisplayName = EntryData.DisplayName;
	DisplayData.ShortDescription = EntryData.ShortDescription;
	DisplayData.DetailDescription = EntryData.DetailDescription;
	DisplayData.UnlockConditionText = FText::GetEmpty();
	DisplayData.RecordText = FText::GetEmpty();
	DisplayData.bIsNew = false;
	DisplayData.bCanShowFullData = true;
	DisplayData.IconAssetKey = EntryData.IconAssetKey;
	DisplayData.PreviewAssetKey = EntryData.PreviewAssetKey;
	DisplayData.AssetBundleName = EntryData.AssetBundleName;
	DisplayData.AccentColor = EntryData.AccentColor;
	return DisplayData;
}

bool UPBCollectionSubsystem::DoesEntryMatchQuery(
	const FPBCollectionEntryData& EntryData,
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
		|| ContainsSearchText(EntryData.ShortDescription, SearchText)
		|| ContainsSearchText(GetCategoryDisplayText(EntryData.Category), SearchText)
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
