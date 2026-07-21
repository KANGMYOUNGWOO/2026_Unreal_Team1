#include "PBBallDetailTooltipViewModel.h"

void UPBBallDetailTooltipViewModel::SetTooltipViewData(const FPBBallDetailTooltipViewData& InViewData)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasBall, InViewData.IsValid());
	UE_MVVM_SET_PROPERTY_VALUE(BallId, InViewData.BallId);
	UE_MVVM_SET_PROPERTY_VALUE(BallIconTexture, InViewData.BallIconTexture);
	UE_MVVM_SET_PROPERTY_VALUE(BallNameText, InViewData.BallNameText);
	UE_MVVM_SET_PROPERTY_VALUE(BallDescriptionText, InViewData.BallDescriptionText);
	UE_MVVM_SET_PROPERTY_VALUE(HpRow, InViewData.HpRow);
	UE_MVVM_SET_PROPERTY_VALUE(MpRow, InViewData.MpRow);
	UE_MVVM_SET_PROPERTY_VALUE(AttackRow, InViewData.AttackRow);
	UE_MVVM_SET_PROPERTY_VALUE(ManaRegenRow, InViewData.ManaRegenRow);
	UE_MVVM_SET_PROPERTY_VALUE(PowerFlipData, InViewData.PowerFlipData);
	UE_MVVM_SET_PROPERTY_VALUE(ClassData, InViewData.ClassData);
	UE_MVVM_SET_PROPERTY_VALUE(RaceDataList, InViewData.RaceDataList);
	UE_MVVM_SET_PROPERTY_VALUE(SkillIconTexture, InViewData.SkillIconTexture);
	UE_MVVM_SET_PROPERTY_VALUE(SkillNameText, InViewData.SkillNameText);
	UE_MVVM_SET_PROPERTY_VALUE(SkillDescriptionText, InViewData.SkillDescriptionText);
}

void UPBBallDetailTooltipViewModel::ClearTooltip()
{
	SetTooltipViewData(FPBBallDetailTooltipViewData());
}
