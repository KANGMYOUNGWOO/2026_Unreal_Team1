#include "PBCollectionSubsystem.h"

#include "Internationalization/Text.h"
#include "Misc/DateTime.h"

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
	ResetDemoData();
}

void UPBCollectionSubsystem::ResetDemoData()
{
	BuildDemoEntries();
	BuildDemoProgress();
	OnCollectionEntryChanged.Broadcast(NAME_None);
}

void UPBCollectionSubsystem::ResetAllProgress()
{
	if (DemoEntries.IsEmpty())
	{
		BuildDemoEntries();
	}

	BuildLockedProgress();
	OnCollectionEntryChanged.Broadcast(NAME_None);
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

	for (const FPBCollectionEntryData& EntryData : DemoEntries)
	{
		const FPBCollectionProgressData* ProgressData = FindProgressData(EntryData.CollectionId);
		if (!ProgressData)
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
		switch (Query.SortMode)
		{
		case EPBCollectionSortMode::NameAsc:
			return A.DisplayName.ToString() < B.DisplayName.ToString();
		case EPBCollectionSortMode::StarGradeDesc:
			return A.StarGrade != B.StarGrade
				? A.StarGrade > B.StarGrade
				: A.SortOrder < B.SortOrder;
		case EPBCollectionSortMode::StarGradeAsc:
			return A.StarGrade != B.StarGrade
				? A.StarGrade < B.StarGrade
				: A.SortOrder < B.SortOrder;
		case EPBCollectionSortMode::StateDesc:
			return A.State != B.State
				? static_cast<uint8>(A.State) > static_cast<uint8>(B.State)
				: A.SortOrder < B.SortOrder;
		case EPBCollectionSortMode::SortOrder:
		default:
			return A.SortOrder < B.SortOrder;
		}
	});

	return Result;
}

bool UPBCollectionSubsystem::GetDisplayEntry(FName CollectionId, FPBCollectionDisplayData& OutDisplayData) const
{
	const FPBCollectionEntryData* EntryData = FindEntryData(CollectionId);
	const FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!EntryData || !ProgressData)
	{
		return false;
	}

	OutDisplayData = MakeDisplayData(*EntryData, *ProgressData);
	return true;
}

TArray<FName> UPBCollectionSubsystem::GetAvailableMetadataIds(EPBCollectionFilterField Field) const
{
	TSet<FName> UniqueIds;
	for (const FPBCollectionEntryData& EntryData : DemoEntries)
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
	for (const FPBCollectionEntryData& EntryData : DemoEntries)
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

bool UPBCollectionSubsystem::DiscoverEntry(FName CollectionId)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!ProgressData)
	{
		return false;
	}

	if (ProgressData->State == EPBCollectionState::Locked)
	{
		ProgressData->State = EPBCollectionState::Discovered;
		if (ProgressData->FirstDiscoveredAtText.IsEmpty())
		{
			ProgressData->FirstDiscoveredAtText = MakeNowText();
		}
		ProgressData->bIsNew = true;
		OnCollectionEntryChanged.Broadcast(CollectionId);
	}

	return true;
}

bool UPBCollectionSubsystem::UnlockEntry(FName CollectionId)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!ProgressData)
	{
		return false;
	}

	if (ProgressData->State == EPBCollectionState::Locked)
	{
		ProgressData->FirstDiscoveredAtText = MakeNowText();
	}

	if (ProgressData->State == EPBCollectionState::Locked || ProgressData->State == EPBCollectionState::Discovered)
	{
		ProgressData->State = EPBCollectionState::Unlocked;
		if (ProgressData->FirstUnlockedAtText.IsEmpty())
		{
			ProgressData->FirstUnlockedAtText = MakeNowText();
		}
		++ProgressData->AcquireCount;
		ProgressData->bIsNew = true;
		OnCollectionEntryChanged.Broadcast(CollectionId);
	}

	return true;
}

bool UPBCollectionSubsystem::CompleteEntry(FName CollectionId, const FString& CompletedByCharacterName)
{
	FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!ProgressData)
	{
		return false;
	}

	if (ProgressData->State != EPBCollectionState::Completed)
	{
		if (ProgressData->FirstDiscoveredAtText.IsEmpty())
		{
			ProgressData->FirstDiscoveredAtText = MakeNowText();
		}
		if (ProgressData->FirstUnlockedAtText.IsEmpty())
		{
			ProgressData->FirstUnlockedAtText = MakeNowText();
		}
		if (ProgressData->CompletedAtText.IsEmpty())
		{
			ProgressData->CompletedAtText = MakeNowText();
		}
	}

	ProgressData->State = EPBCollectionState::Completed;
	ProgressData->CompletedByCharacterName = CompletedByCharacterName.IsEmpty()
		? TEXT("검사 볼")
		: CompletedByCharacterName;
	ProgressData->DefeatCount = FMath::Max(1, ProgressData->DefeatCount + 1);
	ProgressData->bIsNew = true;

	OnCollectionEntryChanged.Broadcast(CollectionId);
	return true;
}

bool UPBCollectionSubsystem::AdvanceDemoState(FName CollectionId)
{
	const FPBCollectionProgressData* ProgressData = FindProgressData(CollectionId);
	if (!ProgressData)
	{
		return false;
	}

	switch (ProgressData->State)
	{
	case EPBCollectionState::Locked:
		return DiscoverEntry(CollectionId);
	case EPBCollectionState::Discovered:
		return UnlockEntry(CollectionId);
	case EPBCollectionState::Unlocked:
		return CompleteEntry(CollectionId, TEXT("검사 볼"));
	case EPBCollectionState::Completed:
		return true;
	default:
		return false;
	}
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

	return FText::FromName(MetadataId);
}

void UPBCollectionSubsystem::BuildDemoEntries()
{
	DemoEntries.Reset();

	AddDemoEntry(TEXT("BALL_SWORD"), EPBCollectionCategory::Ball,
		NSLOCTEXT("PBCollection", "BallSwordName", "검사 볼"),
		NSLOCTEXT("PBCollection", "BallSwordShort", "파워 플립으로 기본 피해를 안정적으로 넣는 리더 후보입니다."),
		NSLOCTEXT("PBCollection", "BallSwordDetail", "충돌 피해와 기본 공격을 담당하는 Ball입니다. 리더로 배치하면 파워 플립 피해가 안정적으로 증가합니다."),
		NSLOCTEXT("PBCollection", "BallSwordUnlock", "기본 지급"),
		TEXT("PowerFlip"),
		TEXT("Leader"),
		TEXT("Physical"),
		3,
		{ TEXT("Leader"), TEXT("PowerFlip"), TEXT("Attack") },
		10,
		FLinearColor(0.72f, 0.20f, 0.18f, 1.0f));

	AddDemoEntry(TEXT("BALL_FIGHTER"), EPBCollectionCategory::Ball,
		NSLOCTEXT("PBCollection", "BallFighterName", "격투 볼"),
		NSLOCTEXT("PBCollection", "BallFighterShort", "보스 그로기 게이지를 빠르게 압박합니다."),
		NSLOCTEXT("PBCollection", "BallFighterDetail", "약점 개방 타이밍에 강한 Ball입니다. 그로기 중심 Boss를 상대할 때 효율이 좋습니다."),
		NSLOCTEXT("PBCollection", "BallFighterUnlock", "훈련 보스 1회 처치"),
		TEXT("Melee"),
		TEXT("Groggy"),
		TEXT("Physical"),
		2,
		{ TEXT("Groggy"), TEXT("Melee") },
		20,
		FLinearColor(0.91f, 0.49f, 0.16f, 1.0f));

	AddDemoEntry(TEXT("BALL_SHOOTER"), EPBCollectionCategory::Ball,
		NSLOCTEXT("PBCollection", "BallShooterName", "사격 볼"),
		NSLOCTEXT("PBCollection", "BallShooterShort", "투사체형 파워 플립으로 원거리 피해를 줍니다."),
		NSLOCTEXT("PBCollection", "BallShooterDetail", "충돌 각도가 나쁘더라도 투사체로 보스를 견제할 수 있는 Ball입니다."),
		NSLOCTEXT("PBCollection", "BallShooterUnlock", "사격 시너지 2개 발견"),
		TEXT("Projectile"),
		TEXT("Damage"),
		TEXT("Fire"),
		4,
		{ TEXT("Projectile"), TEXT("Ranged") },
		30,
		FLinearColor(0.20f, 0.45f, 0.86f, 1.0f));

	AddDemoEntry(TEXT("BALL_SUPPORT"), EPBCollectionCategory::Ball,
		NSLOCTEXT("PBCollection", "BallSupportName", "보조 볼"),
		NSLOCTEXT("PBCollection", "BallSupportShort", "파티 유지력과 스킬 회전을 돕습니다."),
		NSLOCTEXT("PBCollection", "BallSupportDetail", "직접 피해보다는 버프와 회복 보조에 집중하는 Ball입니다."),
		NSLOCTEXT("PBCollection", "BallSupportUnlock", "보조 유물 1개 획득"),
		TEXT("Support"),
		TEXT("Support"),
		TEXT("Light"),
		3,
		{ TEXT("Support"), TEXT("Resource") },
		40,
		FLinearColor(0.16f, 0.64f, 0.40f, 1.0f));

	AddDemoEntry(TEXT("BUMPER_REBOUND"), EPBCollectionCategory::Bumper,
		NSLOCTEXT("PBCollection", "BumperReboundName", "리바운드 범퍼"),
		NSLOCTEXT("PBCollection", "BumperReboundShort", "충돌 각도를 바꿔 Ball을 다시 전투 구역으로 돌려보냅니다."),
		NSLOCTEXT("PBCollection", "BumperReboundDetail", "기본형 설치 범퍼입니다. 일정 횟수 충돌하면 추가 반발력과 소량 피해를 발생시킵니다."),
		NSLOCTEXT("PBCollection", "BumperReboundUnlock", "기본 해금"),
		TEXT("Collision"),
		TEXT("Controller"),
		TEXT("Neutral"),
		1,
		{ TEXT("Bumper"), TEXT("Rebound") },
		110,
		FLinearColor(0.18f, 0.56f, 0.78f, 1.0f));

	AddDemoEntry(TEXT("BUMPER_GATE"), EPBCollectionCategory::Bumper,
		NSLOCTEXT("PBCollection", "BumperGateName", "게이트 범퍼"),
		NSLOCTEXT("PBCollection", "BumperGateShort", "관문 통과를 조건으로 발동하는 설치형 범퍼입니다."),
		NSLOCTEXT("PBCollection", "BumperGateDetail", "Ball이 지정된 관문을 통과하면 발동 횟수를 누적하고, 조건 충족 시 보스에게 피해를 줍니다."),
		NSLOCTEXT("PBCollection", "BumperGateUnlock", "관문 트리거 5회 발동"),
		TEXT("Collision"),
		TEXT("Controller"),
		TEXT("Arcane"),
		2,
		{ TEXT("Bumper"), TEXT("Gate") },
		120,
		FLinearColor(0.38f, 0.48f, 0.24f, 1.0f));

	AddDemoEntry(TEXT("BUMPER_TURRET"), EPBCollectionCategory::Bumper,
		NSLOCTEXT("PBCollection", "BumperTurretName", "터렛 범퍼"),
		NSLOCTEXT("PBCollection", "BumperTurretShort", "소환체를 만들어 일정 시간 보스를 공격합니다."),
		NSLOCTEXT("PBCollection", "BumperTurretDetail", "충돌 조건을 만족하면 터렛을 소환합니다. 터렛은 짧은 시간 동안 보스를 향해 투사체를 발사합니다."),
		NSLOCTEXT("PBCollection", "BumperTurretUnlock", "소환형 범퍼 최초 발견"),
		TEXT("Summon"),
		TEXT("Damage"),
		TEXT("Fire"),
		3,
		{ TEXT("Bumper"), TEXT("Summon"), TEXT("Projectile") },
		130,
		FLinearColor(0.58f, 0.34f, 0.78f, 1.0f));

	AddDemoEntry(TEXT("BUMPER_SHIELD"), EPBCollectionCategory::Bumper,
		NSLOCTEXT("PBCollection", "BumperShieldName", "보호막 범퍼"),
		NSLOCTEXT("PBCollection", "BumperShieldShort", "리더 Ball의 생존을 돕는 지원형 범퍼입니다."),
		NSLOCTEXT("PBCollection", "BumperShieldDetail", "발동 시 리더 Ball에게 임시 보호막을 부여합니다. 보호 대상 범위는 팀 기획 확정이 필요합니다."),
		NSLOCTEXT("PBCollection", "BumperShieldUnlock", "지원형 범퍼 1회 장착"),
		TEXT("Support"),
		TEXT("Tank"),
		TEXT("Water"),
		2,
		{ TEXT("Bumper"), TEXT("Shield"), TEXT("Support") },
		140,
		FLinearColor(0.16f, 0.52f, 0.54f, 1.0f));

	AddDemoEntry(TEXT("BOSS_TRAINING"), EPBCollectionCategory::Boss,
		NSLOCTEXT("PBCollection", "BossTrainingName", "훈련 보스"),
		NSLOCTEXT("PBCollection", "BossTrainingShort", "도감과 전투 흐름 확인용 기본 보스입니다."),
		NSLOCTEXT("PBCollection", "BossTrainingDetail", "약점 개방과 그로기 흐름을 익히는 첫 보스입니다. 패턴은 단순하지만 도감 완료 기록의 기준이 됩니다."),
		NSLOCTEXT("PBCollection", "BossTrainingUnlock", "첫 전투 조우"),
		TEXT("Melee"),
		TEXT("Boss"),
		TEXT("Neutral"),
		2,
		{ TEXT("Boss"), TEXT("Tutorial") },
		210,
		FLinearColor(0.70f, 0.24f, 0.24f, 1.0f));

	AddDemoEntry(TEXT("BOSS_PART_BREAK"), EPBCollectionCategory::Boss,
		NSLOCTEXT("PBCollection", "BossPartBreakName", "부위 파괴 보스"),
		NSLOCTEXT("PBCollection", "BossPartBreakShort", "특정 부위를 파괴해 패턴을 약화시키는 보스입니다."),
		NSLOCTEXT("PBCollection", "BossPartBreakDetail", "부위 파괴 시 일부 패턴이 취소되거나 약화됩니다. 도감에는 약점 부위와 최초 처치 기록을 남깁니다."),
		NSLOCTEXT("PBCollection", "BossPartBreakUnlock", "훈련 보스 처치 후 등장"),
		TEXT("Pattern"),
		TEXT("Boss"),
		TEXT("Dark"),
		4,
		{ TEXT("Boss"), TEXT("PartBreak"), TEXT("Weakness") },
		220,
		FLinearColor(0.55f, 0.20f, 0.20f, 1.0f));

	AddDemoEntry(TEXT("BOSS_PATTERN"), EPBCollectionCategory::Boss,
		NSLOCTEXT("PBCollection", "BossPatternName", "패턴 보스"),
		NSLOCTEXT("PBCollection", "BossPatternShort", "보드 방해 패턴과 투사체를 섞어 사용하는 보스입니다."),
		NSLOCTEXT("PBCollection", "BossPatternDetail", "전투 중 보드 효율을 떨어뜨리는 패턴을 사용합니다. 도감에서는 주요 패턴과 공략 힌트를 단계적으로 공개합니다."),
		NSLOCTEXT("PBCollection", "BossPatternUnlock", "부위 파괴 보스 최초 조우"),
		TEXT("Projectile"),
		TEXT("Boss"),
		TEXT("Arcane"),
		5,
		{ TEXT("Boss"), TEXT("Pattern"), TEXT("Projectile") },
		230,
		FLinearColor(0.40f, 0.23f, 0.63f, 1.0f));

	AddDemoEntry(TEXT("RELIC_IMPACT"), EPBCollectionCategory::Relic,
		NSLOCTEXT("PBCollection", "RelicImpactName", "충돌 강화 유물"),
		NSLOCTEXT("PBCollection", "RelicImpactShort", "Ball의 충돌 피해를 올리는 기본 공격형 유물입니다."),
		NSLOCTEXT("PBCollection", "RelicImpactDetail", "Ball이 보스 또는 범퍼와 충돌할 때 피해 효율을 높입니다. 공격형 조합의 시작점입니다."),
		NSLOCTEXT("PBCollection", "RelicImpactUnlock", "누적 충돌 30회"),
		TEXT("Passive"),
		TEXT("Damage"),
		TEXT("Physical"),
		2,
		{ TEXT("Relic"), TEXT("Damage") },
		310,
		FLinearColor(0.76f, 0.38f, 0.14f, 1.0f));

	AddDemoEntry(TEXT("RELIC_MANA"), EPBCollectionCategory::Relic,
		NSLOCTEXT("PBCollection", "RelicManaName", "마나 회복 유물"),
		NSLOCTEXT("PBCollection", "RelicManaShort", "스킬 회전을 빠르게 만드는 자원형 유물입니다."),
		NSLOCTEXT("PBCollection", "RelicManaDetail", "충돌 또는 시간 경과로 MP 회복량을 증가시킵니다. 액티브 스킬 중심 조합에 어울립니다."),
		NSLOCTEXT("PBCollection", "RelicManaUnlock", "액티브 스킬 3회 사용"),
		TEXT("Passive"),
		TEXT("Resource"),
		TEXT("Water"),
		3,
		{ TEXT("Relic"), TEXT("Resource"), TEXT("Skill") },
		320,
		FLinearColor(0.18f, 0.42f, 0.72f, 1.0f));

	AddDemoEntry(TEXT("RELIC_BUMPER_HASTE"), EPBCollectionCategory::Relic,
		NSLOCTEXT("PBCollection", "RelicBumperHasteName", "범퍼 가속 유물"),
		NSLOCTEXT("PBCollection", "RelicBumperHasteShort", "범퍼 발동 주기를 앞당기는 설치형 지원 유물입니다."),
		NSLOCTEXT("PBCollection", "RelicBumperHasteDetail", "범퍼의 필요 충돌 횟수 또는 발동 대기시간을 줄이는 방향의 유물입니다. 실제 수치는 범퍼 기획 확정 후 조정합니다."),
		NSLOCTEXT("PBCollection", "RelicBumperHasteUnlock", "범퍼 3종 발견"),
		TEXT("Passive"),
		TEXT("Support"),
		TEXT("Nature"),
		4,
		{ TEXT("Relic"), TEXT("Bumper"), TEXT("Support") },
		330,
		FLinearColor(0.28f, 0.55f, 0.36f, 1.0f));
}

void UPBCollectionSubsystem::BuildDemoProgress()
{
	BuildLockedProgress();

	SetInitialProgress(TEXT("BALL_SWORD"), EPBCollectionState::Unlocked);
	SetInitialProgress(TEXT("BALL_FIGHTER"), EPBCollectionState::Discovered);
	SetInitialProgress(TEXT("BUMPER_REBOUND"), EPBCollectionState::Unlocked);
	SetInitialProgress(TEXT("BUMPER_GATE"), EPBCollectionState::Discovered);
	SetInitialProgress(TEXT("BOSS_TRAINING"), EPBCollectionState::Completed);
	SetInitialProgress(TEXT("BOSS_PART_BREAK"), EPBCollectionState::Discovered);
	SetInitialProgress(TEXT("RELIC_IMPACT"), EPBCollectionState::Unlocked);
	SetInitialProgress(TEXT("RELIC_MANA"), EPBCollectionState::Discovered);
}

void UPBCollectionSubsystem::BuildLockedProgress()
{
	ProgressMap.Reset();

	for (const FPBCollectionEntryData& EntryData : DemoEntries)
	{
		SetInitialProgress(EntryData.CollectionId, EPBCollectionState::Locked);
	}
}

void UPBCollectionSubsystem::AddDemoEntry(
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
	const FLinearColor& AccentColor)
{
	FPBCollectionEntryData EntryData;
	EntryData.CollectionId = CollectionId;
	EntryData.Category = Category;
	EntryData.DisplayName = DisplayName;
	EntryData.LockedName = NSLOCTEXT("PBCollection", "LockedName", "???");
	EntryData.ShortDescription = ShortDescription;
	EntryData.DetailDescription = DetailDescription;
	EntryData.UnlockConditionText = UnlockConditionText;
	EntryData.AttackTypeId = AttackTypeId;
	EntryData.RoleId = RoleId;
	EntryData.AttributeId = AttributeId;
	EntryData.StarGrade = StarGrade;
	EntryData.Tags = Tags;
	EntryData.SortOrder = SortOrder;
	EntryData.AccentColor = AccentColor;
	DemoEntries.Add(EntryData);
}

void UPBCollectionSubsystem::SetInitialProgress(FName CollectionId, EPBCollectionState State)
{
	FPBCollectionProgressData& ProgressData = ProgressMap.FindOrAdd(CollectionId);
	ProgressData.CollectionId = CollectionId;
	ProgressData.State = State;

	if (State != EPBCollectionState::Locked)
	{
		ProgressData.FirstDiscoveredAtText = TEXT("2026-06-30 15:00");
	}
	if (State == EPBCollectionState::Unlocked || State == EPBCollectionState::Completed)
	{
		ProgressData.FirstUnlockedAtText = TEXT("2026-06-30 15:05");
		ProgressData.AcquireCount = 1;
		ProgressData.UseCount = 2;
		ProgressData.BestCombo = 14;
		ProgressData.TotalDamage = 320;
	}
	if (State == EPBCollectionState::Completed)
	{
		ProgressData.CompletedAtText = TEXT("2026-06-30 15:12");
		ProgressData.CompletedByCharacterName = TEXT("검사 볼");
		ProgressData.DefeatCount = 1;
	}
}

const FPBCollectionEntryData* UPBCollectionSubsystem::FindEntryData(FName CollectionId) const
{
	return DemoEntries.FindByPredicate([CollectionId](const FPBCollectionEntryData& EntryData)
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
	FString Record;

	if (!ProgressData.FirstDiscoveredAtText.IsEmpty())
	{
		Record += FString::Printf(TEXT("최초 발견: %s\n"), *ProgressData.FirstDiscoveredAtText);
	}
	if (!ProgressData.FirstUnlockedAtText.IsEmpty())
	{
		Record += FString::Printf(TEXT("최초 해금: %s\n"), *ProgressData.FirstUnlockedAtText);
	}
	if (!ProgressData.CompletedAtText.IsEmpty())
	{
		Record += FString::Printf(TEXT("완료 시점: %s\n"), *ProgressData.CompletedAtText);
	}
	if (!ProgressData.CompletedByCharacterName.IsEmpty())
	{
		Record += FString::Printf(TEXT("완료 캐릭터: %s\n"), *ProgressData.CompletedByCharacterName);
	}

	Record += FString::Printf(TEXT("획득/사용: %d / %d\n"), ProgressData.AcquireCount, ProgressData.UseCount);
	Record += FString::Printf(TEXT("처치 횟수: %d\n"), ProgressData.DefeatCount);
	Record += FString::Printf(TEXT("최고 콤보: %d\n"), ProgressData.BestCombo);
	Record += FString::Printf(TEXT("누적 피해: %d"), ProgressData.TotalDamage);

	return FText::FromString(Record);
}

FString UPBCollectionSubsystem::MakeNowText()
{
	return FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M"));
}
