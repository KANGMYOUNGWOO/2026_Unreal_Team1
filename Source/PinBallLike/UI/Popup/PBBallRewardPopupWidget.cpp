#include "PBBallRewardPopupWidget.h"

#include "AssetRegistry/AssetData.h"
#include "Components/SizeBox.h"
#include "Engine/AssetManager.h"
#include "PinBallLike/Actor/Ball/UI/PBBallDetailTooltipWidget.h"
#include "PinBallLike/Struct/Ball/PBBallClassType.h"
#include "PinBallLike/Struct/Ball/PBBallRaceType.h"
#include "PinBallLike/Struct/Ball/PBPowerFlipType.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Struct/Common/PBStatTypes.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"

namespace
{
	FPBBallDetailInfoRowViewData MakeInfoRow(
		const FText& LabelText,
		const int32 Value)
	{
		FPBBallDetailInfoRowViewData RowViewData;
		RowViewData.LabelText = LabelText;
		RowViewData.ValueText = FText::AsNumber(Value);
		return RowViewData;
	}

	FPBBallDetailIconTextViewData MakeIconText(
		UTexture2D* IconTexture,
		const FText& Text)
	{
		FPBBallDetailIconTextViewData ViewData;
		ViewData.IconTexture = IconTexture;
		ViewData.Text = Text;
		return ViewData;
	}

	int32 FindMapValue(
		const TMap<FName, int32>& Values,
		const FName Key)
	{
		const int32* Value = Values.Find(Key);
		return Value ? *Value : 0;
	}

	FText GetEnumDisplayText(const UEnum* Enum, const int64 Value)
	{
		return Enum ? Enum->GetDisplayNameTextByValue(Value) : FText::GetEmpty();
	}

	FName GetEnumValueName(const UEnum* Enum, const int64 Value)
	{
		return Enum ? FName(*Enum->GetNameStringByValue(Value)) : NAME_None;
	}
}

bool UPBBallRewardPopupWidget::InitializeBallRewardPopup(
	const FText& InMessage,
	const FName BallId,
	const int32 StarLevel)
{
	if (!BallDetailContainer || !BallDetailTooltipClass)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BallRewardPopup initialization failed. Container=%s TooltipClass=%s"),
			*GetNameSafe(BallDetailContainer),
			*GetNameSafe(BallDetailTooltipClass));
		return false;
	}

	FPBBallDetailTooltipViewData ViewData;
	if (!BuildBallDetailViewData(BallId, StarLevel, ViewData))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BallRewardPopup failed to build Ball detail data. BallId=%s StarLevel=%d"),
			*BallId.ToString(),
			StarLevel);
		return false;
	}

	InitializePopup(InMessage);

	BallDetailContainer->ClearChildren();
	BallDetailTooltip = CreateWidget<UPBBallDetailTooltipWidget>(
		this,
		BallDetailTooltipClass);
	if (!BallDetailTooltip)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("BallRewardPopup failed to create the Ball detail Tooltip. Class=%s"),
			*GetNameSafe(BallDetailTooltipClass));
		return false;
	}

	BallDetailContainer->SetContent(BallDetailTooltip);
	BallDetailTooltip->SetTooltipViewData(ViewData);
	return true;
}

bool UPBBallRewardPopupWidget::BuildBallDetailViewData(
	const FName BallId,
	const int32 StarLevel,
	FPBBallDetailTooltipViewData& OutViewData) const
{
	OutViewData = FPBBallDetailTooltipViewData();
	if (BallId.IsNone())
	{
		return false;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UPBTableDataSubsystem* TableDataSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBTableDataSubsystem>()
		: nullptr;
	if (!TableDataSubsystem)
	{
		return false;
	}

	FPBBallTableRow BallRow;
	if (!TableDataSubsystem->FindBallRow(BallId, BallRow))
	{
		return false;
	}

	FName StarLevelRowName = NAME_None;
	FPBBallStarLevelRow StarLevelRow;
	if (!TableDataSubsystem->FindBallStarLevelRow(
		BallId,
		FMath::Max(StarLevel, 1),
		StarLevelRowName,
		StarLevelRow))
	{
		return false;
	}

	FPBBallSkillTableRow SkillRow;
	(void)TableDataSubsystem->FindDefaultSkillRowForBall(BallId, SkillRow);

	const UEnum* PowerFlipEnum = StaticEnum<EPBPowerFlipType>();
	const UEnum* RaceEnum = StaticEnum<EPBBallRaceType>();
	const UEnum* ClassEnum = StaticEnum<EPBBallClassType>();

	const FPrimaryAssetId BallAssetId(PBBallAssetIds::Type::BallData, BallId);
	FAssetData BallAssetData;
	const UPBBallDataAsset* BallDataAsset = nullptr;
	if (UAssetManager::Get().GetPrimaryAssetData(BallAssetId, BallAssetData))
	{
		BallDataAsset = Cast<UPBBallDataAsset>(BallAssetData.GetAsset());
	}

	OutViewData.bHasBall = true;
	OutViewData.BallId = BallId;
	OutViewData.BallIconTexture = BallDataAsset
		? BallDataAsset->BallIcon.LoadSynchronous()
		: nullptr;
	OutViewData.BallNameText = BallRow.DisplayName.IsEmpty()
		? FText::FromName(BallId)
		: BallRow.DisplayName;
	OutViewData.BallDescriptionText = BallRow.DescriptionKey;

	OutViewData.HpRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "HPLabel", "HP"),
		FindMapValue(StarLevelRow.BaseResources, PBResourceNames::Health));
	OutViewData.MpRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "MPLabel", "MP"),
		FindMapValue(StarLevelRow.BaseResources, PBResourceNames::Mana));
	OutViewData.AttackRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "AttackLabel", "공격력"),
		FindMapValue(StarLevelRow.BaseStats, PBStatNames::Attack));
	OutViewData.ManaRegenRow = MakeInfoRow(
		NSLOCTEXT("BallDetailTooltip", "ManaRegenLabel", "MP회복"),
		FindMapValue(StarLevelRow.BaseStats, PBStatNames::ManaRegen));

	OutViewData.PowerFlipData = MakeIconText(
		BallDataAsset ? BallDataAsset->PowerFlipIcon.LoadSynchronous() : nullptr,
		GetEnumDisplayText(PowerFlipEnum, static_cast<int64>(BallRow.PowerFlipType)));

	if (BallRow.ClassType != EPBBallClassType::None)
	{
		OutViewData.ClassData = MakeIconText(
			BallDataAsset ? BallDataAsset->ClassIcon.LoadSynchronous() : nullptr,
			GetEnumDisplayText(ClassEnum, static_cast<int64>(BallRow.ClassType)));
	}

	for (int32 RaceIndex = 0; RaceIndex < BallRow.RaceTypes.Num(); ++RaceIndex)
	{
		const EPBBallRaceType RaceType = BallRow.RaceTypes[RaceIndex];
		const FName RaceSynergyId = GetEnumValueName(
			RaceEnum,
			static_cast<int64>(RaceType));
		if (RaceSynergyId.IsNone())
		{
			continue;
		}

		OutViewData.RaceDataList.Add(MakeIconText(
			(BallDataAsset && BallDataAsset->RaceIcons.IsValidIndex(RaceIndex))
				? BallDataAsset->RaceIcons[RaceIndex].LoadSynchronous()
				: nullptr,
			GetEnumDisplayText(RaceEnum, static_cast<int64>(RaceType))));
	}

	OutViewData.SkillNameText = SkillRow.DisplayName;
	OutViewData.SkillDescriptionText = SkillRow.GetDescription(
		FindMapValue(StarLevelRow.BaseStats, PBStatNames::Attack));
	if (BallDataAsset)
	{
		OutViewData.SkillIconTexture = BallDataAsset->SkillIcon.LoadSynchronous();
	}

	return true;
}
