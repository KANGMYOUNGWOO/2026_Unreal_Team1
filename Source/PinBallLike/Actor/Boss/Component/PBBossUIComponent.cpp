#include "PBBossUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/UI/PBBossStatusWidget.h"

UPBBossUIComponent::UPBBossUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPBBossUIComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateBossStatusWidget();
}

void UPBBossUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveBossStatusWidget();

	Super::EndPlay(EndPlayReason);
}

void UPBBossUIComponent::CreateBossStatusWidget()
{
	if (BossStatusWidget || !BossStatusWidgetClass)
	{
		return;
	}

	APBBossBase* Boss = Cast<APBBossBase>(GetOwner());
	if (!Boss)
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	BossStatusWidget = CreateWidget<UPBBossStatusWidget>(PlayerController, BossStatusWidgetClass);
	if (!BossStatusWidget)
	{
		return;
	}

	BossStatusWidget->SetBoss(Boss);
	BossStatusWidget->AddToViewport(BossStatusWidgetZOrder);
}

void UPBBossUIComponent::RemoveBossStatusWidget()
{
	if (!BossStatusWidget)
	{
		return;
	}

	BossStatusWidget->ClearBoss();
	BossStatusWidget->RemoveFromParent();
	BossStatusWidget = nullptr;
}

void UPBBossUIComponent::ConfigureBossStatusWidget(TSubclassOf<UPBBossStatusWidget> NewBossStatusWidgetClass, int32 NewBossStatusWidgetZOrder)
{
	BossStatusWidgetClass = NewBossStatusWidgetClass;
	BossStatusWidgetZOrder = NewBossStatusWidgetZOrder;
}
