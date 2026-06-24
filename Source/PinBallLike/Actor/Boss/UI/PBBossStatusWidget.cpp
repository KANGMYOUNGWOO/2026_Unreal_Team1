#include "PBBossStatusWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/Component/PBBossGroggyComponent.h"
#include "PinBallLike/Actor/Boss/Component/PBBossStatComponent.h"

void UPBBossStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RefreshBossStatus();
}

void UPBBossStatusWidget::SetBoss(APBBossBase* NewBoss)
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

void UPBBossStatusWidget::ClearBoss()
{
	UnbindBossEvents();
	Boss = nullptr;

	RefreshBossName();
	HandleHPChanged(0, 1);
	HandleGroggyGaugeChanged(0, 1);
}

void UPBBossStatusWidget::NativeDestruct()
{
	ClearBoss();
	Super::NativeDestruct();
}

void UPBBossStatusWidget::HandleHPChanged(int32 HP, int32 MaxHP)
{
	if (HPProgressBar)
	{
		HPProgressBar->SetPercent(CalculateGaugePercent(HP, MaxHP));
	}

	if (HPText)
	{
		HPText->SetText(FText::Format(NSLOCTEXT("BossStatusWidget", "HPTextFormat", "{0} / {1}"), HP, MaxHP));
	}
}

void UPBBossStatusWidget::HandleGroggyGaugeChanged(int32 GroggyGauge, int32 MaxGroggyGauge)
{
	if (GroggyProgressBar)
	{
		GroggyProgressBar->SetPercent(CalculateGaugePercent(GroggyGauge, MaxGroggyGauge));
	}
}

void UPBBossStatusWidget::BindBossEvents()
{
	if (!Boss)
	{
		return;
	}

	if (UPBBossStatComponent* BossStatComponent = Boss->GetBossStatComponent())
	{
		BossStatComponent->OnHPChanged.AddUniqueDynamic(this, &UPBBossStatusWidget::HandleHPChanged);
	}

	if (UPBBossGroggyComponent* BossGroggyComponent = Boss->GetBossGroggyComponent())
	{
		BossGroggyComponent->OnGroggyGaugeChanged.AddUniqueDynamic(this, &UPBBossStatusWidget::HandleGroggyGaugeChanged);
	}
}

void UPBBossStatusWidget::UnbindBossEvents()
{
	if (!Boss)
	{
		return;
	}

	if (UPBBossStatComponent* BossStatComponent = Boss->GetBossStatComponent())
	{
		BossStatComponent->OnHPChanged.RemoveDynamic(this, &UPBBossStatusWidget::HandleHPChanged);
	}

	if (UPBBossGroggyComponent* BossGroggyComponent = Boss->GetBossGroggyComponent())
	{
		BossGroggyComponent->OnGroggyGaugeChanged.RemoveDynamic(this, &UPBBossStatusWidget::HandleGroggyGaugeChanged);
	}
}

void UPBBossStatusWidget::RefreshBossName()
{
	if (!BossNameText)
	{
		return;
	}

	BossNameText->SetText(Boss ? Boss->GetBossName() : FText::GetEmpty());
}

void UPBBossStatusWidget::RefreshBossStatus()
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

float UPBBossStatusWidget::CalculateGaugePercent(int32 CurrentValue, int32 MaxValue)
{
	if (MaxValue <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentValue) / static_cast<float>(MaxValue), 0.0f, 1.0f);
}
