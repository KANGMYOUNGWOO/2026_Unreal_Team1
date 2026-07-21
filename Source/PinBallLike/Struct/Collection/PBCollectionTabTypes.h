#pragma once

#include "CoreMinimal.h"
#include "PBCollectionTabTypes.generated.h"

USTRUCT(BlueprintType)
struct FPBCollectionItemSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FName SourceRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FText Subtitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FName IconAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FName PreviewAssetKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FName AssetBundleName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	FLinearColor AccentColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Catalog")
	int32 SortOrder = 0;
};

USTRUCT(BlueprintType)
struct FPBCollectionBallDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FPBCollectionItemSummary Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FText PowerFlipTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FText RaceTypesText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FText ClassTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FName SkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FText SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FText SkillDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FText SkillStatsText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	bool bHasValidSkill = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Ball")
	FText ValidationText;
};

USTRUCT(BlueprintType)
struct FPBCollectionSynergyTierDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	int32 RequiredCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FName EffectSetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy", meta = (DeprecatedProperty, DeprecationMessage = "EffectSetId를 사용하세요."))
	FName EffectId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FText TierDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FText EffectSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FText ModifierSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FText TriggerSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	bool bHasValidEffect = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	int32 DeclaredEffectCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	int32 ResolvedEffectCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FText ValidationText;
};

USTRUCT(BlueprintType)
struct FPBCollectionSynergyDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FPBCollectionItemSummary Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FText SynergyKindText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	FText RuleTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Synergy")
	TArray<FPBCollectionSynergyTierDisplayData> Tiers;
};

USTRUCT(BlueprintType)
struct FPBCollectionRelicDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Relic")
	FPBCollectionItemSummary Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Relic")
	FText RarityText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Relic")
	FText ModifierSummary;
};

USTRUCT(BlueprintType)
struct FPBCollectionBumperDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FPBCollectionItemSummary Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText BumperTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText RoleTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText EffectTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText TriggerTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText PositionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText TriggerDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	bool bHasValidTrigger = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	int32 RequiredTriggerCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText ExecutionPolicyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	FText EffectDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	bool bHasValidEffect = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Bumper")
	float EffectPower = 0.0f;
};

USTRUCT(BlueprintType)
struct FPBCollectionBossHitPointDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	FText PartTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	int32 DamageMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	int32 GroggyMultiplierPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	bool bIsWeaknessPoint = false;
};

USTRUCT(BlueprintType)
struct FPBCollectionBossPatternDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	FText PhaseTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	float CooldownSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FPBCollectionBossDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	FPBCollectionItemSummary Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	int32 MaxHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	int32 MaxGroggyGauge = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	float GroggyDurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	int32 EnrageHPRatioPercent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	TArray<FPBCollectionBossHitPointDisplayData> HitPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Boss")
	TArray<FPBCollectionBossPatternDisplayData> Patterns;
};

UENUM(BlueprintType)
enum class EPBCollectionValidationSeverity : uint8
{
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FPBCollectionValidationIssue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Validation")
	EPBCollectionValidationSeverity Severity = EPBCollectionValidationSeverity::Warning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Validation")
	FName Topic = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Validation")
	FName SourceRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection|Validation")
	FText Message;
};
