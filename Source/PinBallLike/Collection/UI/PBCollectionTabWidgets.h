#pragma once

#include "CoreMinimal.h"
#include "PBCollectionTabWidgetBase.h"
#include "PBCollectionTabWidgets.generated.h"

class UListView;
class UTextBlock;
class UTileView;

/** Ball과 기본 Skill을 결합해 타일 및 상세 패널로 표시하는 도감 탭입니다. */
UCLASS()
class PINBALLLIKE_API UPBCollectionBallTabWidget : public UPBCollectionTabWidgetBase
{
	GENERATED_BODY()

public:
	virtual void RefreshTab() override;

private:
	UFUNCTION()
	void HandleSelectionChanged(UObject* Item);
	void ShowDetails(int32 DataIndex) const;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Ball", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTileView> BallTileView;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailNameText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailMetaText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailDescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> SkillNameText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> SkillDescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> SkillStatsText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ValidationText;

	UPROPERTY(Transient)
	TArray<FPBCollectionBallDisplayData> CatalogData;
};

/** Synergy의 단계, 효과, Modifier, Trigger를 조합해 표시하는 도감 탭입니다. */
UCLASS()
class PINBALLLIKE_API UPBCollectionSynergyTabWidget : public UPBCollectionTabWidgetBase
{
	GENERATED_BODY()

public:
	virtual void RefreshTab() override;

private:
	UFUNCTION()
	void HandleSelectionChanged(UObject* Item);
	void ShowDetails(int32 DataIndex) const;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Synergy", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UListView> SynergyListView;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailNameText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailMetaText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailDescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TierListText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> EffectDetailText;

	UPROPERTY(Transient)
	TArray<FPBCollectionSynergyDisplayData> CatalogData;
};

/** Relic과 Modifier를 조합해 타일 및 상세 패널로 표시하는 도감 탭입니다. */
UCLASS()
class PINBALLLIKE_API UPBCollectionRelicTabWidget : public UPBCollectionTabWidgetBase
{
	GENERATED_BODY()

public:
	virtual void RefreshTab() override;

private:
	UFUNCTION()
	void HandleSelectionChanged(UObject* Item);
	void ShowDetails(int32 DataIndex) const;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Relic", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTileView> RelicTileView;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailNameText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailMetaText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailDescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ModifierText;

	UPROPERTY(Transient)
	TArray<FPBCollectionRelicDisplayData> CatalogData;
};

/** 범퍼, Trigger, Effect 시트의 연결 결과를 표시하는 도감 탭입니다. */
UCLASS()
class PINBALLLIKE_API UPBCollectionBumperTabWidget : public UPBCollectionTabWidgetBase
{
	GENERATED_BODY()

public:
	virtual void RefreshTab() override;

private:
	UFUNCTION()
	void HandleSelectionChanged(UObject* Item);
	void ShowDetails(int32 DataIndex) const;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Bumper", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTileView> BumperTileView;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailNameText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailMetaText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailDescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TriggerDetailText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> EffectDetailText;

	UPROPERTY(Transient)
	TArray<FPBCollectionBumperDisplayData> CatalogData;
};

/** Boss와 약점, 패턴 시트를 조합해 표시하는 도감 탭입니다. */
UCLASS()
class PINBALLLIKE_API UPBCollectionBossTabWidget : public UPBCollectionTabWidgetBase
{
	GENERATED_BODY()

public:
	virtual void RefreshTab() override;

private:
	UFUNCTION()
	void HandleSelectionChanged(UObject* Item);
	void ShowDetails(int32 DataIndex) const;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Boss", meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UListView> BossSelector;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailNameText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailMetaText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DetailDescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> BossStatsText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> HitPointText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> PatternText;

	UPROPERTY(Transient)
	TArray<FPBCollectionBossDisplayData> CatalogData;
};
