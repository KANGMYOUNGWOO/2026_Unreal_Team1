#include "PBCollectionEffectSetProjector.h"

#include "PinBallLike/Effect/Handlers/PBEffectHandler.h"
#include "PinBallLike/Effect/TargetResolver/PBEffectTargetResolver.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectSetRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

#define LOCTEXT_NAMESPACE "PBCollectionEffectSetProjector"

namespace
{
FText GetTargetDisplayText(const FName TargetType)
{
	if (TargetType == PBEffectTypes::TargetType::Ball)
	{
		return LOCTEXT("TargetBall", "볼");
	}
	if (TargetType == PBEffectTypes::TargetType::PartyBall
		|| TargetType == PBEffectTypes::TargetType::PartyBalls)
	{
		return LOCTEXT("TargetPartyBalls", "아군 볼");
	}
	if (TargetType == PBEffectTypes::TargetType::Leader)
	{
		return LOCTEXT("TargetLeader", "리더 볼");
	}
	if (TargetType == PBEffectTypes::TargetType::Follower
		|| TargetType == PBEffectTypes::TargetType::Followers)
	{
		return LOCTEXT("TargetFollowers", "팔로워 볼");
	}
	if (TargetType == PBEffectTypes::TargetType::Battle)
	{
		return LOCTEXT("TargetBattle", "전투");
	}
	if (TargetType == PBEffectTypes::TargetType::Shop)
	{
		return LOCTEXT("TargetShop", "상점");
	}

	return TargetType.IsNone()
		? LOCTEXT("TargetUnspecified", "대상 미지정")
		: LOCTEXT("TargetUnsupported", "지원되지 않는 대상");
}

FText GetParameterDisplayText(const FName ParamKey)
{
	if (ParamKey == PBEffectTypes::ParamKey::StatusEffectId) return LOCTEXT("ParamStatusEffect", "상태 효과");
	if (ParamKey == PBEffectTypes::ParamKey::StatName) return LOCTEXT("ParamStatName", "능력치");
	if (ParamKey == PBEffectTypes::ParamKey::ResourceName) return LOCTEXT("ParamResourceName", "자원");
	if (ParamKey == PBEffectTypes::ParamKey::ModifyType) return LOCTEXT("ParamModifyType", "적용 방식");
	if (ParamKey == PBEffectTypes::ParamKey::ApplyTo) return LOCTEXT("ParamApplyTo", "적용 범위");
	if (ParamKey == PBEffectTypes::ParamKey::Value) return LOCTEXT("ParamValue", "수치");
	if (ParamKey == PBEffectTypes::ParamKey::AttackPercent) return LOCTEXT("ParamAttackPercent", "공격력 비율");
	if (ParamKey == PBEffectTypes::ParamKey::ExtraDamage) return LOCTEXT("ParamExtraDamage", "추가 피해");
	if (ParamKey == PBEffectTypes::ParamKey::RequiredCombo) return LOCTEXT("ParamRequiredCombo", "필요 콤보");
	if (ParamKey == PBEffectTypes::ParamKey::ChancePercent) return LOCTEXT("ParamChancePercent", "확률");
	if (ParamKey == PBEffectTypes::ParamKey::IgnoreCount) return LOCTEXT("ParamIgnoreCount", "무시 횟수");
	if (ParamKey == PBEffectTypes::ParamKey::ComboInterval) return LOCTEXT("ParamComboInterval", "콤보 간격");
	if (ParamKey == PBEffectTypes::ParamKey::DamageAmount) return LOCTEXT("ParamDamageAmount", "피해량");
	if (ParamKey == PBEffectTypes::ParamKey::Count) return LOCTEXT("ParamCount", "횟수");
	if (ParamKey == PBEffectTypes::ParamKey::DamageRetentionPercent) return LOCTEXT("ParamDamageRetention", "피해 유지율");
	if (ParamKey == PBEffectTypes::ParamKey::ReviveValue) return LOCTEXT("ParamReviveValue", "부활 수치");
	if (ParamKey == PBEffectTypes::ParamKey::ResourceCost) return LOCTEXT("ParamResourceCost", "자원 비용");
	if (ParamKey == PBEffectTypes::ParamKey::Duration) return LOCTEXT("ParamDuration", "지속 시간");

	return ParamKey.IsNone()
		? LOCTEXT("ParamUnspecified", "미지정")
		: LOCTEXT("ParamUnsupported", "지원되지 않는 항목");
}

bool IsKnownParameterKey(const FName ParamKey)
{
	static const TSet<FName> KnownKeys = {
		PBEffectTypes::ParamKey::StatusEffectId,
		PBEffectTypes::ParamKey::StatName,
		PBEffectTypes::ParamKey::ResourceName,
		PBEffectTypes::ParamKey::ModifyType,
		PBEffectTypes::ParamKey::ApplyTo,
		PBEffectTypes::ParamKey::Value,
		PBEffectTypes::ParamKey::AttackPercent,
		PBEffectTypes::ParamKey::ExtraDamage,
		PBEffectTypes::ParamKey::RequiredCombo,
		PBEffectTypes::ParamKey::ChancePercent,
		PBEffectTypes::ParamKey::IgnoreCount,
		PBEffectTypes::ParamKey::ComboInterval,
		PBEffectTypes::ParamKey::DamageAmount,
		PBEffectTypes::ParamKey::Count,
		PBEffectTypes::ParamKey::DamageRetentionPercent,
		PBEffectTypes::ParamKey::ReviveValue,
		PBEffectTypes::ParamKey::ResourceCost,
		PBEffectTypes::ParamKey::Duration
	};
	return KnownKeys.Contains(ParamKey);
}

FText JoinTextLines(const TArray<FText>& Lines)
{
	TArray<FString> Strings;
	Strings.Reserve(Lines.Num());
	for (const FText& Line : Lines)
	{
		if (!Line.IsEmpty())
		{
			Strings.Add(Line.ToString());
		}
	}

	return Strings.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(Strings, LINE_TERMINATOR));
}
}

FPBCollectionEffectSetProjection FPBCollectionEffectSetProjector::Build(
	const UPBTableDataSubsystem& TableData,
	const FName EffectSetId)
{
	FPBCollectionEffectSetProjection Projection;
	Projection.EffectSetId = EffectSetId;

	if (EffectSetId.IsNone())
	{
		Projection.ValidationText = LOCTEXT("MissingEffectSetId", "EffectSetId가 비어 있습니다.");
		return Projection;
	}

	TArray<FPBEffectSetRow> EffectSetRows;
	if (!TableData.GetEffectSetRows(EffectSetId, EffectSetRows))
	{
		Projection.ValidationText = FText::Format(
			LOCTEXT("MissingEffectSet", "EffectSet {0}을(를) 찾을 수 없거나 등록된 효과가 없습니다."),
			FText::FromName(EffectSetId));
		return Projection;
	}

	Projection.DeclaredEffectCount = EffectSetRows.Num();
	TArray<FText> EffectLines;
	TArray<FText> ParameterLines;
	TArray<FText> TargetLines;
	TArray<FString> ValidationIssues;
	TSet<int32> SeenOrders;
	TArray<FName> SupportedTargetTypes;
	if (const UPBEffectTargetResolver* TargetResolver = GetDefault<UPBEffectTargetResolver>())
	{
		TargetResolver->GetSupportedTargetTypes(SupportedTargetTypes);
	}
	const UPBEffectHandler* EffectHandler = GetDefault<UPBEffectHandler>();

	for (const FPBEffectSetRow& SetRow : EffectSetRows)
	{
		if (SetRow.Order < 0 || SeenOrders.Contains(SetRow.Order))
		{
			ValidationIssues.Add(FString::Printf(
				TEXT("EffectSet 순서가 유효하지 않거나 중복됩니다: %d"),
				SetRow.Order));
		}
		SeenOrders.Add(SetRow.Order);

		FPBEffectTableRow EffectRow;
		if (SetRow.EffectId.IsNone() || !TableData.FindEffectRow(SetRow.EffectId, EffectRow))
		{
			ValidationIssues.Add(FString::Printf(
				TEXT("Effect 참조를 찾을 수 없습니다: %s"),
				SetRow.EffectId.IsNone() ? TEXT("<None>") : *SetRow.EffectId.ToString()));
			continue;
		}

		++Projection.ResolvedEffectCount;
		if (!EffectHandler || !EffectHandler->IsEffectTypeSupported(EffectRow.EffectType))
		{
			ValidationIssues.Add(FString::Printf(
				TEXT("Handler가 지원하지 않는 EffectType입니다: %s (%s)"),
				*EffectRow.EffectType.ToString(),
				*SetRow.EffectId.ToString()));
		}
		if (!SupportedTargetTypes.Contains(EffectRow.TargetType))
		{
			ValidationIssues.Add(FString::Printf(
				TEXT("TargetResolver가 지원하지 않는 TargetType입니다: %s (%s)"),
				*EffectRow.TargetType.ToString(),
				*SetRow.EffectId.ToString()));
		}
		if (EffectRow.Description.IsNone())
		{
			ValidationIssues.Add(FString::Printf(
				TEXT("사용자 표시 설명이 비어 있습니다: %s"),
				*SetRow.EffectId.ToString()));
		}
		const FText EffectText = EffectRow.Description.IsNone()
			? LOCTEXT("MissingEffectDescription", "효과 설명 준비 중")
			: FText::FromName(EffectRow.Description);
		EffectLines.Add(FText::Format(
			LOCTEXT("EffectLine", "{0}. {1}"),
			FText::AsNumber(SetRow.Order),
			EffectText));

		FText TargetText = GetTargetDisplayText(EffectRow.TargetType);
		if (!EffectRow.TargetFilter.IsNone() && EffectRow.TargetFilter != PBEffectTypes::TargetFilter::All)
		{
			TargetText = FText::Format(
				LOCTEXT("FilteredTarget", "{0} ({1})"),
				TargetText,
				FText::FromName(EffectRow.TargetFilter));
		}
		if (EffectRow.TriggerTag.IsValid())
		{
			TargetText = FText::Format(
				LOCTEXT("TriggeredTarget", "{0} / 발동 {1}"),
				TargetText,
				FText::FromName(EffectRow.TriggerTag.GetTagName()));
		}
		TargetLines.Add(TargetText);

		TArray<FPBEffectParamRow> ParamRows;
		TableData.GetEffectParamRows(SetRow.EffectId, ParamRows);
		ParamRows.Sort([](const FPBEffectParamRow& Left, const FPBEffectParamRow& Right)
		{
			return Left.ParamKey.LexicalLess(Right.ParamKey);
		});
		TSet<FName> SeenParamKeys;
		for (const FPBEffectParamRow& ParamRow : ParamRows)
		{
			if (!IsKnownParameterKey(ParamRow.ParamKey))
			{
				ValidationIssues.Add(FString::Printf(
					TEXT("지원되지 않는 ParamKey입니다: %s (%s)"),
					*ParamRow.ParamKey.ToString(),
					*SetRow.EffectId.ToString()));
			}
			if (SeenParamKeys.Contains(ParamRow.ParamKey))
			{
				ValidationIssues.Add(FString::Printf(
					TEXT("중복 ParamKey입니다: %s (%s)"),
					*ParamRow.ParamKey.ToString(),
					*SetRow.EffectId.ToString()));
			}
			SeenParamKeys.Add(ParamRow.ParamKey);
			if (ParamRow.ParamValue.TrimStartAndEnd().IsEmpty())
			{
				ValidationIssues.Add(FString::Printf(
					TEXT("ParamValue가 비어 있습니다: %s (%s)"),
					*ParamRow.ParamKey.ToString(),
					*SetRow.EffectId.ToString()));
			}
			ParameterLines.Add(FText::Format(
				LOCTEXT("ParameterLine", "{0}: {1}"),
				GetParameterDisplayText(ParamRow.ParamKey),
				FText::FromString(ParamRow.ParamValue)));
		}
	}

	Projection.EffectSummary = JoinTextLines(EffectLines);
	Projection.ParameterSummary = JoinTextLines(ParameterLines);
	Projection.TargetSummary = JoinTextLines(TargetLines);
	Projection.bIsValid = Projection.DeclaredEffectCount > 0
		&& Projection.ResolvedEffectCount == Projection.DeclaredEffectCount
		&& ValidationIssues.IsEmpty();

	if (!Projection.bIsValid)
	{
		if (ValidationIssues.IsEmpty())
		{
			ValidationIssues.Add(TEXT("EffectSet에 유효한 Effect가 없습니다."));
		}
		Projection.ValidationText = FText::Format(
			LOCTEXT("InvalidEffectSet", "EffectSet {0} 검증 실패: {1}"),
			FText::FromName(EffectSetId),
			FText::FromString(FString::Join(ValidationIssues, TEXT(" / "))));
	}

	return Projection;
}

#undef LOCTEXT_NAMESPACE
