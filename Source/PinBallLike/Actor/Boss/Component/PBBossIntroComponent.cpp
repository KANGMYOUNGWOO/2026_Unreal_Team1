#include "PBBossIntroComponent.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/UI/PBBossIntroWidget.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"

UPBBossIntroComponent::UPBBossIntroComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	BossIntroWidgetClass = TSoftClassPtr<UPBBossIntroWidget>(FSoftObjectPath(
		TEXT("/Game/Blueprints/Boss/UI/WBP_BossIntro.WBP_BossIntro_C")));
}

void UPBBossIntroComponent::BeginPlay()
{
	Super::BeginPlay();

	RegisterBattlePhaseListener();
}

void UPBBossIntroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterBattlePhaseListener();
	RemoveBossIntroWidget();

	Super::EndPlay(EndPlayReason);
}

void UPBBossIntroComponent::CompleteBossIntro()
{
	if (!IsBossIntroActive || !UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	IsBossIntroActive = false;
	RemoveBossIntroWidget();

	FPBBattleBossIntroCompletedMessage Message;
	Message.BossActor = GetOwner();
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Boss_Intro_Completed,
		Message);
}

void UPBBossIntroComponent::RegisterBattlePhaseListener()
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	BattlePhaseChangedListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePhaseChangedMessage>(
		GameplayTags::Event_Battle_Phase_Changed,
		this,
		&UPBBossIntroComponent::HandleBattlePhaseChangedMessage);
}

void UPBBossIntroComponent::UnregisterBattlePhaseListener()
{
	if (!BattlePhaseChangedListenerHandle.IsValid())
	{
		return;
	}

	BattlePhaseChangedListenerHandle.Unregister();
	BattlePhaseChangedListenerHandle = FGameplayMessageListenerHandle();
}

void UPBBossIntroComponent::HandleBattlePhaseChangedMessage(
	FGameplayTag Channel,
	const FPBBattlePhaseChangedMessage& Message)
{
	static_cast<void>(Channel);

	if (Message.NewPhase != EPBBattleLevelPhase::BossIntro)
	{
		return;
	}

	IsBossIntroActive = true;
	OnBossIntroStarted.Broadcast();

	if (!BossIntroWidgetClass.IsNull())
	{
		CreateBossIntroWidget();
		return;
	}

	CompleteBossIntro();
}

void UPBBossIntroComponent::CreateBossIntroWidget()
{
	if (BossIntroWidget || BossIntroWidgetClass.IsNull())
	{
		return;
	}

	APBBossBase* Boss = Cast<APBBossBase>(GetOwner());
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (!Boss || !PlayerController)
	{
		CompleteBossIntro();
		return;
	}

	const TSubclassOf<UPBBossIntroWidget> LoadedWidgetClass = BossIntroWidgetClass.LoadSynchronous();
	if (!LoadedWidgetClass)
	{
		CompleteBossIntro();
		return;
	}

	BossIntroWidget = CreateWidget<UPBBossIntroWidget>(PlayerController, LoadedWidgetClass);
	if (!BossIntroWidget)
	{
		CompleteBossIntro();
		return;
	}

	BossIntroWidget->OnBossIntroFinished.AddUniqueDynamic(this, &UPBBossIntroComponent::HandleBossIntroFinished);
	BossIntroWidget->SetBoss(Boss);
	BossIntroWidget->AddToViewport(BossIntroWidgetZOrder);
	BossIntroWidget->PlayBossIntroAnimation();
}

void UPBBossIntroComponent::RemoveBossIntroWidget()
{
	if (!BossIntroWidget)
	{
		return;
	}

	BossIntroWidget->OnBossIntroFinished.RemoveDynamic(this, &UPBBossIntroComponent::HandleBossIntroFinished);
	BossIntroWidget->ClearBoss();
	BossIntroWidget->RemoveFromParent();
	BossIntroWidget = nullptr;
}

void UPBBossIntroComponent::HandleBossIntroFinished()
{
	CompleteBossIntro();
}
