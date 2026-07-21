#include "PBDeckOverviewViewModel.h"

#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectParamRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectSetRow.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"
#include "PinBallLike/Table/Synergy/DataAsset/PBSynergyDataAsset.h"
#include "PinBallLike/Table/Synergy/PBSynergyAssetIds.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTableRow.h"
#include "PinBallLike/Table/Synergy/Struct/PBSynergyTierRow.h"

void UPBDeckOverviewViewModel::Initialize(UObject* InWorldContextObject)
{
	WorldContextObject = InWorldContextObject;
}

void UPBDeckOverviewViewModel::SetSynergyStates(const TArray<FPBSynergyState>& InSynergyStates)
{
	TArray<FPBSynergyViewData> NewViewData;

	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem)
	{
		UE_MVVM_SET_PROPERTY_VALUE(ActiveSynergyViewData, NewViewData);
		return;
	}

	TArray<TTuple<FPBSynergyViewData, int32>> ActiveSynergies;
	for (const FPBSynergyState& SynergyState : InSynergyStates)
	{
		if (!SynergyState.IsValid())
		{
			continue;
		}

		FPBSynergyTableRow SynergyRow;
		if (!TableDataSubsystem->FindSynergyRow(SynergyState.SynergyId, SynergyRow))
		{
			continue;
		}

		FPBSynergyViewData ViewData = BuildSynergyViewData(SynergyState, SynergyRow);

		ActiveSynergies.Add(MakeTuple(ViewData, SynergyRow.SortOrder));
	}

	ActiveSynergies.Sort(
		[](const TTuple<FPBSynergyViewData, int32>& Left, const TTuple<FPBSynergyViewData, int32>& Right)
		{
			if (Left.Get<1>() == Right.Get<1>())
			{
				return Left.Get<0>().SynergyId.LexicalLess(Right.Get<0>().SynergyId);
			}

			return Left.Get<1>() < Right.Get<1>();
		});

	NewViewData.Reserve(ActiveSynergies.Num());
	for (const TTuple<FPBSynergyViewData, int32>& ActiveSynergy : ActiveSynergies)
	{
		NewViewData.Add(ActiveSynergy.Get<0>());
	}

	UE_MVVM_SET_PROPERTY_VALUE(ActiveSynergyViewData, NewViewData);
}

void UPBDeckOverviewViewModel::ClearSynergyViewData()
{
	TArray<FPBSynergyViewData> EmptyViewData;
	UE_MVVM_SET_PROPERTY_VALUE(ActiveSynergyViewData, EmptyViewData);
}

const UPBTableDataSubsystem* UPBDeckOverviewViewModel::GetTableDataSubsystem() const
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;
}

FPBSynergyViewData UPBDeckOverviewViewModel::BuildSynergyViewData(
	const FPBSynergyState& SynergyState,
	const FPBSynergyTableRow& SynergyRow) const
{
	FPBSynergyViewData ViewData;
	ViewData.SynergyId = SynergyState.SynergyId;
	ViewData.SynergyName = SynergyRow.DisplayName.IsEmpty()
		? FText::FromName(SynergyState.SynergyId)
		: SynergyRow.DisplayName;
	ViewData.Description = MakeDisplayTextFromKey(SynergyRow.DescriptionKey);
	ViewData.CurrentCount = SynergyState.CurrentCount;
	ViewData.CurrentCountText = FText::AsNumber(SynergyState.CurrentCount);
	ViewData.CountListText = BuildSynergyCountListText(SynergyState.SynergyId, SynergyState.CurrentCount);
	ViewData.Icon = ResolveSynergyIcon(SynergyState.SynergyId);
	ViewData.TierViewDataList = BuildTierViewDataList(SynergyState.SynergyId, SynergyState.CurrentCount);
	ViewData.BallIconViewDataList = BuildBallIconViewDataList(SynergyState.SynergyId);
	return ViewData;
}

FText UPBDeckOverviewViewModel::BuildSynergyCountListText(const FName SynergyId, const int32 CurrentCount) const
{
	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem || SynergyId.IsNone())
	{
		return FText::GetEmpty();
	}

	TArray<FPBSynergyTierRow> TierRows;
	if (!TableDataSubsystem->GetSynergyTierRows(SynergyId, TierRows))
	{
		return FText::GetEmpty();
	}

	TArray<FString> TierTexts;
	for (const FPBSynergyTierRow& TierRow : TierRows)
	{
		const TCHAR* StyleTag = CurrentCount >= TierRow.RequiredCount ? TEXT("Active") : TEXT("InActive");
		TierTexts.Add(FString::Printf(TEXT("<%s>%d</>"), StyleTag, TierRow.RequiredCount));
	}

	return FText::FromString(FString::Join(TierTexts, TEXT(" ")));
}

TArray<FPBSynergyTierViewData> UPBDeckOverviewViewModel::BuildTierViewDataList(
	const FName SynergyId,
	const int32 CurrentCount) const
{
	TArray<FPBSynergyTierViewData> TierViewDataList;

	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem || SynergyId.IsNone())
	{
		return TierViewDataList;
	}

	TArray<FPBSynergyTierRow> TierRows;
	if (!TableDataSubsystem->GetSynergyTierRows(SynergyId, TierRows))
	{
		return TierViewDataList;
	}

	int32 ActiveRequiredCount = 0;
	for (const FPBSynergyTierRow& TierRow : TierRows)
	{
		if (TierRow.RequiredCount <= CurrentCount)
		{
			ActiveRequiredCount = FMath::Max(ActiveRequiredCount, TierRow.RequiredCount);
		}
	}

	TierViewDataList.Reserve(TierRows.Num());
	for (const FPBSynergyTierRow& TierRow : TierRows)
	{
		FPBSynergyTierViewData TierViewData;
		TierViewData.RequiredCount = TierRow.RequiredCount;
		TierViewData.RequiredCountText = FText::AsNumber(TierRow.RequiredCount);
		TierViewData.EffectText = BuildTierEffectText(TierRow);
		TierViewData.bIsActive = CurrentCount >= TierRow.RequiredCount;
		TierViewData.bIsCurrentTier = TierRow.RequiredCount > 0 && TierRow.RequiredCount == ActiveRequiredCount;
		TierViewDataList.Add(TierViewData);
	}

	return TierViewDataList;
}

TArray<FPBSynergyBallIconViewData> UPBDeckOverviewViewModel::BuildBallIconViewDataList(const FName SynergyId) const
{
	TArray<FPBSynergyBallIconViewData> BallIconViewDataList;
	TSet<FName> AddedBallIds;

	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem || SynergyId.IsNone())
	{
		return BallIconViewDataList;
	}

	TArray<FName> BallRowNames;
	TArray<FPBBallTableRow> BallRows;
	if (!TableDataSubsystem->GetAllBallRows(BallRowNames, BallRows))
	{
		return BallIconViewDataList;
	}

	for (int32 RowIndex = 0; RowIndex < BallRows.Num(); ++RowIndex)
	{
		if (!BallRowNames.IsValidIndex(RowIndex) || !DoesBallMatchSynergy(BallRows[RowIndex], SynergyId))
		{
			continue;
		}

		const FName BallId = BallRowNames[RowIndex];
		if (BallId.IsNone() || AddedBallIds.Contains(BallId))
		{
			continue;
		}

		FPBSynergyBallIconViewData BallIconViewData;
		BallIconViewData.BallId = BallId;
		BallIconViewData.Icon = ResolveBallIcon(BallId);
		BallIconViewDataList.Add(BallIconViewData);
		AddedBallIds.Add(BallId);
	}

	return BallIconViewDataList;
}

bool UPBDeckOverviewViewModel::DoesBallMatchSynergy(
	const FPBBallTableRow& BallRow,
	const FName SynergyId) const
{
	const UEnum* RaceEnum = StaticEnum<EPBBallRaceType>();
	for (const EPBBallRaceType RaceType : BallRow.RaceTypes)
	{
		const FName RaceSynergyId = RaceEnum ? FName(*RaceEnum->GetNameStringByValue(static_cast<int64>(RaceType))) : NAME_None;
		if (RaceSynergyId == SynergyId)
		{
			return true;
		}
	}

	const UEnum* ClassEnum = StaticEnum<EPBBallClassType>();
	if (BallRow.ClassType == EPBBallClassType::None)
	{
		return false;
	}

	const FName ClassSynergyId = ClassEnum
		? FName(*ClassEnum->GetNameStringByValue(static_cast<int64>(BallRow.ClassType)))
		: NAME_None;
	return ClassSynergyId == SynergyId;
}

UTexture2D* UPBDeckOverviewViewModel::ResolveSynergyIcon(const FName SynergyId) const
{
	if (SynergyId.IsNone())
	{
		return nullptr;
	}

	FAssetData SynergyAssetData;
	const FPrimaryAssetId SynergyAssetId(PBSynergyAssetIds::Type::SynergyData, SynergyId);
	if (UAssetManager::Get().GetPrimaryAssetData(SynergyAssetId, SynergyAssetData))
	{
		const UPBSynergyDataAsset* SynergyDataAsset = Cast<UPBSynergyDataAsset>(SynergyAssetData.GetAsset());
		if (SynergyDataAsset)
		{
			if (UTexture2D* Icon = SynergyDataAsset->Icon.LoadSynchronous())
			{
				return Icon;
			}
		}
	}

	const FString IconName = FString::Printf(TEXT("Icon_%s"), *SynergyId.ToString());
	const FString IconPath = FString::Printf(TEXT("/Game/Resources/Synergy/%s.%s"), *IconName, *IconName);
	return Cast<UTexture2D>(FSoftObjectPath(IconPath).TryLoad());
}

UTexture2D* UPBDeckOverviewViewModel::ResolveBallIcon(const FName BallId) const
{
	if (BallId.IsNone())
	{
		return nullptr;
	}

	FAssetData BallAssetData;
	const FPrimaryAssetId BallAssetId(PBBallAssetIds::Type::BallData, BallId);
	if (!UAssetManager::Get().GetPrimaryAssetData(BallAssetId, BallAssetData))
	{
		return nullptr;
	}

	const UPBBallDataAsset* BallDataAsset = Cast<UPBBallDataAsset>(BallAssetData.GetAsset());
	return BallDataAsset ? BallDataAsset->BallIcon.LoadSynchronous() : nullptr;
}

FText UPBDeckOverviewViewModel::BuildTierEffectText(const FPBSynergyTierRow& TierRow) const
{
	const FText TierDescription = MakeDisplayTextFromKey(TierRow.TierDescriptionKey);
	if (!TierDescription.IsEmpty())
	{
		return TierDescription;
	}

	return BuildEffectSetText(TierRow.EffectSetId);
}

FText UPBDeckOverviewViewModel::BuildEffectSetText(const FName EffectSetId) const
{
	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem || EffectSetId.IsNone())
	{
		return FText::GetEmpty();
	}

	TArray<FPBEffectSetRow> EffectSetRows;
	if (!TableDataSubsystem->GetEffectSetRows(EffectSetId, EffectSetRows))
	{
		return FText::GetEmpty();
	}

	EffectSetRows.Sort(
		[](const FPBEffectSetRow& Left, const FPBEffectSetRow& Right)
		{
			return Left.Order < Right.Order;
		});

	TArray<FString> EffectTexts;
	for (const FPBEffectSetRow& EffectSetRow : EffectSetRows)
	{
		FPBEffectTableRow EffectRow;
		if (!TableDataSubsystem->FindEffectRow(EffectSetRow.EffectId, EffectRow))
		{
			continue;
		}

		const FText EffectText = BuildEffectText(EffectSetRow.EffectId, EffectRow);
		if (!EffectText.IsEmpty())
		{
			EffectTexts.Add(EffectText.ToString());
		}
	}

	return EffectTexts.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(EffectTexts, TEXT(" / ")));
}

FText UPBDeckOverviewViewModel::BuildEffectText(const FName EffectId, const FPBEffectTableRow& EffectRow) const
{
	const FText DescriptionText = MakeDisplayTextFromKey(EffectRow.Description);
	if (!DescriptionText.IsEmpty())
	{
		return DescriptionText;
	}

	const UPBTableDataSubsystem* TableDataSubsystem = GetTableDataSubsystem();
	if (!TableDataSubsystem || EffectId.IsNone())
	{
		return FText::FromName(EffectRow.EffectType);
	}

	TArray<FPBEffectParamRow> ParamRows;
	TableDataSubsystem->GetEffectParamRows(EffectId, ParamRows);

	const FText TargetText = MakeEffectTargetText(EffectRow.TargetType, EffectRow.TargetFilter);

	if (EffectRow.EffectType == PBEffectTypes::EffectType::ResourceBuff)
	{
		const FName ResourceName = FindEffectParamName(ParamRows, PBEffectTypes::ParamKey::ResourceName);
		const FName ModifyType = FindEffectParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
		const FName ApplyTo = FindEffectParamName(ParamRows, PBEffectTypes::ParamKey::ApplyTo);
		const float Value = FindEffectParamFloat(ParamRows, PBEffectTypes::ParamKey::Value);
		const FText ValueText = MakeModifyValueText(ModifyType, Value);
		const FText ResourceText = FText::FromName(ResourceName);

		if (ApplyTo == PBEffectTypes::ResourceApplyTo::Max)
		{
			return FText::Format(NSLOCTEXT("PBDeckOverview", "ResourceBuffMax", "{0} {1} 최대 {2}"),
				TargetText,
				ResourceText,
				ValueText);
		}

		return FText::Format(NSLOCTEXT("PBDeckOverview", "ResourceBuff", "{0} {1} {2}"),
			TargetText,
			ResourceText,
			ValueText);
	}

	if (EffectRow.EffectType == PBEffectTypes::EffectType::StatBuff)
	{
		const FName StatName = FindEffectParamName(ParamRows, PBEffectTypes::ParamKey::StatName);
		const FName ModifyType = FindEffectParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
		const float Value = FindEffectParamFloat(ParamRows, PBEffectTypes::ParamKey::Value);
		return FText::Format(NSLOCTEXT("PBDeckOverview", "StatBuff", "{0} {1} {2}"),
			TargetText,
			FText::FromName(StatName),
			MakeModifyValueText(ModifyType, Value));
	}

	if (EffectRow.EffectType == PBEffectTypes::EffectType::ShopPriceDiscount)
	{
		const FName ModifyType = FindEffectParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
		const float Value = FindEffectParamFloat(ParamRows, PBEffectTypes::ParamKey::Value);
		return FText::Format(NSLOCTEXT("PBDeckOverview", "ShopPriceDiscount", "상점 구매 비용 {0}"),
			MakeModifyValueText(ModifyType, -FMath::Abs(Value)));
	}

	if (EffectRow.EffectType == PBEffectTypes::EffectType::ShopRerollDiscount)
	{
		const FName ModifyType = FindEffectParamName(ParamRows, PBEffectTypes::ParamKey::ModifyType);
		const float Value = FindEffectParamFloat(ParamRows, PBEffectTypes::ParamKey::Value);
		return FText::Format(NSLOCTEXT("PBDeckOverview", "ShopRerollDiscount", "리롤 비용 {0}"),
			MakeModifyValueText(ModifyType, -FMath::Abs(Value)));
	}

	if (EffectRow.EffectType == PBEffectTypes::EffectType::SwitchCountBonus)
	{
		const float Count = FindEffectParamFloat(ParamRows, PBEffectTypes::ParamKey::Count);
		return FText::Format(NSLOCTEXT("PBDeckOverview", "SwitchCountBonus", "교체 횟수 +{0}"),
			FText::AsNumber(FMath::RoundToInt(Count)));
	}

	return FText::FromName(EffectRow.EffectType);
}

FString UPBDeckOverviewViewModel::FindEffectParamValue(
	const TArray<FPBEffectParamRow>& ParamRows,
	const FName ParamKey) const
{
	for (const FPBEffectParamRow& ParamRow : ParamRows)
	{
		if (ParamRow.ParamKey == ParamKey)
		{
			return ParamRow.ParamValue;
		}
	}

	return FString();
}

FName UPBDeckOverviewViewModel::FindEffectParamName(
	const TArray<FPBEffectParamRow>& ParamRows,
	const FName ParamKey) const
{
	const FString ParamValue = FindEffectParamValue(ParamRows, ParamKey);
	return ParamValue.IsEmpty() ? NAME_None : FName(*ParamValue);
}

float UPBDeckOverviewViewModel::FindEffectParamFloat(
	const TArray<FPBEffectParamRow>& ParamRows,
	const FName ParamKey,
	const float DefaultValue) const
{
	const FString ParamValue = FindEffectParamValue(ParamRows, ParamKey);
	if (ParamValue.IsEmpty())
	{
		return DefaultValue;
	}

	return FCString::Atof(*ParamValue);
}

FText UPBDeckOverviewViewModel::MakeDisplayTextFromKey(const FName TextKey) const
{
	return TextKey.IsNone() ? FText::GetEmpty() : FText::FromName(TextKey);
}

FText UPBDeckOverviewViewModel::MakeModifyValueText(const FName ModifyType, const float Value) const
{
	if (ModifyType == PBEffectTypes::ModifyType::PercentAdd)
	{
		return FText::FromString(FString::Printf(TEXT("%+.0f%%"), Value));
	}

	return FText::FromString(FString::Printf(TEXT("%+.0f"), Value));
}

FText UPBDeckOverviewViewModel::MakeEffectTargetText(const FName TargetType, const FName TargetFilter) const
{
	if (TargetType == PBEffectTypes::TargetType::Shop)
	{
		return FText::FromString(TEXT("상점"));
	}

	if (TargetFilter == FName(TEXT("Self")))
	{
		return FText::FromString(TEXT("해당 시너지"));
	}

	if (TargetType == PBEffectTypes::TargetType::PartyBalls
		|| TargetType == PBEffectTypes::TargetType::PartyBall)
	{
		return FText::FromString(TEXT("아군"));
	}

	if (TargetType == PBEffectTypes::TargetType::Leader)
	{
		return FText::FromString(TEXT("리더"));
	}

	if (TargetType == PBEffectTypes::TargetType::Follower
		|| TargetType == PBEffectTypes::TargetType::Followers)
	{
		return FText::FromString(TEXT("팔로워"));
	}

	return FText::GetEmpty();
}
