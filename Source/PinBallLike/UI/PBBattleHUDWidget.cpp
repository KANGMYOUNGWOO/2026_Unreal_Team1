#include "PBBattleHUDWidget.h"

#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/UI/PBBallStatusWidget.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/GameState/PBBattleGameState.h"
#include "PinBallLike/Deck/UI/View/PBDeckOverviewWidget.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckAssetLoadService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "TimerManager.h"

void UPBBattleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CacheBallPanels();
	CacheDeckSubsystem();
	CachePartyController();
	BindDeckEvents();
	EnsureDeckOverviewWidget();
	RegisterBattleMessageListeners();
	ScheduleRefreshBallPanels();
	RefreshDeckOverview();

	const UWorld* World = GetWorld();
	const APBBattleGameState* BattleGameState = World ? World->GetGameState<APBBattleGameState>() : nullptr;
	if (BattleGameState)
	{
		ApplyBattlePhaseToDeckOverview(BattleGameState->GetBattleLevelPhase());
	}
}

void UPBBattleHUDWidget::NativeDestruct()
{
	UnregisterBattleMessageListeners();
	UnbindDeckEvents();

	for (UPBBallStatusWidget* BallPanel : BallPanels)
	{
		if (BallPanel)
		{
			BallPanel->ClearBall();
		}
	}
	BallPanels.Reset();

	if (DeckOverviewWidget)
	{
		DeckOverviewWidget->ShutdownOverview();
	}

	Super::NativeDestruct();
}

void UPBBattleHUDWidget::RefreshBallPanels()
{
	CachePartyController();

	TArray<APBBallBase*> PartyBalls;
	if (PartyController)
	{
		PartyBalls = PartyController->GetValidPartyBalls();
	}

	for (int32 PanelIndex = 0; PanelIndex < MaxBallPanelCount; ++PanelIndex)
	{
		APBBallBase* Ball = PartyBalls.IsValidIndex(PanelIndex) ? PartyBalls[PanelIndex] : nullptr;
		SetBallPanel(PanelIndex, Ball);
	}
}

void UPBBattleHUDWidget::RefreshSynergyPanels()
{
	EnsureDeckOverviewWidget();
	if (DeckOverviewWidget)
	{
		DeckOverviewWidget->RefreshSynergyPanels();
	}
}

void UPBBattleHUDWidget::EnsureDeckOverviewWidget()
{
	if (DeckOverviewWidget)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[BattleHUD] DeckOverviewWidget is not bound."));
}

void UPBBattleHUDWidget::ApplyBattlePhaseToDeckOverview(const EPBBattleLevelPhase NewPhase)
{
	EnsureDeckOverviewWidget();
	if (!DeckOverviewWidget)
	{
		return;
	}

	if (NewPhase == EPBBattleLevelPhase::BallDeployment)
	{
		RefreshDeckOverview();
		DeckOverviewWidget->OpenDeployment();
	}
	else if (NewPhase == EPBBattleLevelPhase::Combat)
	{
		DeckOverviewWidget->CloseAll();
	}
}

void UPBBattleHUDWidget::CacheBallPanels()
{
	BallPanels.Reset();
	if (!BallPanelContainer)
	{
		return;
	}

	const int32 ChildCount = FMath::Min(BallPanelContainer->GetChildrenCount(), MaxBallPanelCount);
	for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		if (UPBBallStatusWidget* BallPanel = Cast<UPBBallStatusWidget>(BallPanelContainer->GetChildAt(ChildIndex)))
		{
			BallPanels.Add(BallPanel);
		}
	}
}

void UPBBattleHUDWidget::CacheDeckSubsystem()
{
	if (DeckSubsystem)
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	DeckSubsystem = GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
}

void UPBBattleHUDWidget::CachePartyController()
{
	if (IsValid(PartyController))
	{
		return;
	}

	PartyController = Cast<APBCombatPartyController>(
		UGameplayStatics::GetActorOfClass(this, APBCombatPartyController::StaticClass()));
}

void UPBBattleHUDWidget::BindDeckEvents()
{
	if (bDeckEventsBound)
	{
		return;
	}

	CacheDeckSubsystem();
	if (!DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.AddUniqueDynamic(
		this,
		&UPBBattleHUDWidget::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.AddUniqueDynamic(
		this,
		&UPBBattleHUDWidget::HandleDeploymentChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.AddUniqueDynamic(
		this,
		&UPBBattleHUDWidget::HandleDeploymentChanged);

	bDeckEventsBound = true;
}

void UPBBattleHUDWidget::UnbindDeckEvents()
{
	if (!bDeckEventsBound || !DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(
		this,
		&UPBBattleHUDWidget::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.RemoveDynamic(
		this,
		&UPBBattleHUDWidget::HandleDeploymentChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.RemoveDynamic(
		this,
		&UPBBattleHUDWidget::HandleDeploymentChanged);

	bDeckEventsBound = false;
}

void UPBBattleHUDWidget::RegisterBattleMessageListeners()
{
	if (!UGameplayMessageSubsystem::HasInstance(this) || BattlePhaseChangedListenerHandle.IsValid())
	{
		return;
	}

	BattlePhaseChangedListenerHandle =
		UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBattlePhaseChangedMessage>(
			GameplayTags::Event_Battle_Phase_Changed,
			this,
			&UPBBattleHUDWidget::HandleBattlePhaseChangedMessage);
}

void UPBBattleHUDWidget::UnregisterBattleMessageListeners()
{
	if (BattlePhaseChangedListenerHandle.IsValid())
	{
		BattlePhaseChangedListenerHandle.Unregister();
		BattlePhaseChangedListenerHandle = FGameplayMessageListenerHandle();
	}
}

void UPBBattleHUDWidget::ScheduleRefreshBallPanels()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UPBBattleHUDWidget::RefreshBallPanels));
	}
}

void UPBBattleHUDWidget::RefreshDeckOverview()
{
	EnsureDeckOverviewWidget();
	if (DeckOverviewWidget)
	{
		DeckOverviewWidget->RefreshAll();
	}
}

void UPBBattleHUDWidget::SetBallPanel(const int32 PanelIndex, APBBallBase* Ball)
{
	UPBBallStatusWidget* BallPanel = GetBallPanel(PanelIndex);
	if (!BallPanel)
	{
		return;
	}

	if (!IsValid(Ball))
	{
		BallPanel->ClearBall();
		return;
	}

	BallPanel->SetBall(Ball, GetBallIcon(Ball));
}

UPBBallStatusWidget* UPBBattleHUDWidget::GetBallPanel(const int32 PanelIndex) const
{
	return BallPanels.IsValidIndex(PanelIndex) ? BallPanels[PanelIndex] : nullptr;
}

UTexture2D* UPBBattleHUDWidget::GetBallIcon(APBBallBase* Ball) const
{
	if (!DeckSubsystem || !IsValid(Ball))
	{
		return nullptr;
	}

	const UPBBallDeckAssetLoadService* AssetLoadService = DeckSubsystem->GetAssetLoadService();
	return AssetLoadService ? AssetLoadService->GetLoadedBallIcon(Ball->GetBallInstanceId()) : nullptr;
}

void UPBBattleHUDWidget::HandleDeploymentSlotChanged(const int32 SlotIndex, const int32 BallInstanceId)
{
	(void)SlotIndex;
	(void)BallInstanceId;
	ScheduleRefreshBallPanels();
}

void UPBBattleHUDWidget::HandleDeploymentChanged()
{
	ScheduleRefreshBallPanels();
}

void UPBBattleHUDWidget::HandleBattlePhaseChangedMessage(
	FGameplayTag Channel,
	const FPBBattlePhaseChangedMessage& Message)
{
	(void)Channel;
	UE_LOG(LogTemp, Log, TEXT("[BattleHUD] Battle phase changed. NewPhase=%d"), static_cast<int32>(Message.NewPhase));
	ApplyBattlePhaseToDeckOverview(Message.NewPhase);
}
