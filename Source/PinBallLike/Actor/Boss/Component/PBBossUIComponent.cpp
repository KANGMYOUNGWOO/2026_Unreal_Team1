#include "PBBossUIComponent.h"

#include "GameFramework/PlayerController.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/UI/PBBossUILayerWidget.h"

UPBBossUIComponent::UPBBossUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	BossUILayerClass = TSoftClassPtr<UPBBossUILayerWidget>(FSoftObjectPath(
		TEXT("/Game/Blueprints/Boss/UI/WBP_BossLayer.WBP_BossLayer_C")));
}

void UPBBossUIComponent::BeginPlay()
{
	Super::BeginPlay();
	CreateBossUILayer();
}

void UPBBossUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveBossUILayer();
	Super::EndPlay(EndPlayReason);
}

void UPBBossUIComponent::ShowEnrageWarning()
{
	if (BossUILayer)
	{
		BossUILayer->ShowEnrageWarning();
	}
}

void UPBBossUIComponent::HideEnrageWarning()
{
	if (BossUILayer)
	{
		BossUILayer->HideEnrageWarning();
	}
}

UPBBossIntroWidget* UPBBossUIComponent::GetBossIntroWidget() const
{
	return BossUILayer ? BossUILayer->GetBossIntroWidget() : nullptr;
}

void UPBBossUIComponent::ConfigureBossUILayerClass(TSoftClassPtr<UPBBossUILayerWidget> NewBossUILayerClass)
{
	if (!NewBossUILayerClass.IsNull())
	{
		BossUILayerClass = NewBossUILayerClass;
	}
}

void UPBBossUIComponent::CreateBossUILayer()
{
	if (BossUILayer || BossUILayerClass.IsNull())
	{
		return;
	}

	APBBossBase* Boss = Cast<APBBossBase>(GetOwner());
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	const TSubclassOf<UPBBossUILayerWidget> LoadedWidgetClass = BossUILayerClass.LoadSynchronous();
	if (!Boss || !PlayerController || !LoadedWidgetClass)
	{
		return;
	}

	BossUILayer = CreateWidget<UPBBossUILayerWidget>(PlayerController, LoadedWidgetClass);
	if (!BossUILayer)
	{
		return;
	}

	BossUILayer->SetBoss(Boss);
	BossUILayer->AddToViewport();
}

void UPBBossUIComponent::RemoveBossUILayer()
{
	if (!BossUILayer)
	{
		return;
	}

	BossUILayer->ClearBoss();
	BossUILayer->RemoveFromParent();
	BossUILayer = nullptr;
}
