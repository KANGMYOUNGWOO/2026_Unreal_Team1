#include "PBCollectionTabWidgets.h"

#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "PinBallLike/Collection/PBCollectionSubsystem.h"
#include "PBCollectionCatalogItemObject.h"

#define LOCTEXT_NAMESPACE "PBCollectionTabs"

namespace
{
void SetText(UTextBlock* TextBlock, const FText& Text)
{
	if (TextBlock)
	{
		TextBlock->SetText(Text);
	}
}

FText JoinTexts(const TArray<FText>& Texts)
{
	TArray<FString> Lines;
	for (const FText& Text : Texts)
	{
		if (!Text.IsEmpty())
		{
			Lines.Add(Text.ToString());
		}
	}
	return FText::FromString(FString::Join(Lines, LINE_TERMINATOR));
}
}

void UPBCollectionBallTabWidget::RefreshTab()
{
	UPBCollectionSubsystem* Subsystem = GetCollectionSubsystem();
	if (!PrepareCatalogRefresh(BallTileView, LOCTEXT("BallLoading", "볼 데이터를 준비하는 중입니다.")))
	{
		CatalogData.Reset();
		ShowDetails(INDEX_NONE);
		return;
	}

	BallTileView->OnItemSelectionChanged().RemoveAll(this);
	BallTileView->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleSelectionChanged);
	CatalogData = Subsystem->GetBallCatalogEntries();

	TArray<FPBCollectionItemSummary> Summaries;
	TArray<int32> DataIndexes;
	for (int32 Index = 0; Index < CatalogData.Num(); ++Index)
	{
		const FPBCollectionBallDisplayData& Item = CatalogData[Index];
		if (MatchesSearch(Item.Summary, {Item.SkillName, Item.SkillDescription, Item.ClassTypeText, Item.RaceTypesText}))
		{
			Summaries.Add(Item.Summary);
			DataIndexes.Add(Index);
		}
	}

	const int32 SelectedIndex = PopulateCatalogItems(BallTileView, EPBCollectionCategory::Ball, Summaries, DataIndexes);
	SetStatus(LOCTEXT("NoBallData", "표시할 볼이 없습니다."), SelectedIndex == INDEX_NONE);
	ShowDetails(SelectedIndex);
}

void UPBCollectionBallTabWidget::HandleSelectionChanged(UObject* Item)
{
	if (const UPBCollectionCatalogItemObject* CatalogItem = ResolveCatalogItem(Item))
	{
		ShowDetails(CatalogItem->DataIndex);
	}
}

void UPBCollectionBallTabWidget::ShowDetails(const int32 DataIndex) const
{
	if (!CatalogData.IsValidIndex(DataIndex))
	{
		SetText(DetailNameText, LOCTEXT("SelectBall", "볼을 선택하세요."));
		SetText(DetailMetaText, FText::GetEmpty());
		SetText(DetailDescriptionText, FText::GetEmpty());
		SetText(SkillNameText, FText::GetEmpty());
		SetText(SkillDescriptionText, FText::GetEmpty());
		SetText(SkillStatsText, FText::GetEmpty());
		SetText(ValidationText, FText::GetEmpty());
		return;
	}

	const FPBCollectionBallDisplayData& Data = CatalogData[DataIndex];
	SetText(DetailNameText, Data.Summary.DisplayName);
	SetText(DetailMetaText, FText::Format(
		LOCTEXT("BallMeta", "{0} · {1} · {2}"),
		Data.ClassTypeText,
		Data.RaceTypesText,
		Data.PowerFlipTypeText));
	SetText(DetailDescriptionText, Data.Summary.Description);
	SetText(SkillNameText, Data.bHasValidSkill ? Data.SkillName : LOCTEXT("InvalidSkill", "스킬 정보 없음"));
	SetText(SkillDescriptionText, Data.SkillDescription);
	SetText(SkillStatsText, Data.SkillStatsText);
	SetText(ValidationText, Data.ValidationText);
}

void UPBCollectionSynergyTabWidget::RefreshTab()
{
	UPBCollectionSubsystem* Subsystem = GetCollectionSubsystem();
	if (!PrepareCatalogRefresh(SynergyListView, LOCTEXT("SynergyLoading", "시너지 데이터를 준비하는 중입니다.")))
	{
		CatalogData.Reset();
		ShowDetails(INDEX_NONE);
		return;
	}

	SynergyListView->OnItemSelectionChanged().RemoveAll(this);
	SynergyListView->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleSelectionChanged);
	CatalogData = Subsystem->GetSynergyCatalogEntries();
	TArray<FPBCollectionItemSummary> Summaries;
	TArray<int32> DataIndexes;
	for (int32 Index = 0; Index < CatalogData.Num(); ++Index)
	{
		const FPBCollectionSynergyDisplayData& Item = CatalogData[Index];
		if (MatchesSearch(Item.Summary, {Item.SynergyKindText, Item.RuleTypeText}))
		{
			Summaries.Add(Item.Summary);
			DataIndexes.Add(Index);
		}
	}

	const int32 SelectedIndex = PopulateCatalogItems(SynergyListView, EPBCollectionCategory::Synergy, Summaries, DataIndexes);
	SetStatus(LOCTEXT("NoSynergyData", "표시할 시너지가 없습니다."), SelectedIndex == INDEX_NONE);
	ShowDetails(SelectedIndex);
}

void UPBCollectionSynergyTabWidget::HandleSelectionChanged(UObject* Item)
{
	if (const UPBCollectionCatalogItemObject* CatalogItem = ResolveCatalogItem(Item))
	{
		ShowDetails(CatalogItem->DataIndex);
	}
}

void UPBCollectionSynergyTabWidget::ShowDetails(const int32 DataIndex) const
{
	if (!CatalogData.IsValidIndex(DataIndex))
	{
		SetText(DetailNameText, LOCTEXT("SelectSynergy", "시너지를 선택하세요."));
		SetText(DetailMetaText, FText::GetEmpty());
		SetText(DetailDescriptionText, FText::GetEmpty());
		SetText(TierListText, FText::GetEmpty());
		SetText(EffectDetailText, FText::GetEmpty());
		return;
	}

	const FPBCollectionSynergyDisplayData& Data = CatalogData[DataIndex];
	SetText(DetailNameText, Data.Summary.DisplayName);
	SetText(DetailMetaText, FText::Format(LOCTEXT("SynergyMeta", "{0} · {1}"), Data.SynergyKindText, Data.RuleTypeText));
	SetText(DetailDescriptionText, Data.Summary.Description);

	TArray<FText> TierLines;
	TArray<FText> EffectLines;
	for (const FPBCollectionSynergyTierDisplayData& Tier : Data.Tiers)
	{
		TierLines.Add(Tier.TierDescription);
		EffectLines.Add(FText::Format(
			LOCTEXT("SynergyTierDetail", "[{0}]\n{1}\n{2}\n{3}"),
			Tier.TierDescription,
			Tier.EffectSummary,
			Tier.ModifierSummary,
			Tier.TriggerSummary));
	}
	SetText(TierListText, JoinTexts(TierLines));
	SetText(
		EffectDetailText,
		EffectLines.IsEmpty() ? LOCTEXT("NoSynergyTierEffects", "등록된 시너지 단계 효과가 없습니다.") : JoinTexts(EffectLines));
}

void UPBCollectionRelicTabWidget::RefreshTab()
{
	UPBCollectionSubsystem* Subsystem = GetCollectionSubsystem();
	if (!PrepareCatalogRefresh(RelicTileView, LOCTEXT("RelicLoading", "유물 데이터를 준비하는 중입니다.")))
	{
		CatalogData.Reset();
		ShowDetails(INDEX_NONE);
		return;
	}

	RelicTileView->OnItemSelectionChanged().RemoveAll(this);
	RelicTileView->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleSelectionChanged);
	CatalogData = Subsystem->GetRelicCatalogEntries();
	TArray<FPBCollectionItemSummary> Summaries;
	TArray<int32> DataIndexes;
	for (int32 Index = 0; Index < CatalogData.Num(); ++Index)
	{
		const FPBCollectionRelicDisplayData& Item = CatalogData[Index];
		if (MatchesSearch(Item.Summary, {Item.RarityText, Item.ModifierSummary}))
		{
			Summaries.Add(Item.Summary);
			DataIndexes.Add(Index);
		}
	}

	const int32 SelectedIndex = PopulateCatalogItems(RelicTileView, EPBCollectionCategory::Relic, Summaries, DataIndexes);
	SetStatus(LOCTEXT("NoRelicData", "표시할 유물이 없습니다."), SelectedIndex == INDEX_NONE);
	ShowDetails(SelectedIndex);
}

void UPBCollectionRelicTabWidget::HandleSelectionChanged(UObject* Item)
{
	if (const UPBCollectionCatalogItemObject* CatalogItem = ResolveCatalogItem(Item))
	{
		ShowDetails(CatalogItem->DataIndex);
	}
}

void UPBCollectionRelicTabWidget::ShowDetails(const int32 DataIndex) const
{
	if (!CatalogData.IsValidIndex(DataIndex))
	{
		SetText(DetailNameText, LOCTEXT("SelectRelic", "유물을 선택하세요."));
		SetText(DetailMetaText, FText::GetEmpty());
		SetText(DetailDescriptionText, FText::GetEmpty());
		SetText(ModifierText, FText::GetEmpty());
		return;
	}

	const FPBCollectionRelicDisplayData& Data = CatalogData[DataIndex];
	SetText(DetailNameText, Data.Summary.DisplayName);
	SetText(DetailMetaText, Data.RarityText);
	SetText(DetailDescriptionText, Data.Summary.Description);
	SetText(
		ModifierText,
		Data.ModifierSummary.IsEmpty() ? LOCTEXT("NoRelicModifiers", "등록된 유물 효과가 없습니다.") : Data.ModifierSummary);
}

void UPBCollectionBumperTabWidget::RefreshTab()
{
	UPBCollectionSubsystem* Subsystem = GetCollectionSubsystem();
	if (!PrepareCatalogRefresh(BumperTileView, LOCTEXT("BumperLoading", "범퍼 데이터를 준비하는 중입니다.")))
	{
		CatalogData.Reset();
		ShowDetails(INDEX_NONE);
		return;
	}

	BumperTileView->OnItemSelectionChanged().RemoveAll(this);
	BumperTileView->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleSelectionChanged);
	CatalogData = Subsystem->GetBumperCatalogEntries();
	TArray<FPBCollectionItemSummary> Summaries;
	TArray<int32> DataIndexes;
	for (int32 Index = 0; Index < CatalogData.Num(); ++Index)
	{
		const FPBCollectionBumperDisplayData& Item = CatalogData[Index];
		if (MatchesSearch(Item.Summary, {Item.BumperTypeText, Item.RoleTypeText, Item.EffectTypeText, Item.PositionText}))
		{
			Summaries.Add(Item.Summary);
			DataIndexes.Add(Index);
		}
	}

	const int32 SelectedIndex = PopulateCatalogItems(BumperTileView, EPBCollectionCategory::Bumper, Summaries, DataIndexes);
	SetStatus(LOCTEXT("NoBumperData", "표시할 범퍼가 없습니다."), SelectedIndex == INDEX_NONE);
	ShowDetails(SelectedIndex);
}

void UPBCollectionBumperTabWidget::HandleSelectionChanged(UObject* Item)
{
	if (const UPBCollectionCatalogItemObject* CatalogItem = ResolveCatalogItem(Item))
	{
		ShowDetails(CatalogItem->DataIndex);
	}
}

void UPBCollectionBumperTabWidget::ShowDetails(const int32 DataIndex) const
{
	if (!CatalogData.IsValidIndex(DataIndex))
	{
		SetText(DetailNameText, LOCTEXT("SelectBumper", "범퍼를 선택하세요."));
		SetText(DetailMetaText, FText::GetEmpty());
		SetText(DetailDescriptionText, FText::GetEmpty());
		SetText(TriggerDetailText, FText::GetEmpty());
		SetText(EffectDetailText, FText::GetEmpty());
		return;
	}

	const FPBCollectionBumperDisplayData& Data = CatalogData[DataIndex];
	SetText(DetailNameText, Data.Summary.DisplayName);
	SetText(DetailMetaText, FText::Format(
		LOCTEXT("BumperMeta", "{0} · {1} · {2}"),
		Data.BumperTypeText,
		Data.RoleTypeText,
		Data.EffectTypeText));
	SetText(DetailDescriptionText, Data.Summary.Description);
	SetText(
		TriggerDetailText,
		Data.bHasValidTrigger
			? FText::Format(
				LOCTEXT("BumperTriggerDetail", "{0} / {1}회\n위치: {2}\n{3}"),
				Data.TriggerTypeText,
				FText::AsNumber(Data.RequiredTriggerCount),
				Data.PositionText,
				Data.TriggerDescription)
			: LOCTEXT("MissingBumperTriggerDetail", "연결된 발동 조건을 찾을 수 없습니다."));
	SetText(
		EffectDetailText,
		Data.bHasValidEffect
			? FText::Format(
				LOCTEXT("BumperEffectDetail", "{0} / Power {1}\n{2}"),
				Data.ExecutionPolicyText,
				FText::AsNumber(Data.EffectPower),
				Data.EffectDescription)
			: LOCTEXT("MissingBumperEffectDetail", "연결된 효과를 찾을 수 없습니다."));
}

void UPBCollectionBossTabWidget::RefreshTab()
{
	UPBCollectionSubsystem* Subsystem = GetCollectionSubsystem();
	if (!PrepareCatalogRefresh(BossSelector, LOCTEXT("BossLoading", "보스 데이터를 준비하는 중입니다.")))
	{
		CatalogData.Reset();
		ShowDetails(INDEX_NONE);
		return;
	}

	BossSelector->OnItemSelectionChanged().RemoveAll(this);
	BossSelector->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleSelectionChanged);
	CatalogData = Subsystem->GetBossCatalogEntries();
	TArray<FPBCollectionItemSummary> Summaries;
	TArray<int32> DataIndexes;
	for (int32 Index = 0; Index < CatalogData.Num(); ++Index)
	{
		const FPBCollectionBossDisplayData& Item = CatalogData[Index];
		if (MatchesSearch(Item.Summary, {Item.MovementTypeText}))
		{
			Summaries.Add(Item.Summary);
			DataIndexes.Add(Index);
		}
	}

	const int32 SelectedIndex = PopulateCatalogItems(BossSelector, EPBCollectionCategory::Boss, Summaries, DataIndexes);
	SetStatus(LOCTEXT("NoBossData", "표시할 보스가 없습니다."), SelectedIndex == INDEX_NONE);
	ShowDetails(SelectedIndex);
}

void UPBCollectionBossTabWidget::HandleSelectionChanged(UObject* Item)
{
	if (const UPBCollectionCatalogItemObject* CatalogItem = ResolveCatalogItem(Item))
	{
		ShowDetails(CatalogItem->DataIndex);
	}
}

void UPBCollectionBossTabWidget::ShowDetails(const int32 DataIndex) const
{
	if (!CatalogData.IsValidIndex(DataIndex))
	{
		SetText(DetailNameText, LOCTEXT("SelectBoss", "보스를 선택하세요."));
		SetText(DetailMetaText, FText::GetEmpty());
		SetText(DetailDescriptionText, FText::GetEmpty());
		SetText(BossStatsText, FText::GetEmpty());
		SetText(HitPointText, FText::GetEmpty());
		SetText(PatternText, FText::GetEmpty());
		return;
	}

	const FPBCollectionBossDisplayData& Data = CatalogData[DataIndex];
	SetText(DetailNameText, Data.Summary.DisplayName);
	SetText(DetailMetaText, Data.MovementTypeText);
	SetText(DetailDescriptionText, Data.Summary.Description);
	SetText(BossStatsText, FText::Format(
		LOCTEXT("BossStats", "HP {0} / 그로기 {1} / 그로기 지속 {2}초 / 격노 {3}%"),
		FText::AsNumber(Data.MaxHP),
		FText::AsNumber(Data.MaxGroggyGauge),
		FText::AsNumber(Data.GroggyDurationSeconds),
		FText::AsNumber(Data.EnrageHPRatioPercent)));

	TArray<FText> HitPointLines;
	for (const FPBCollectionBossHitPointDisplayData& HitPoint : Data.HitPoints)
	{
		HitPointLines.Add(FText::Format(
			LOCTEXT("BossHitPoint", "{0} · {1} · 피해 {2}% · 그로기 {3}%{4}"),
			HitPoint.Name,
			HitPoint.PartTypeText,
			FText::AsNumber(HitPoint.DamageMultiplierPercent),
			FText::AsNumber(HitPoint.GroggyMultiplierPercent),
			HitPoint.bIsWeaknessPoint ? LOCTEXT("Weakness", " · 약점") : FText::GetEmpty()));
	}
	SetText(
		HitPointText,
		HitPointLines.IsEmpty() ? LOCTEXT("NoBossHitPoints", "등록된 부위 정보가 없습니다.") : JoinTexts(HitPointLines));

	TArray<FText> PatternLines;
	for (const FPBCollectionBossPatternDisplayData& Pattern : Data.Patterns)
	{
		PatternLines.Add(FText::Format(
			LOCTEXT("BossPattern", "{0} · {1} · 재사용 {2}초"),
			Pattern.Name,
			Pattern.PhaseTypeText,
			FText::AsNumber(Pattern.CooldownSeconds)));
	}
	SetText(
		PatternText,
		PatternLines.IsEmpty() ? LOCTEXT("NoBossPatterns", "등록된 활성 패턴이 없습니다.") : JoinTexts(PatternLines));
}

#undef LOCTEXT_NAMESPACE
