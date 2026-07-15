#include "PBBossUILayerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBoss.h"
#include "PinBallLike/Actor/Boss/Golem/UI/PBGolemHandStatusWidget.h"
#include "PBBossEnrageWidget.h"
#include "PBBossIntroWidget.h"
#include "PBBossStatusWidget.h"

void UPBBossUILayerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	CacheBossWidgets();
}

void UPBBossUILayerWidget::NativeDestruct()
{
	ClearBoss();
	Super::NativeDestruct();
}

void UPBBossUILayerWidget::SetBoss(APBBossBase* NewBoss)
{
	CacheBossWidgets();

	if (BossStatusWidget)
	{
		BossStatusWidget->SetBoss(NewBoss);
	}

	if (GolemHandStatusWidget)
	{
		GolemHandStatusWidget->SetGolemBoss(Cast<APBGolemBoss>(NewBoss));
	}

	if (BossIntroWidget)
	{
		BossIntroWidget->SetBoss(NewBoss);
		BossIntroWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	HideEnrageWarning();
}
void UPBBossUILayerWidget::ClearBoss()
{
	if (BossStatusWidget)
	{
		BossStatusWidget->ClearBoss();
	}

	if (GolemHandStatusWidget)
	{
		GolemHandStatusWidget->ClearGolemBoss();
	}

	if (BossIntroWidget)
	{
		BossIntroWidget->ClearBoss();
		BossIntroWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	HideEnrageWarning();
}

void UPBBossUILayerWidget::ShowEnrageWarning()
{
	if (BossEnrageWidget)
	{
		BossEnrageWidget->ShowEnrageWarning();
	}
}

void UPBBossUILayerWidget::HideEnrageWarning()
{
	if (BossEnrageWidget)
	{
		BossEnrageWidget->HideEnrageWarning();
	}
}

UPBBossIntroWidget* UPBBossUILayerWidget::GetBossIntroWidget() const
{
	return BossIntroWidget;
}

void UPBBossUILayerWidget::CacheBossWidgets()
{
	if (!WidgetTree)
	{
		return;
	}

	WidgetTree->ForEachWidget(
		[this](UWidget* Widget)
		{
			if (!BossStatusWidget)
			{
				BossStatusWidget = Cast<UPBBossStatusWidget>(Widget);
			}

			if (!GolemHandStatusWidget)
			{
				GolemHandStatusWidget = Cast<UPBGolemHandStatusWidget>(Widget);
			}

			if (!BossEnrageWidget)
			{
				BossEnrageWidget = Cast<UPBBossEnrageWidget>(Widget);
			}

			if (!BossIntroWidget)
			{
				BossIntroWidget = Cast<UPBBossIntroWidget>(Widget);
			}
		});
}
