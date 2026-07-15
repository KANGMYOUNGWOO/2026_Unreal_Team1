#include "PBGolemHandStatusViewModel.h"

#include "PinBallLike/Actor/Boss/Golem/PBGolemBoss.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossHand.h"

void UPBGolemHandStatusViewModel::SetGolemBoss(APBGolemBoss* NewGolemBoss)
{
	if (GolemBoss == NewGolemBoss)
	{
		BindGolemHandEvents();
		RefreshGolemHandStatus();
		return;
	}

	ClearGolemBoss();
	GolemBoss = NewGolemBoss;
	if (GolemBoss)
	{
		GolemBoss->OnGolemHandsChanged.AddUniqueDynamic(this, &UPBGolemHandStatusViewModel::HandleGolemHandsChanged);
	}

	BindGolemHandEvents();
	RefreshGolemHandStatus();
}

void UPBGolemHandStatusViewModel::ClearGolemBoss()
{
	UnbindGolemHandEvents();
	if (GolemBoss)
	{
		GolemBoss->OnGolemHandsChanged.RemoveDynamic(this, &UPBGolemHandStatusViewModel::HandleGolemHandsChanged);
	}

	GolemBoss = nullptr;
	HandleLeftHandHPChanged(0, 1);
	HandleRightHandHPChanged(0, 1);
}

void UPBGolemHandStatusViewModel::BeginDestroy()
{
	ClearGolemBoss();
	Super::BeginDestroy();
}

void UPBGolemHandStatusViewModel::HandleGolemHandsChanged()
{
	BindGolemHandEvents();
	RefreshGolemHandStatus();
}

void UPBGolemHandStatusViewModel::HandleLeftHandHPChanged(int32 CurrentHandHP, int32 MaxHandHP)
{
	UE_MVVM_SET_PROPERTY_VALUE(LeftHandHPPercent, CalculateHPPercent(CurrentHandHP, MaxHandHP));
}

void UPBGolemHandStatusViewModel::HandleRightHandHPChanged(int32 CurrentHandHP, int32 MaxHandHP)
{
	UE_MVVM_SET_PROPERTY_VALUE(RightHandHPPercent, CalculateHPPercent(CurrentHandHP, MaxHandHP));
}

void UPBGolemHandStatusViewModel::BindGolemHandEvents()
{
	UnbindGolemHandEvents();
	if (!GolemBoss)
	{
		return;
	}

	LeftGolemHand = GolemBoss->GetGolemHand(EPBGolemBossHandType::Left);
	RightGolemHand = GolemBoss->GetGolemHand(EPBGolemBossHandType::Right);

	if (LeftGolemHand)
	{
		LeftGolemHand->OnHandHPChanged.AddUniqueDynamic(this, &UPBGolemHandStatusViewModel::HandleLeftHandHPChanged);
	}

	if (RightGolemHand)
	{
		RightGolemHand->OnHandHPChanged.AddUniqueDynamic(this, &UPBGolemHandStatusViewModel::HandleRightHandHPChanged);
	}
}

void UPBGolemHandStatusViewModel::UnbindGolemHandEvents()
{
	if (LeftGolemHand)
	{
		LeftGolemHand->OnHandHPChanged.RemoveDynamic(this, &UPBGolemHandStatusViewModel::HandleLeftHandHPChanged);
	}

	if (RightGolemHand)
	{
		RightGolemHand->OnHandHPChanged.RemoveDynamic(this, &UPBGolemHandStatusViewModel::HandleRightHandHPChanged);
	}

	LeftGolemHand = nullptr;
	RightGolemHand = nullptr;
}

void UPBGolemHandStatusViewModel::RefreshGolemHandStatus()
{
	HandleLeftHandHPChanged(
		LeftGolemHand ? LeftGolemHand->GetCurrentHandHP() : 0,
		LeftGolemHand ? LeftGolemHand->GetMaxHandHP() : 1);
	HandleRightHandHPChanged(
		RightGolemHand ? RightGolemHand->GetCurrentHandHP() : 0,
		RightGolemHand ? RightGolemHand->GetMaxHandHP() : 1);
}

float UPBGolemHandStatusViewModel::CalculateHPPercent(int32 CurrentHP, int32 MaxHP)
{
	if (MaxHP <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentHP) / static_cast<float>(MaxHP), 0.0f, 1.0f);
}
