#include "PBBossStatusViewModel.h"

#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"

void UPBBossStatusViewModel::SetBoss(APBBossBase* NewBoss)
{
	if (Boss == NewBoss)
	{
		RefreshBossStatus();
		return;
	}

	UnbindBossEvents();
	Boss = NewBoss;
	BindBossEvents();
	RefreshBossStatus();
}

void UPBBossStatusViewModel::ClearBoss()
{
	UnbindBossEvents();
	Boss = nullptr;

	RefreshBossName();
	HandleHPChanged(0, 1);
	HandleGroggyGaugeChanged(0, 1);
}

void UPBBossStatusViewModel::BeginDestroy()
{
	ClearBoss();
	Super::BeginDestroy();
}

void UPBBossStatusViewModel::HandleHPChanged(int32 HP, int32 MaxHP)
{
	UE_MVVM_SET_PROPERTY_VALUE(HPPercent, CalculateGaugePercent(HP, MaxHP));
	UE_MVVM_SET_PROPERTY_VALUE(
		HPText,
		FText::Format(NSLOCTEXT("BossStatusViewModel", "HPTextFormat", "{0} / {1}"), HP, MaxHP));
}

void UPBBossStatusViewModel::HandleGroggyGaugeChanged(int32 GroggyGauge, int32 MaxGroggyGauge)
{
	UE_MVVM_SET_PROPERTY_VALUE(GroggyPercent, CalculateGaugePercent(GroggyGauge, MaxGroggyGauge));
}

void UPBBossStatusViewModel::BindBossEvents()
{
	if (!Boss)
	{
		return;
	}

	if (UPBBossStatComponent* BossStatComponent = Boss->GetBossStatComponent())
	{
		BossStatComponent->OnHPChanged.AddUniqueDynamic(this, &UPBBossStatusViewModel::HandleHPChanged);
	}

	if (UPBBossGroggyComponent* BossGroggyComponent = Boss->GetBossGroggyComponent())
	{
		BossGroggyComponent->OnGroggyGaugeChanged.AddUniqueDynamic(
			this,
			&UPBBossStatusViewModel::HandleGroggyGaugeChanged);
	}
}

void UPBBossStatusViewModel::UnbindBossEvents()
{
	if (!Boss)
	{
		return;
	}

	if (UPBBossStatComponent* BossStatComponent = Boss->GetBossStatComponent())
	{
		BossStatComponent->OnHPChanged.RemoveDynamic(this, &UPBBossStatusViewModel::HandleHPChanged);
	}

	if (UPBBossGroggyComponent* BossGroggyComponent = Boss->GetBossGroggyComponent())
	{
		BossGroggyComponent->OnGroggyGaugeChanged.RemoveDynamic(
			this,
			&UPBBossStatusViewModel::HandleGroggyGaugeChanged);
	}
}

void UPBBossStatusViewModel::RefreshBossName()
{
	UE_MVVM_SET_PROPERTY_VALUE(BossNameText, Boss ? Boss->GetBossName() : FText::GetEmpty());
}

void UPBBossStatusViewModel::RefreshBossStatus()
{
	RefreshBossName();

	if (!Boss)
	{
		HandleHPChanged(0, 1);
		HandleGroggyGaugeChanged(0, 1);
		return;
	}

	if (const UPBBossStatComponent* BossStatComponent = Boss->GetBossStatComponent())
	{
		HandleHPChanged(BossStatComponent->HP, BossStatComponent->MaxHP);
	}

	if (const UPBBossGroggyComponent* BossGroggyComponent = Boss->GetBossGroggyComponent())
	{
		HandleGroggyGaugeChanged(BossGroggyComponent->GroggyGauge, BossGroggyComponent->MaxGroggyGauge);
	}
}

float UPBBossStatusViewModel::CalculateGaugePercent(int32 CurrentValue, int32 MaxValue)
{
	if (MaxValue <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentValue) / static_cast<float>(MaxValue), 0.0f, 1.0f);
}
