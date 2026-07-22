#include "PBBattleHUDWidget.h"

#include "Components/Image.h"
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
#include "PinBallLike/UI/Loading/PBLoadingScreenController.h"
#include "TimerManager.h"

void UPBBattleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LoadingScreenController = MakeUnique<FPBLoadingScreenController>();
	LoadingScreenController->Show(GetWorld());
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
		const EPBBattleLevelPhase CurrentPhase = BattleGameState->GetBattleLevelPhase();
		ApplyBattlePhaseToDeckOverview(CurrentPhase);
		ApplyBattlePhaseToLoadingScreen(CurrentPhase);
	}
}

void UPBBattleHUDWidget::NativeDestruct()
{
	if (LoadingScreenController)
	{
		LoadingScreenController->Shutdown();
		LoadingScreenController.Reset();
	}
	UnregisterBattleMessageListeners();
	UnbindDeckEvents();
	UnbindDisplayedBallEvents();

	for (UPBBallStatusWidget* BallPanel : BallPanels)
	{
		if (BallPanel)
		{
			BallPanel->ClearBall();
		}
	}
	BallPanels.Reset();
	BallPanelInputIndicators.Reset();

	if (DeckOverviewWidget)
	{
		DeckOverviewWidget->ShutdownOverview();
	}

	Super::NativeDestruct();
}

void UPBBattleHUDWidget::ApplyBattlePhaseToLoadingScreen(const EPBBattleLevelPhase NewPhase)
{
	if (NewPhase == EPBBattleLevelPhase::DataLoading
		|| NewPhase == EPBBattleLevelPhase::LevelPreparing)
	{
		LoadingScreenController->Show(GetWorld());
		return;
	}

	LoadingScreenController->Hide();
}

void UPBBattleHUDWidget::RefreshBallPanels()
{
	CachePartyController();
	UnbindDisplayedBallEvents();

	TArray<APBBallBase*> PartyBalls;
	if (PartyController)
	{
		PartyBalls = PartyController->GetValidPartyBalls();
	}

	for (int32 PanelIndex = 0; PanelIndex < MaxBallPanelCount; ++PanelIndex)
	{
		APBBallBase* Ball = PartyBalls.IsValidIndex(PanelIndex) ? PartyBalls[PanelIndex] : nullptr;
		SetBallPanel(PanelIndex, Ball);
		if (IsValid(Ball))
		{
			Ball->OnDestroyed.AddUniqueDynamic(this, &UPBBattleHUDWidget::HandleDisplayedBallDestroyed);
			DisplayedBalls.Add(Ball);
		}
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
	BallPanelInputIndicators.Reset();
	if (!BallPanelContainer)
	{
		return;
	}

	UWidget* PendingInputIndicator = nullptr;
	const int32 ChildCount = BallPanelContainer->GetChildrenCount();
	for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		UWidget* ChildWidget = BallPanelContainer->GetChildAt(ChildIndex);
		if (UImage* InputIndicator = Cast<UImage>(ChildWidget))
		{
			PendingInputIndicator = InputIndicator;
		}
		else if (UPBBallStatusWidget* BallPanel = Cast<UPBBallStatusWidget>(ChildWidget))
		{
			if (BallPanels.Num() < MaxBallPanelCount)
			{
				BallPanelInputIndicators.Add(PendingInputIndicator);
				BallPanels.Add(BallPanel);
				PendingInputIndicator = nullptr;
			}
		}
	}

	int32 ValidIndicatorCount = 0;
	for (const TObjectPtr<UWidget>& Indicator : BallPanelInputIndicators)
	{
		if (IsValid(Indicator))
		{
			++ValidIndicatorCount;
		}
	}
	if (BallPanels.Num() != MaxBallPanelCount
		|| ValidIndicatorCount != MaxBallPanelCount)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BattleHUD] Ball panel slot composition mismatch. Panels=%d Indicators=%d Expected=%d"),
			BallPanels.Num(),
			ValidIndicatorCount,
			MaxBallPanelCount);
	}

	for (int32 PanelIndex = 0; PanelIndex < MaxBallPanelCount; ++PanelIndex)
	{
		SetBallPanelSlotVisibility(PanelIndex, false);
	}
}

void UPBBattleHUDWidget::UnbindDisplayedBallEvents()
{
	for (const TWeakObjectPtr<APBBallBase>& DisplayedBall : DisplayedBalls)
	{
		if (APBBallBase* Ball = DisplayedBall.Get())
		{
			Ball->OnDestroyed.RemoveDynamic(this, &UPBBattleHUDWidget::HandleDisplayedBallDestroyed);
		}
	}
	DisplayedBalls.Reset();
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
	if (!IsValid(Ball))
	{
		if (BallPanel)
		{
			BallPanel->ClearBall();
		}
		SetBallPanelSlotVisibility(PanelIndex, false);
		return;
	}

	if (!BallPanel)
	{
		SetBallPanelSlotVisibility(PanelIndex, false);
		return;
	}

	BallPanel->SetBall(Ball, GetBallIcon(Ball));
	SetBallPanelSlotVisibility(PanelIndex, true);
}

void UPBBattleHUDWidget::SetBallPanelSlotVisibility(const int32 PanelIndex, const bool bVisible)
{
	if (UPBBallStatusWidget* BallPanel = GetBallPanel(PanelIndex))
	{
		BallPanel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (UWidget* InputIndicator = GetBallPanelInputIndicator(PanelIndex))
	{
		InputIndicator->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

UPBBallStatusWidget* UPBBattleHUDWidget::GetBallPanel(const int32 PanelIndex) const
{
	return BallPanels.IsValidIndex(PanelIndex) ? BallPanels[PanelIndex] : nullptr;
}

UWidget* UPBBattleHUDWidget::GetBallPanelInputIndicator(const int32 PanelIndex) const
{
	return BallPanelInputIndicators.IsValidIndex(PanelIndex)
		? BallPanelInputIndicators[PanelIndex]
		: nullptr;
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

void UPBBattleHUDWidget::HandleDisplayedBallDestroyed(AActor* DestroyedActor)
{
	(void)DestroyedActor;
	ScheduleRefreshBallPanels();
}

void UPBBattleHUDWidget::HandleBattlePhaseChangedMessage(
	FGameplayTag Channel,
	const FPBBattlePhaseChangedMessage& Message)
{
	(void)Channel;
	UE_LOG(LogTemp, Log, TEXT("[BattleHUD] Battle phase changed. NewPhase=%d"), static_cast<int32>(Message.NewPhase));
	ApplyBattlePhaseToDeckOverview(Message.NewPhase);
	ApplyBattlePhaseToLoadingScreen(Message.NewPhase);
}
