#include "PBBossIntroViewModel.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"

void UPBBossIntroViewModel::SetBoss(APBBossBase* NewBoss)
{
	if (!NewBoss)
	{
		ClearBoss();
		return;
	}

	FSlateBrush NewBossIntroBrush;
	NewBossIntroBrush.SetResourceObject(NewBoss->GetBossIntroImage());

	UE_MVVM_SET_PROPERTY_VALUE(BossNameText, NewBoss->GetBossName());
	UE_MVVM_SET_PROPERTY_VALUE(BossIntroBrush, NewBossIntroBrush);
}

void UPBBossIntroViewModel::ClearBoss()
{
	UE_MVVM_SET_PROPERTY_VALUE(BossNameText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(BossIntroBrush, FSlateBrush());
}
