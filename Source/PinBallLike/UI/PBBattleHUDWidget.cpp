#include "PBBattleHUDWidget.h"

#include "Components/PanelWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Ball/UI/PBBallStatusWidget.h"
#include "PinBallLike/Actor/Party/PBCombatPartyController.h"
#include "PinBallLike/Actor/Synergy/UI/PBSynergyPanelWidget.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSynergyService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckAssetLoadService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/UI/ViewModel/PBBattleHUDViewModel.h"
#include "TimerManager.h"

void UPBBattleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CacheBallPanels();
	CacheDeckSubsystem();
	CachePartyController();
	EnsureBattleHUDViewModel();
	BindDeckEvents();
	BindSynergyEvents();
	RegisterBattleMessageListeners();
	ScheduleRefreshBallPanels();
	ScheduleRefreshSynergyPanels();
}

void UPBBattleHUDWidget::NativeDestruct()
{
	UnregisterBattleMessageListeners();
	UnbindSynergyEvents();
	UnbindDeckEvents();

	for (UPBBallStatusWidget* BallPanel : BallPanels)
	{
		if (BallPanel)
		{
			BallPanel->ClearBall();
		}
	}
	BallPanels.Reset();

	if (SynergyPanelContainer)
	{
		SynergyPanelContainer->ClearChildren();
	}
	if (BattleHUDViewModel)
	{
		BattleHUDViewModel->ClearSynergyViewData();
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
	if (!SynergyPanelContainer)
	{
		return;
	}

	EnsureBattleHUDViewModel();
	if (!BattleHUDViewModel)
	{
		SynergyPanelContainer->ClearChildren();
		return;
	}

	BindSynergyEvents();
	UPBBallDeckSynergyService* SynergyService = GetSynergyService();
	if (!SynergyService)
	{
		SynergyPanelContainer->ClearChildren();
		return;
	}

	SynergyService->RefreshSynergyStatesFromPlacedDeck();
	BattleHUDViewModel->SetSynergyStates(SynergyService->GetActiveSynergyStates());
	const TArray<FPBSynergyViewData>& ViewDataList = BattleHUDViewModel->GetActiveSynergyViewData();

	SynergyPanelContainer->ClearChildren();

	UClass* WidgetClass = SynergyPanelWidgetClass.Get();
	if (!WidgetClass)
	{
		WidgetClass = UPBSynergyPanelWidget::StaticClass();
	}

	for (const FPBSynergyViewData& ViewData : ViewDataList)
	{
		UPBSynergyPanelWidget* SynergyPanel = CreateWidget<UPBSynergyPanelWidget>(this, WidgetClass);
		if (!SynergyPanel)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BattleHUD] Failed to create synergy panel. WidgetClass=%s SynergyId=%s"),
				*GetNameSafe(WidgetClass),
				*ViewData.SynergyId.ToString());
			continue;
		}

		SynergyPanel->InitializeSynergyPanel(ViewData);
		SynergyPanelContainer->AddChild(SynergyPanel);
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

UPBBallDeckSynergyService* UPBBattleHUDWidget::GetSynergyService() const
{
	return DeckSubsystem ? DeckSubsystem->GetSynergyService() : nullptr;
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

void UPBBattleHUDWidget::BindSynergyEvents()
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
		&UPBBattleHUDWidget::HandleSynergyStatesChanged);
	bSynergyEventsBound = SynergyStatesChangedHandle.IsValid();
}

void UPBBattleHUDWidget::UnbindSynergyEvents()
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

void UPBBattleHUDWidget::ScheduleRefreshSynergyPanels()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UPBBattleHUDWidget::RefreshSynergyPanels));
	}
}

void UPBBattleHUDWidget::EnsureBattleHUDViewModel()
{
	CacheDeckSubsystem();
	if (!BattleHUDViewModel)
	{
		BattleHUDViewModel = NewObject<UPBBattleHUDViewModel>(this);
	}

	if (BattleHUDViewModel)
	{
		BattleHUDViewModel->Initialize(this);
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
	ScheduleRefreshSynergyPanels();
}

void UPBBattleHUDWidget::HandleDeploymentChanged()
{
	ScheduleRefreshBallPanels();
	ScheduleRefreshSynergyPanels();
}

void UPBBattleHUDWidget::HandleSynergyStatesChanged(const TArray<FPBSynergyState>& SynergyStates)
{
	(void)SynergyStates;
	ScheduleRefreshSynergyPanels();
}

void UPBBattleHUDWidget::HandleBattlePhaseChangedMessage(
	FGameplayTag Channel,
	const FPBBattlePhaseChangedMessage& Message)
{
	(void)Channel;
	if (Message.NewPhase == EPBBattleLevelPhase::BallDeployment)
	{
		ScheduleRefreshBallPanels();
		ScheduleRefreshSynergyPanels();
	}
}
