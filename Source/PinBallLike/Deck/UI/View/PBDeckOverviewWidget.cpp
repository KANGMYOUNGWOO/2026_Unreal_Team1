#include "PBDeckOverviewWidget.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Components/Button.h"
#include "PBDeckWidget.h"
#include "PBDeploymentWidget.h"
#include "PinBallLike/Deck/UI/ViewModel/PBDeckOverviewViewModel.h"
#include "PinBallLike/GameState/PBBattleGameState.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSynergyService.h"
#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"
#include "PinBallLike/UI/Global/PBGlobalToolbarWidget.h"
#include "TimerManager.h"
#include "View/MVVMView.h"

void UPBDeckOverviewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeOverview();
	BindGlobalToolbarEvents();
}

void UPBDeckOverviewWidget::NativeDestruct()
{
	UnbindGlobalToolbarEvents();
	ShutdownOverview();

	Super::NativeDestruct();
}

void UPBDeckOverviewWidget::InitializeOverview()
{
	CacheDeckSubsystem();
	EnsureViewModel();
	BindDeckEvents();
	BindSynergyEvents();
	RefreshAll();
}

void UPBDeckOverviewWidget::ShutdownOverview()
{
	UnbindSynergyEvents();
	UnbindDeckEvents();

	if (ViewModel)
	{
		ViewModel->ClearSynergyViewData();
	}
}

void UPBDeckOverviewWidget::RefreshAll()
{
	RefreshDeck();
	RefreshDeployment();
	RefreshSynergyPanels();
}

void UPBDeckOverviewWidget::RefreshDeck()
{
	if (DeckWidget)
	{
		DeckWidget->RefreshBenchSlots();
	}
}

void UPBDeckOverviewWidget::RefreshDeployment()
{
	if (DeploymentWidget)
	{
		DeploymentWidget->RefreshDeploymentSlots();
	}
}

void UPBDeckOverviewWidget::OpenDeck()
{
	if (bIsDeckOpen || bIsDeckAnimationPlaying)
	{
		return;
	}

	bIsDeckOpen = true;
	bIsDeckAnimationPlaying = true;
	BP_PlayDeckOpenAnimation();
}

void UPBDeckOverviewWidget::CloseDeck()
{
	if (!bIsDeckOpen || bIsDeckAnimationPlaying)
	{
		return;
	}

	bIsDeckOpen = false;
	bIsDeckAnimationPlaying = true;
	BP_PlayDeckCloseAnimation();
}

void UPBDeckOverviewWidget::ToggleDeck()
{
	if (bIsDeckAnimationPlaying)
	{
		return;
	}

	if (bIsDeckOpen)
	{
		CloseDeck();
		return;
	}

	OpenDeck();
}

void UPBDeckOverviewWidget::OpenDeployment()
{
	if (bIsDeploymentOpen || bIsDeploymentAnimationPlaying)
	{
		return;
	}

	bIsDeploymentOpen = true;
	bIsDeploymentAnimationPlaying = true;
	BP_PlayDeploymentOpenAnimation();
}

void UPBDeckOverviewWidget::CloseDeployment()
{
	if (bDeploymentPinnedOpen || !bIsDeploymentOpen || bIsDeploymentAnimationPlaying)
	{
		return;
	}

	bIsDeploymentOpen = false;
	bIsDeploymentAnimationPlaying = true;
	BP_PlayDeploymentCloseAnimation();
}

void UPBDeckOverviewWidget::OpenAll()
{
	OpenDeck();
	OpenDeployment();
}

void UPBDeckOverviewWidget::CloseAll()
{
	CloseDeck();
	if (!bDeploymentPinnedOpen)
	{
		CloseDeployment();
	}
}

void UPBDeckOverviewWidget::SetDeploymentPinnedOpen(const bool bPinnedOpen)
{
	bDeploymentPinnedOpen = bPinnedOpen;
	if (bDeploymentPinnedOpen && !bIsDeploymentAnimationPlaying)
	{
		OpenDeployment();
	}
}

void UPBDeckOverviewWidget::NotifyDeckAnimationFinished()
{
	bIsDeckAnimationPlaying = false;
}

void UPBDeckOverviewWidget::NotifyDeploymentAnimationFinished()
{
	bIsDeploymentAnimationPlaying = false;
	if (bDeploymentPinnedOpen && !bIsDeploymentOpen)
	{
		OpenDeployment();
	}
}

void UPBDeckOverviewWidget::RefreshSynergyPanels()
{
	EnsureViewModel();
	if (!ViewModel)
	{
		return;
	}

	BindSynergyEvents();
	UPBBallDeckSynergyService* SynergyService = GetSynergyService();
	if (!SynergyService)
	{
		ViewModel->ClearSynergyViewData();
		BP_OnSynergyViewDataChanged(ViewModel->GetActiveSynergyViewData());
		return;
	}

	SynergyService->RefreshSynergyStatesFromPlacedDeck();
	ViewModel->SetSynergyStates(SynergyService->GetActiveSynergyStates());
	BP_OnSynergyViewDataChanged(ViewModel->GetActiveSynergyViewData());
}

void UPBDeckOverviewWidget::CacheDeckSubsystem()
{
	if (DeckSubsystem)
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	DeckSubsystem = GameInstance ? GameInstance->GetSubsystem<UPBBallDeckSubsystem>() : nullptr;
}

UPBBallDeckSynergyService* UPBDeckOverviewWidget::GetSynergyService() const
{
	return DeckSubsystem ? DeckSubsystem->GetSynergyService() : nullptr;
}

void UPBDeckOverviewWidget::BindDeckEvents()
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
		&UPBDeckOverviewWidget::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.AddUniqueDynamic(
		this,
		&UPBDeckOverviewWidget::HandleDeploymentChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.AddUniqueDynamic(
		this,
		&UPBDeckOverviewWidget::HandleDeploymentChanged);

	bDeckEventsBound = true;
}

void UPBDeckOverviewWidget::UnbindDeckEvents()
{
	if (!bDeckEventsBound || !DeckSubsystem)
	{
		return;
	}

	DeckSubsystem->OnDeploymentSlotChanged.RemoveDynamic(
		this,
		&UPBDeckOverviewWidget::HandleDeploymentSlotChanged);
	DeckSubsystem->OnDeploymentSlotsReordered.RemoveDynamic(
		this,
		&UPBDeckOverviewWidget::HandleDeploymentChanged);
	DeckSubsystem->OnDeploymentSlotsRotated.RemoveDynamic(
		this,
		&UPBDeckOverviewWidget::HandleDeploymentChanged);

	bDeckEventsBound = false;
}

void UPBDeckOverviewWidget::BindSynergyEvents()
{
	if (bSynergyEventsBound)
	{
		return;
	}

	UPBBallDeckSynergyService* SynergyService = GetSynergyService();
	if (!SynergyService)
	{
		return;
	}

	SynergyStatesChangedHandle = SynergyService->OnSynergyStatesChanged.AddUObject(
		this,
		&UPBDeckOverviewWidget::HandleSynergyStatesChanged);
	bSynergyEventsBound = SynergyStatesChangedHandle.IsValid();
}

void UPBDeckOverviewWidget::UnbindSynergyEvents()
{
	if (!bSynergyEventsBound)
	{
		return;
	}

	UPBBallDeckSynergyService* SynergyService = GetSynergyService();
	if (SynergyService && SynergyStatesChangedHandle.IsValid())
	{
		SynergyService->OnSynergyStatesChanged.Remove(SynergyStatesChangedHandle);
	}

	SynergyStatesChangedHandle.Reset();
	bSynergyEventsBound = false;
}

void UPBDeckOverviewWidget::ScheduleRefreshSynergyPanels()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UPBDeckOverviewWidget::RefreshSynergyPanels));
	}
}

void UPBDeckOverviewWidget::EnsureViewModel()
{
	CacheDeckSubsystem();
	if (!ViewModel)
	{
		ViewModel = NewObject<UPBDeckOverviewViewModel>(this);
	}

	if (ViewModel)
	{
		ViewModel->Initialize(this);
		ApplyViewModelToWidget();
	}
}

bool UPBDeckOverviewWidget::ApplyViewModelToWidget()
{
	if (!ViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeckOverview] ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(ViewModel);
	const bool bResult = View->SetViewModelByClass(ViewModelInterface);
	if (!bResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DeckOverview] ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(ViewModel));
	}
	return bResult;
}

void UPBDeckOverviewWidget::BindGlobalToolbarEvents()
{
	if (UIManagerSubsystem)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UIManagerSubsystem = IsValid(GameInstance)
		? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
		: nullptr;
	if (UIManagerSubsystem)
	{
		UIManagerSubsystem->OnGlobalDeckToggleRequested.AddUniqueDynamic(
			this,
			&UPBDeckOverviewWidget::HandleGlobalDeckToggleRequested);
	}
}

void UPBDeckOverviewWidget::UnbindGlobalToolbarEvents()
{
	if (UIManagerSubsystem)
	{
		UIManagerSubsystem->OnGlobalDeckToggleRequested.RemoveDynamic(
			this,
			&UPBDeckOverviewWidget::HandleGlobalDeckToggleRequested);
	}
	UIManagerSubsystem = nullptr;
}

bool UPBDeckOverviewWidget::ShouldCloseDeckOnlyForGlobalRequest() const
{
	const UWorld* World = GetWorld();
	return World && World->GetGameState<APBBattleGameState>();
}

bool UPBDeckOverviewWidget::ShouldIgnoreGlobalRequestForBattlePhase() const
{
	const UWorld* World = GetWorld();
	const APBBattleGameState* BattleGameState = World
		? World->GetGameState<APBBattleGameState>()
		: nullptr;
	return BattleGameState
		&& BattleGameState->GetBattleLevelPhase() == EPBBattleLevelPhase::Combat;
}

void UPBDeckOverviewWidget::HandleGlobalDeckToggleRequested()
{
	if (bIsDeckAnimationPlaying || bIsDeploymentAnimationPlaying)
	{
		return;
	}

	if (ShouldIgnoreGlobalRequestForBattlePhase())
	{
		return;
	}

	if (bIsDeckOpen)
	{
		if (bDeploymentPinnedOpen || ShouldCloseDeckOnlyForGlobalRequest())
		{
			CloseDeck();
			return;
		}

		CloseAll();
		return;
	}

	OpenAll();
}

void UPBDeckOverviewWidget::HandleDeploymentSlotChanged(const int32 SlotIndex, const int32 BallInstanceId)
{
	(void)SlotIndex;
	(void)BallInstanceId;
	ScheduleRefreshSynergyPanels();
}

void UPBDeckOverviewWidget::HandleDeploymentChanged()
{
	ScheduleRefreshSynergyPanels();
}

void UPBDeckOverviewWidget::HandleSynergyStatesChanged(const TArray<FPBSynergyState>& SynergyStates)
{
	(void)SynergyStates;
	ScheduleRefreshSynergyPanels();
}
