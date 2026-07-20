#include "PBCollectionSubsystem.h"

#include "Engine/DataTable.h"
#include "PinBallLike/DeveloperSettings/PBGameDataSettings.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallSkillTableRow.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossHitPointTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossPatternTableRow.h"
#include "PinBallLike/Table/Boss/Struct/PBBossTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperEffectRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTableRow.h"
#include "PinBallLike/Table/Bumper/Struct/PBBumperTriggerRow.h"
#include "PinBallLike/Table/Collection/Struct/PBCollectionTableRow.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicModifierRow.h"
#include "PinBallLike/Table/Relic/Struct/PBRelicTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"
#include "PinBallLike/Utils/PBTextFormatUtils.h"

#define LOCTEXT_NAMESPACE "PBCollectionCatalog"

namespace
{
template <typename EnumType>
FText GetEnumDisplayText(const EnumType Value)
{
	const UEnum* Enum = StaticEnum<EnumType>();
	return Enum
		? Enum->GetDisplayNameTextByValue(static_cast<int64>(Value))
		: FText::GetEmpty();
}

template <typename EnumType>
FText JoinEnumDisplayTexts(const TArray<EnumType>& Values)
{
	TArray<FString> Parts;
	Parts.Reserve(Values.Num());
	for (const EnumType Value : Values)
	{
		const FString Part = GetEnumDisplayText(Value).ToString();
		if (!Part.IsEmpty())
		{
			Parts.AddUnique(Part);
		}
	}

	return FText::FromString(FString::Join(Parts, TEXT(", ")));
}

bool IsSummaryLess(const FPBCollectionItemSummary& Left, const FPBCollectionItemSummary& Right)
{
	if (Left.SortOrder != Right.SortOrder)
	{
		return Left.SortOrder < Right.SortOrder;
	}

	const FString LeftName = Left.DisplayName.ToString();
	const FString RightName = Right.DisplayName.ToString();
	return LeftName == RightName
		? Left.SourceRowName.LexicalLess(Right.SourceRowName)
		: LeftName < RightName;
}

FLinearColor GetCategoryAccentColor(const EPBCollectionCategory Category)
{
	switch (Category)
	{
	case EPBCollectionCategory::Ball: return FLinearColor(0.88f, 0.35f, 0.35f, 1.0f);
	case EPBCollectionCategory::Synergy: return FLinearColor(0.27f, 0.70f, 0.61f, 1.0f);
	case EPBCollectionCategory::Relic: return FLinearColor(0.82f, 0.65f, 0.28f, 1.0f);
	case EPBCollectionCategory::Bumper: return FLinearColor(0.26f, 0.60f, 0.85f, 1.0f);
	case EPBCollectionCategory::Boss: return FLinearColor(0.69f, 0.36f, 0.76f, 1.0f);
	default: return FLinearColor::White;
	}
}

FText JoinLines(const TArray<FString>& Lines)
{
	return FText::FromString(FString::Join(Lines, LINE_TERMINATOR));
}

void AddIssue(
	TArray<FPBCollectionValidationIssue>& Issues,
	const EPBCollectionValidationSeverity Severity,
	const FName Topic,
	const FName SourceRowName,
	const FText& Message)
{
	FPBCollectionValidationIssue& Issue = Issues.AddDefaulted_GetRef();
	Issue.Severity = Severity;
	Issue.Topic = Topic;
	Issue.SourceRowName = SourceRowName;
	Issue.Message = Message;
}

bool ContainsIndexedPlaceholder(const FText& Text)
{
	const FString Value = Text.ToString();
	for (int32 Index = 0; Index < 10; ++Index)
	{
		if (Value.Contains(FString::Printf(TEXT("{%d}"), Index)))
		{
			return true;
		}
	}

	return false;
}

bool IsCurrentCatalogCategory(const EPBCollectionCategory Category)
{
	return Category == EPBCollectionCategory::Ball
		|| Category == EPBCollectionCategory::Synergy
		|| Category == EPBCollectionCategory::Relic
		|| Category == EPBCollectionCategory::Bumper
		|| Category == EPBCollectionCategory::Boss;
}
}

const FPBCollectionEntryData* UPBCollectionSubsystem::FindEntryDataBySourceRow(
	const EPBCollectionCategory Category,
	const FName SourceRowName) const
{
	if (SourceRowName.IsNone())
	{
		return nullptr;
	}

	return Entries.FindByPredicate(
		[Category, SourceRowName](const FPBCollectionEntryData& Entry)
		{
			return Entry.Category == Category && Entry.SourceRowName == SourceRowName;
		});
}

FPBCollectionItemSummary UPBCollectionSubsystem::MakeItemSummary(
	const EPBCollectionCategory Category,
	const FName SourceRowName,
	const FText& SourceDisplayName,
	const FText& SourceDescription,
	const int32 DefaultSortOrder) const
{
	FPBCollectionItemSummary Summary;
	Summary.ItemId = SourceRowName;
	Summary.SourceRowName = SourceRowName;
	Summary.DisplayName = SourceDisplayName.IsEmpty() ? FText::FromName(SourceRowName) : SourceDisplayName;
	Summary.Description = SourceDescription;
	Summary.SortOrder = DefaultSortOrder;
	Summary.AccentColor = GetCategoryAccentColor(Category);

	if (const FPBCollectionEntryData* Metadata = FindEntryDataBySourceRow(Category, SourceRowName))
	{
		Summary.ItemId = Metadata->CollectionId.IsNone() ? SourceRowName : Metadata->CollectionId;
		if (!Metadata->DisplayName.IsEmpty())
		{
			Summary.DisplayName = Metadata->DisplayName;
		}
		Summary.Subtitle = Metadata->ShortDescription;
		if (!Metadata->DetailDescription.IsEmpty())
		{
			Summary.Description = Metadata->DetailDescription;
		}
		Summary.IconAssetKey = Metadata->IconAssetKey;
		Summary.PreviewAssetKey = Metadata->PreviewAssetKey;
		Summary.AssetBundleName = Metadata->AssetBundleName;
		if (Metadata->AccentColor != FLinearColor::White)
		{
			Summary.AccentColor = Metadata->AccentColor;
		}
		Summary.SortOrder = Metadata->SortOrder;
	}

	return Summary;
}

TArray<FPBCollectionBallDisplayData> UPBCollectionSubsystem::GetBallCatalogEntries() const
{
	TArray<FPBCollectionBallDisplayData> Result;
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableData = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!IsValid(TableData))
	{
		return Result;
	}

	TArray<FName> BallRowNames;
	if (const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>())
	{
		if (const UDataTable* BallTable = Settings->BallTable.Get())
		{
			BallRowNames = BallTable->GetRowNames();
		}
	}

	if (BallRowNames.IsEmpty())
	{
		for (const FPBCollectionEntryData& Entry : Entries)
		{
			if (Entry.Category == EPBCollectionCategory::Ball && !Entry.SourceRowName.IsNone())
			{
				BallRowNames.AddUnique(Entry.SourceRowName);
			}
		}
	}

	for (const FName BallRowName : BallRowNames)
	{
		FPBBallTableRow BallRow;
		if (!TableData->FindBallRow(BallRowName, BallRow))
		{
			continue;
		}

		FPBCollectionBallDisplayData& DisplayData = Result.AddDefaulted_GetRef();
		DisplayData.Summary = MakeItemSummary(
			EPBCollectionCategory::Ball,
			BallRowName,
			BallRow.DisplayName,
			BallRow.DescriptionKey);
		DisplayData.PowerFlipTypeText = GetEnumDisplayText(BallRow.PowerFlipType);
		DisplayData.RaceTypesText = JoinEnumDisplayTexts(BallRow.RaceTypes);
		DisplayData.ClassTypeText = GetEnumDisplayText(BallRow.ClassType);

		if (BallRow.DefaultSkillIds.Num() != 1)
		{
			DisplayData.ValidationText = FText::Format(
				LOCTEXT("BallSkillCountInvalid", "스킬 참조가 {0}개입니다. 볼 하나에는 스킬 하나만 지정해야 합니다."),
				FText::AsNumber(BallRow.DefaultSkillIds.Num()));
		}

		if (!BallRow.DefaultSkillIds.IsEmpty())
		{
			DisplayData.SkillId = BallRow.DefaultSkillIds[0];
			FPBBallSkillTableRow SkillRow;
			if (TableData->FindSkillRow(DisplayData.SkillId, SkillRow))
			{
				DisplayData.SkillName = SkillRow.DisplayName;
				DisplayData.SkillDescription = SkillRow.Description;
				//DisplayData.SkillStatsText = FText::Format(
				//	LOCTEXT("BallSkillStats", "피해 배율 {0} / 지속 {1}초 / 값 {2}"),
				//	FText::AsNumber(SkillRow.DamageMultiplier),
				//	FText::AsNumber(SkillRow.Duration),
				//	FText::AsNumber(SkillRow.Value));
				DisplayData.bHasValidSkill = BallRow.DefaultSkillIds.Num() == 1;
			}
			else
			{
				DisplayData.ValidationText = FText::Format(
					LOCTEXT("BallSkillMissing", "참조한 스킬 {0}을(를) Skill 테이블에서 찾을 수 없습니다."),
					FText::FromName(DisplayData.SkillId));
			}
		}
	}

	Result.Sort([](const FPBCollectionBallDisplayData& Left, const FPBCollectionBallDisplayData& Right)
	{
		return IsSummaryLess(Left.Summary, Right.Summary);
	});
	return Result;
}

TArray<FPBCollectionSynergyDisplayData> UPBCollectionSubsystem::GetSynergyCatalogEntries() const
{
	TArray<FPBCollectionSynergyDisplayData> Result;
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableData = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!IsValid(TableData))
	{
		return Result;
	}

	TArray<FName> RowNames;
	TArray<FPBSynergyTableRow> Rows;
	if (!TableData->GetAllSynergyRows(RowNames, Rows))
	{
		return Result;
	}

	const int32 RowCount = FMath::Min(RowNames.Num(), Rows.Num());
	for (int32 Index = 0; Index < RowCount; ++Index)
	{
		const FName SynergyRowName = RowNames[Index];
		const FPBSynergyTableRow& SynergyRow = Rows[Index];
		FPBCollectionSynergyDisplayData& DisplayData = Result.AddDefaulted_GetRef();
		DisplayData.Summary = MakeItemSummary(
			EPBCollectionCategory::Synergy,
			SynergyRowName,
			SynergyRow.DisplayName,
			FText::GetEmpty(),
			SynergyRow.SortOrder);
		DisplayData.SynergyKindText = GetEnumDisplayText(SynergyRow.SynergyKind);
		DisplayData.RuleTypeText = GetEnumDisplayText(SynergyRow.RuleType);
		DisplayData.Summary.Subtitle = DisplayData.SynergyKindText;

		TArray<FPBSynergyTierRow> TierRows;
		TableData->GetSynergyTierRows(SynergyRowName, TierRows);
		for (const FPBSynergyTierRow& TierRow : TierRows)
		{
			FPBCollectionSynergyTierDisplayData& TierData = DisplayData.Tiers.AddDefaulted_GetRef();
			TierData.RequiredCount = TierRow.RequiredCount;
			TierData.EffectId = TierRow.EffectSetId;
			TierData.TierDescription = FText::Format(
				LOCTEXT("SynergyTierCount", "{0}개 구성"),
				FText::AsNumber(TierRow.RequiredCount));

			// FPBSynergyEffectRow EffectRow;
			// if (!TableData->FindSynergyEffectRow(TierRow.SynergyEffectId, EffectRow))
			// {
			// 	TierData.EffectSummary = LOCTEXT("MissingSynergyEffect", "효과 정보를 찾을 수 없습니다.");
			// 	continue;
			// }
			//
			// TierData.bHasValidEffect = true;
			// TierData.EffectSummary = FText::Format(
			// 	LOCTEXT("SynergyEffectSummary", "중첩 {0} / 지속 {1} ({2}) / 간격 {3}초"),
			// 	GetEnumDisplayText(EffectRow.StackType),
			// 	GetEnumDisplayText(EffectRow.DurationPolicy),
			// 	FText::AsNumber(EffectRow.DurationValue),
			// 	FText::AsNumber(EffectRow.Interval));
			//
			// TArray<FPBSynergyEffectModifierRow> ModifierRows;
			// TableData->GetSynergyEffectModifierRows(TierRow.SynergyEffectId, ModifierRows);
			// TArray<FString> ModifierLines;
			// for (const FPBSynergyEffectModifierRow& Modifier : ModifierRows)
			// {
			// 	ModifierLines.Add(FString::Printf(
			// 		TEXT("%s %g%s"),
			// 		*GetEnumDisplayText(Modifier.ModifyType).ToString(),
			// 		Modifier.Value,
			// 		Modifier.bScaleWithStack ? TEXT(" (중첩 비례)") : TEXT("")));
			// }
			// TierData.ModifierSummary = JoinLines(ModifierLines);
			//
			// TArray<FPBSynergyEffectTriggerRow> TriggerRows;
			// TableData->GetSynergyEffectTriggerRows(TierRow.SynergyEffectId, TriggerRows);
			// TArray<FString> TriggerLines;
			// for (const FPBSynergyEffectTriggerRow& Trigger : TriggerRows)
			// {
			// 	TriggerLines.Add(FString::Printf(
			// 		TEXT("발동 확률 %g%% / 값 %g%s"),
			// 		Trigger.ProcChance,
			// 		Trigger.Value,
			// 		Trigger.bScaleWithStack ? TEXT(" (중첩 비례)") : TEXT("")));
			// }
			// TierData.TriggerSummary = JoinLines(TriggerLines);
		}
	}

	Result.Sort([](const FPBCollectionSynergyDisplayData& Left, const FPBCollectionSynergyDisplayData& Right)
	{
		return IsSummaryLess(Left.Summary, Right.Summary);
	});
	return Result;
}

TArray<FPBCollectionRelicDisplayData> UPBCollectionSubsystem::GetRelicCatalogEntries() const
{
	TArray<FPBCollectionRelicDisplayData> Result;
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableData = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!IsValid(TableData))
	{
		return Result;
	}

	TArray<FName> RelicIds;
	TableData->GetAllRelicIds(RelicIds);
	for (const FName RelicId : RelicIds)
	{
		FPBRelicTableRow RelicRow;
		if (!TableData->FindRelicRow(RelicId, RelicRow))
		{
			continue;
		}

		FPBCollectionRelicDisplayData& DisplayData = Result.AddDefaulted_GetRef();
		DisplayData.Summary = MakeItemSummary(
			EPBCollectionCategory::Relic,
			RelicId,
			RelicRow.DisplayName,
			RelicRow.Description);
		DisplayData.RarityText = GetEnumDisplayText(RelicRow.Rarity);
		DisplayData.Summary.Subtitle = DisplayData.RarityText;

		TArray<FPBRelicModifierRow> ModifierRows;
		TableData->GetRelicModifierRows(RelicId, ModifierRows);
		TArray<FString> ModifierLines;
		for (const FPBRelicModifierRow& Modifier : ModifierRows)
		{
			ModifierLines.Add(FString::Printf(
				TEXT("%s · %s · %s %g"),
				*GetEnumDisplayText(Modifier.TargetType).ToString(),
				*Modifier.TargetStat.ToString(),
				*GetEnumDisplayText(Modifier.ModifyType).ToString(),
				Modifier.Value));
		}
		DisplayData.ModifierSummary = JoinLines(ModifierLines);
	}

	Result.Sort([](const FPBCollectionRelicDisplayData& Left, const FPBCollectionRelicDisplayData& Right)
	{
		return IsSummaryLess(Left.Summary, Right.Summary);
	});
	return Result;
}

TArray<FPBCollectionBumperDisplayData> UPBCollectionSubsystem::GetBumperCatalogEntries() const
{
	TArray<FPBCollectionBumperDisplayData> Result;
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableData = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!IsValid(TableData))
	{
		return Result;
	}

	TArray<FName> RowNames;
	TArray<FPBBumperTableRow> Rows;
	if (!TableData->GetAllBumperRows(RowNames, Rows))
	{
		return Result;
	}

	const int32 RowCount = FMath::Min(RowNames.Num(), Rows.Num());
	for (int32 Index = 0; Index < RowCount; ++Index)
	{
		const FName BumperRowName = RowNames[Index];
		const FPBBumperTableRow& BumperRow = Rows[Index];
		FPBCollectionBumperDisplayData& DisplayData = Result.AddDefaulted_GetRef();
		DisplayData.Summary = MakeItemSummary(
			EPBCollectionCategory::Bumper,
			BumperRowName,
			BumperRow.DisplayName,
			BumperRow.Description);
		DisplayData.Summary.Description = PBTextFormatUtils::FormatSingleValueTemplate(
			DisplayData.Summary.Description,
			FText::AsNumber(BumperRow.RequiredTriggerCount));
		DisplayData.BumperTypeText = GetEnumDisplayText(BumperRow.BumperType);
		DisplayData.RoleTypeText = GetEnumDisplayText(BumperRow.RoleType);
		DisplayData.EffectTypeText = GetEnumDisplayText(BumperRow.EffectType);
		DisplayData.RequiredTriggerCount = BumperRow.RequiredTriggerCount;
		DisplayData.Summary.Subtitle = FText::Format(
			LOCTEXT("BumperSubtitle", "{0} · {1}"),
			DisplayData.BumperTypeText,
			DisplayData.RoleTypeText);

		FPBBumperTriggerRow TriggerRow;
		if (TableData->FindBumperTriggerRow(BumperRow.TriggerID, TriggerRow))
		{
			DisplayData.bHasValidTrigger = true;
			DisplayData.TriggerTypeText = GetEnumDisplayText(TriggerRow.TriggerType);
			DisplayData.PositionText = JoinEnumDisplayTexts(TriggerRow.PositionIds);
			DisplayData.TriggerDescription = PBTextFormatUtils::FormatSingleValueTemplate(
				TriggerRow.TriggerDescription,
				FText::AsNumber(BumperRow.RequiredTriggerCount));
		}

		FPBBumperEffectRow EffectRow;
		if (TableData->FindBumperEffectRow(BumperRow.EffectID, EffectRow))
		{
			DisplayData.bHasValidEffect = true;
			DisplayData.ExecutionPolicyText = GetEnumDisplayText(EffectRow.ExecutionPolicy);
			DisplayData.EffectDescription = PBTextFormatUtils::FormatSingleValueTemplate(
				EffectRow.Description,
				FText::AsNumber(EffectRow.Power));
			DisplayData.EffectPower = EffectRow.Power;
		}
	}

	Result.Sort([](const FPBCollectionBumperDisplayData& Left, const FPBCollectionBumperDisplayData& Right)
	{
		return IsSummaryLess(Left.Summary, Right.Summary);
	});
	return Result;
}

TArray<FPBCollectionBossDisplayData> UPBCollectionSubsystem::GetBossCatalogEntries() const
{
	TArray<FPBCollectionBossDisplayData> Result;
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableData = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!IsValid(TableData))
	{
		return Result;
	}

	TArray<FName> BossRowNames;
	if (!TableData->GetBossRowNames(BossRowNames))
	{
		return Result;
	}

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	const UDataTable* HitPointTable = Settings ? Settings->BossHitPoint.Get() : nullptr;
	const UDataTable* PatternTable = Settings ? Settings->BossPattern.Get() : nullptr;
	const TArray<FName> HitPointRowNames = HitPointTable ? HitPointTable->GetRowNames() : TArray<FName>();
	const TArray<FName> PatternRowNames = PatternTable ? PatternTable->GetRowNames() : TArray<FName>();

	for (const FName BossRowName : BossRowNames)
	{
		FPBBossTableRow BossRow;
		if (!TableData->FindBossRow(BossRowName, BossRow))
		{
			continue;
		}

		FPBCollectionBossDisplayData& DisplayData = Result.AddDefaulted_GetRef();
		DisplayData.Summary = MakeItemSummary(
			EPBCollectionCategory::Boss,
			BossRowName,
			BossRow.DisplayName,
			FText::GetEmpty());
		DisplayData.MaxHP = BossRow.MaxHP;
		DisplayData.MaxGroggyGauge = BossRow.MaxGroggyGauge;
		DisplayData.GroggyDurationSeconds = BossRow.GroggyDurationSeconds;
		DisplayData.EnrageHPRatioPercent = BossRow.EnrageHPRatioPercent;

		for (const FName HitPointRowName : HitPointRowNames)
		{
			FPBBossHitPointTableRow HitPointRow;
			if (!TableData->FindBossHitPointRow(HitPointRowName, HitPointRow)
				|| HitPointRow.BossRowName != BossRowName)
			{
				continue;
			}

			FPBCollectionBossHitPointDisplayData& HitPoint = DisplayData.HitPoints.AddDefaulted_GetRef();
			HitPoint.Name = FText::FromName(HitPointRow.HitPointName);
			HitPoint.PartTypeText = GetEnumDisplayText(HitPointRow.HitPartType);
			HitPoint.DamageMultiplierPercent = HitPointRow.HPDamageMultiplierPercent;
			HitPoint.GroggyMultiplierPercent = HitPointRow.GroggyMultiplierPercent;
			HitPoint.bIsWeaknessPoint = HitPointRow.IsWeaknessPoint;
		}

		for (const FName PatternRowName : PatternRowNames)
		{
			FPBBossPatternTableRow PatternRow;
			if (!TableData->FindBossPatternRow(PatternRowName, PatternRow)
				|| PatternRow.BossRowName != BossRowName
				|| !PatternRow.IsEnabled)
			{
				continue;
			}

			FPBCollectionBossPatternDisplayData& Pattern = DisplayData.Patterns.AddDefaulted_GetRef();
			Pattern.Name = FText::FromName(PatternRow.PatternName);
			Pattern.PhaseTypeText = GetEnumDisplayText(PatternRow.PatternPhaseType);
			Pattern.CooldownSeconds = PatternRow.CooldownSeconds;
		}
	}

	Result.Sort([](const FPBCollectionBossDisplayData& Left, const FPBCollectionBossDisplayData& Right)
	{
		return IsSummaryLess(Left.Summary, Right.Summary);
	});
	return Result;
}

TArray<FPBCollectionValidationIssue> UPBCollectionSubsystem::GetCatalogValidationIssues() const
{
	TArray<FPBCollectionValidationIssue> Issues;
	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableData = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!IsValid(TableData))
	{
		AddIssue(
			Issues,
			EPBCollectionValidationSeverity::Error,
			TEXT("CollectionDataSource"),
			NAME_None,
			LOCTEXT("MissingCollectionDataSource", "원본 게임 테이블 서브시스템을 찾을 수 없습니다."));
		return Issues;
	}

	TArray<FPBCollectionTableRow> CollectionRows;
	TableData->GetAllCollectionRows(CollectionRows);

	TSet<FName> CollectionIds;
	for (const FPBCollectionTableRow& CollectionRow : CollectionRows)
	{
		const FPBCollectionEntryData Metadata = CollectionRow.ToEntryData();
		if (Metadata.CollectionId.IsNone())
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("CollectionMetadata"),
				Metadata.SourceRowName,
				LOCTEXT("MissingCollectionId", "도감 메타데이터의 CollectionId가 비어 있습니다."));
			continue;
		}

		if (CollectionIds.Contains(Metadata.CollectionId))
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("CollectionMetadata"),
				Metadata.SourceRowName,
				FText::Format(
					LOCTEXT("DuplicateCollectionId", "CollectionId {0}이(가) 중복되었습니다."),
					FText::FromName(Metadata.CollectionId)));
		}
		CollectionIds.Add(Metadata.CollectionId);
	}

	TSet<FString> MetadataSourceKeys;
	for (const FPBCollectionEntryData& Metadata : Entries)
	{
		if (!IsCurrentCatalogCategory(Metadata.Category))
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("CollectionMetadata"),
				Metadata.SourceRowName,
				LOCTEXT("UnsupportedCollectionCategory", "현재 5개 도감 탭에서 사용하지 않는 카테고리입니다."));
		}

		if (Metadata.SourceRowName.IsNone())
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("CollectionMetadata"),
				NAME_None,
				FText::Format(
					LOCTEXT("MissingMetadataSourceRowName", "도감 메타데이터 {0}에 SourceRowName이 없습니다."),
					FText::FromName(Metadata.CollectionId)));
			continue;
		}

		const FString MetadataSourceKey = FString::Printf(
			TEXT("%d:%s"),
			static_cast<int32>(Metadata.Category),
			*Metadata.SourceRowName.ToString());
		if (MetadataSourceKeys.Contains(MetadataSourceKey))
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("CollectionMetadata"),
				Metadata.SourceRowName,
				LOCTEXT("DuplicateMetadataSource", "같은 카테고리와 원본 RowName을 가리키는 메타데이터가 중복되었습니다."));
		}
		MetadataSourceKeys.Add(MetadataSourceKey);

		bool bHasSourceRow = true;
		switch (Metadata.Category)
		{
		case EPBCollectionCategory::Ball:
		{
			FPBBallTableRow Row;
			bHasSourceRow = TableData->FindBallRow(Metadata.SourceRowName, Row);
			break;
		}
		case EPBCollectionCategory::Synergy:
		{
			FPBSynergyTableRow Row;
			bHasSourceRow = TableData->FindSynergyRow(Metadata.SourceRowName, Row);
			break;
		}
		case EPBCollectionCategory::Relic:
		{
			FPBRelicTableRow Row;
			bHasSourceRow = TableData->FindRelicRow(Metadata.SourceRowName, Row);
			break;
		}
		case EPBCollectionCategory::Bumper:
		{
			FPBBumperTableRow Row;
			bHasSourceRow = TableData->FindBumperRow(Metadata.SourceRowName, Row);
			break;
		}
		case EPBCollectionCategory::Boss:
		{
			FPBBossTableRow Row;
			bHasSourceRow = TableData->FindBossRow(Metadata.SourceRowName, Row);
			break;
		}
		default:
			continue;
		}

		if (!bHasSourceRow)
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("CollectionMetadata"),
				Metadata.SourceRowName,
				FText::Format(
					LOCTEXT("MissingMetadataSourceRow", "도감 메타데이터 {0}이(가) 존재하지 않는 원본 행 {1}을(를) 참조합니다."),
					FText::FromName(Metadata.CollectionId),
					FText::FromName(Metadata.SourceRowName)));
		}
	}

	for (const FPBCollectionBallDisplayData& Ball : GetBallCatalogEntries())
	{
		if (!Ball.bHasValidSkill)
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("Ball"),
				Ball.Summary.SourceRowName,
				Ball.ValidationText.IsEmpty()
					? LOCTEXT("InvalidBallSkill", "유효한 단일 스킬 참조가 없습니다.")
					: Ball.ValidationText);
		}
		else if (ContainsIndexedPlaceholder(Ball.SkillDescription))
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("BallSkillDescription"),
				Ball.Summary.SourceRowName,
				FText::Format(
					LOCTEXT("UnresolvedSkillPlaceholder", "스킬 {0} 설명에 치환 규칙이 없는 숫자 자리표시자가 남아 있습니다."),
					FText::FromName(Ball.SkillId)));
		}
	}

	for (const FPBCollectionSynergyDisplayData& Synergy : GetSynergyCatalogEntries())
	{
		if (Synergy.Tiers.IsEmpty())
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("Synergy"),
				Synergy.Summary.SourceRowName,
				LOCTEXT("MissingSynergyTiers", "시너지 단계가 하나도 없습니다."));
		}

		int32 PreviousCount = INDEX_NONE;
		for (const FPBCollectionSynergyTierDisplayData& Tier : Synergy.Tiers)
		{
			if (Tier.RequiredCount == PreviousCount)
			{
				AddIssue(
					Issues,
					EPBCollectionValidationSeverity::Error,
					TEXT("Synergy"),
					Synergy.Summary.SourceRowName,
					FText::Format(
						LOCTEXT("DuplicateSynergyTier", "요구 수치 {0} 단계가 중복되었습니다."),
						FText::AsNumber(Tier.RequiredCount)));
			}
			PreviousCount = Tier.RequiredCount;

			if (!Tier.bHasValidEffect)
			{
				AddIssue(
					Issues,
					EPBCollectionValidationSeverity::Error,
					TEXT("SynergyEffect"),
					Synergy.Summary.SourceRowName,
					FText::Format(
						LOCTEXT("MissingSynergyEffectIssue", "{0}개 단계가 존재하지 않는 Effect {1}을(를) 참조합니다."),
						FText::AsNumber(Tier.RequiredCount),
						FText::FromName(Tier.EffectId)));
			}
		}
	}

	for (const FPBCollectionBumperDisplayData& Bumper : GetBumperCatalogEntries())
	{
		if (!Bumper.bHasValidTrigger)
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("BumperTrigger"),
				Bumper.Summary.SourceRowName,
				LOCTEXT("MissingBumperTrigger", "연결된 Trigger Row를 찾을 수 없습니다."));
		}
		if (!Bumper.bHasValidEffect)
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("BumperEffect"),
				Bumper.Summary.SourceRowName,
				LOCTEXT("MissingBumperEffect", "연결된 Effect Row를 찾을 수 없습니다."));
		}
		if (ContainsIndexedPlaceholder(Bumper.Summary.Description)
			|| ContainsIndexedPlaceholder(Bumper.TriggerDescription)
			|| ContainsIndexedPlaceholder(Bumper.EffectDescription))
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("BumperDescription"),
				Bumper.Summary.SourceRowName,
				LOCTEXT("UnresolvedBumperPlaceholder", "범퍼 설명에 치환되지 않은 {n} 자리표시자가 남아 있습니다."));
		}
		if (Bumper.RequiredTriggerCount <= 0)
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("BumperTrigger"),
				Bumper.Summary.SourceRowName,
				LOCTEXT("InvalidBumperTriggerCount", "범퍼의 필요 충돌 횟수는 1 이상이어야 합니다."));
		}
		if (Bumper.bHasValidEffect && (!FMath::IsFinite(Bumper.EffectPower) || Bumper.EffectPower <= 0.0f))
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Error,
				TEXT("BumperEffect"),
				Bumper.Summary.SourceRowName,
				LOCTEXT("InvalidBumperEffectPower", "범퍼 Effect Power는 유한한 양수여야 합니다."));
		}
	}

	const UPBGameDataSettings* Settings = GetDefault<UPBGameDataSettings>();
	if (const UDataTable* RelicModifierTable = Settings ? Settings->RelicModifierTable.Get() : nullptr)
	{
		RelicModifierTable->ForeachRow<FPBRelicModifierRow>(
			TEXT("CollectionRelicValidation"),
			[&Issues, TableData](const FName& RowName, const FPBRelicModifierRow& ModifierRow)
			{
				FPBRelicTableRow RelicRow;
				if (ModifierRow.RelicId.IsNone() || !TableData->FindRelicRow(ModifierRow.RelicId, RelicRow))
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Error,
						TEXT("RelicModifier"),
						RowName,
						FText::Format(
							LOCTEXT("MissingRelicForModifier", "Modifier가 존재하지 않는 Relic {0}을(를) 참조합니다."),
							FText::FromName(ModifierRow.RelicId)));
				}
				if (ModifierRow.TargetStat.IsNone())
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Warning,
						TEXT("RelicModifier"),
						RowName,
						LOCTEXT("MissingRelicTargetStat", "Relic Modifier의 TargetStat이 비어 있습니다."));
				}
				if (!FMath::IsFinite(ModifierRow.Value))
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Error,
						TEXT("RelicModifier"),
						RowName,
						LOCTEXT("InvalidRelicModifierValue", "Relic Modifier Value가 유효한 숫자가 아닙니다."));
				}
			});
	}

	for (const FPBCollectionBossDisplayData& Boss : GetBossCatalogEntries())
	{
		if (Boss.HitPoints.IsEmpty())
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("BossHitPoint"),
				Boss.Summary.SourceRowName,
				LOCTEXT("MissingBossHitPoints", "표시할 보스 부위 정보가 없습니다."));
		}
		if (Boss.Patterns.IsEmpty())
		{
			AddIssue(
				Issues,
				EPBCollectionValidationSeverity::Warning,
				TEXT("BossPattern"),
				Boss.Summary.SourceRowName,
				LOCTEXT("MissingBossPatterns", "표시할 활성 보스 패턴이 없습니다."));
		}
	}

	if (const UDataTable* HitPointTable = Settings ? Settings->BossHitPoint.Get() : nullptr)
	{
		HitPointTable->ForeachRow<FPBBossHitPointTableRow>(
			TEXT("CollectionBossHitPointValidation"),
			[&Issues, TableData](const FName& RowName, const FPBBossHitPointTableRow& HitPointRow)
			{
				FPBBossTableRow BossRow;
				if (HitPointRow.BossRowName.IsNone() || !TableData->FindBossRow(HitPointRow.BossRowName, BossRow))
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Error,
						TEXT("BossHitPoint"),
						RowName,
						FText::Format(
							LOCTEXT("MissingBossForHitPoint", "부위가 존재하지 않는 Boss {0}을(를) 참조합니다."),
							FText::FromName(HitPointRow.BossRowName)));
				}
				if (HitPointRow.HitPointName.IsNone())
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Error,
						TEXT("BossHitPoint"),
						RowName,
						LOCTEXT("MissingBossHitPointName", "보스 부위의 HitPointName이 비어 있습니다."));
				}
			});
	}

	if (const UDataTable* PatternTable = Settings ? Settings->BossPattern.Get() : nullptr)
	{
		PatternTable->ForeachRow<FPBBossPatternTableRow>(
			TEXT("CollectionBossPatternValidation"),
			[&Issues, TableData](const FName& RowName, const FPBBossPatternTableRow& PatternRow)
			{
				FPBBossTableRow BossRow;
				if (PatternRow.BossRowName.IsNone() || !TableData->FindBossRow(PatternRow.BossRowName, BossRow))
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Error,
						TEXT("BossPattern"),
						RowName,
						FText::Format(
							LOCTEXT("MissingBossForPattern", "패턴이 존재하지 않는 Boss {0}을(를) 참조합니다."),
							FText::FromName(PatternRow.BossRowName)));
				}
				if (PatternRow.IsEnabled && (PatternRow.PatternName.IsNone() || PatternRow.PatternClassID.IsNone()))
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Error,
						TEXT("BossPattern"),
						RowName,
						LOCTEXT("IncompleteEnabledBossPattern", "활성 보스 패턴에는 PatternName과 PatternClassID가 모두 필요합니다."));
				}
				if (!FMath::IsFinite(PatternRow.CooldownSeconds) || PatternRow.CooldownSeconds < 0.0f)
				{
					AddIssue(
						Issues,
						EPBCollectionValidationSeverity::Error,
						TEXT("BossPattern"),
						RowName,
						LOCTEXT("InvalidBossPatternCooldown", "보스 패턴 CooldownSeconds는 0 이상의 유한한 값이어야 합니다."));
				}
			});
	}

	return Issues;
}

#undef LOCTEXT_NAMESPACE
