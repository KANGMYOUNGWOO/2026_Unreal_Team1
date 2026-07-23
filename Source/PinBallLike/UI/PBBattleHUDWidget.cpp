#include "PBBattleHUDWidget.h"

#include "AssetRegistry/AssetData.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
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
#include "PinBallLike/Struct/Deck/PBBallDeckSlot.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckAssetLoadService.h"
#include "PinBallLike/Subsystem/Deck/PBBallDeckSubsystem.h"
#include "PinBallLike/Table/Ball/DataAsset/PBBallDataAsset.h"
#include "PinBallLike/Table/Ball/PBBallAssetIds.h"
#include "PinBallLike/UI/Loading/PBLoadingScreenController.h"
#include "TimerManager.h"

void UPBBattleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DisplayedCombo = 0;
	ResetComboVisualState();
	LoadingScreenController = MakeUnique<FPBLoadingScreenController>();
	LoadingScreenController->Show(GetWorld());
	CacheBallPanels();
	CacheDeckSubsystem();
	CachePartyController();
	BindDeckEvents();
	EnsureDeckOverviewWidget();
	if (DeckOverviewWidget)
	{
		DeckOverviewWidget->SetDeploymentPinnedOpen(false);
	}
	RegisterBattleMessageListeners();
	BindComboEvents();
	ScheduleRefreshBallPanels(true);
	RefreshDeckOverview();
	RefreshComboText();
	RefreshLaunchCountText();

	if (BattleGameState)
	{
		const EPBBattleLevelPhase CurrentPhase = BattleGameState->GetBattleLevelPhase();
		ApplyBattlePhaseToDeckOverview(EPBBattleLevelPhase::DataLoading, CurrentPhase);
		ApplyBattlePhaseToLoadingScreen(CurrentPhase);
	}
}

void UPBBattleHUDWidget::NativeDestruct()
{
	ResetComboVisualState();

	if (LoadingScreenController)
	{
		LoadingScreenController->Shutdown();
		LoadingScreenController.Reset();
	}
	UnregisterBattleMessageListeners();
	UnbindComboEvents();
	UnbindDeckEvents();
	UnbindDisplayedBallEvents();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboBindRetryTimerHandle);
		World->GetTimerManager().ClearTimer(BallPanelRefreshRetryTimerHandle);
	}

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

void UPBBattleHUDWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateComboPulse(InDeltaTime);
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
	CacheDeckSubsystem();
	CachePartyController();
	UnbindDisplayedBallEvents();

	bool bNeedsRetry = false;
	for (int32 PanelIndex = 0; PanelIndex < MaxBallPanelCount; ++PanelIndex)
	{
		const int32 ExpectedBallInstanceId = DeckSubsystem
			? DeckSubsystem->GetSlotBallInstanceId(EPBBallDeckSlotType::Deployment, PanelIndex)
			: INDEX_NONE;
		APBBallBase* Ball = FindPartyBallForDeploymentSlot(PanelIndex);
		SetBallPanel(PanelIndex, Ball);
		bNeedsRetry |= ExpectedBallInstanceId != INDEX_NONE && !IsValid(Ball);
		if (IsValid(Ball))
		{
			Ball->OnDestroyed.AddUniqueDynamic(this, &UPBBattleHUDWidget::HandleDisplayedBallDestroyed);
			DisplayedBalls.Add(Ball);
		}
	}

	if (bNeedsRetry && BallPanelRefreshRetryCount < MaxBallPanelRefreshRetryCount)
	{
		++BallPanelRefreshRetryCount;
		ScheduleRefreshBallPanels();
		return;
	}

	BallPanelRefreshRetryCount = 0;
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

void UPBBattleHUDWidget::ApplyBattlePhaseToDeckOverview(
	const EPBBattleLevelPhase PreviousPhase,
	const EPBBattleLevelPhase NewPhase)
{
	EnsureDeckOverviewWidget();
	if (!DeckOverviewWidget)
	{
		return;
	}

	if (NewPhase == EPBBattleLevelPhase::BallDeployment)
	{
		RefreshDeckOverview();
		if (PreviousPhase == EPBBattleLevelPhase::Combat)
		{
			DeckOverviewWidget->SetDeploymentPinnedOpen(true);
			DeckOverviewWidget->CloseDeck();
			DeckOverviewWidget->OpenDeployment();
			return;
		}

		DeckOverviewWidget->SetDeploymentPinnedOpen(false);
		DeckOverviewWidget->OpenAll();
	}
	else if (NewPhase == EPBBattleLevelPhase::Combat)
	{
		DeckOverviewWidget->SetDeploymentPinnedOpen(false);
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

	const int32 ChildCount = BallPanelContainer->GetChildrenCount();
	for (int32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		if (UPBBallStatusWidget* BallPanel = Cast<UPBBallStatusWidget>(
			BallPanelContainer->GetChildAt(ChildIndex)))
		{
			if (BallPanels.Num() < MaxBallPanelCount)
			{
				BallPanels.Add(BallPanel);
			}
		}
	}

	if (BallPanels.Num() != MaxBallPanelCount)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[BattleHUD] Ball panel slot composition mismatch. Panels=%d Expected=%d"),
			BallPanels.Num(),
			MaxBallPanelCount);
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

void UPBBattleHUDWidget::CacheBattleGameState()
{
	if (IsValid(BattleGameState))
	{
		return;
	}

	const UWorld* World = GetWorld();
	BattleGameState = World ? World->GetGameState<APBBattleGameState>() : nullptr;
}

void UPBBattleHUDWidget::BindComboEvents()
{
	if (bComboEventsBound)
	{
		return;
	}

	CacheBattleGameState();
	if (!BattleGameState)
	{
		ScheduleBindComboEvents();
		return;
	}

	BattleGameState->OnBattleComboChanged.AddUniqueDynamic(this, &UPBBattleHUDWidget::HandleBattleComboChanged);
	BattleGameState->OnBattleLaunchCountChanged.AddUniqueDynamic(this, &UPBBattleHUDWidget::HandleBattleLaunchCountChanged);
	bComboEventsBound = true;
	bLaunchCountEventsBound = true;
	RefreshComboText();
	RefreshLaunchCountText();
}

void UPBBattleHUDWidget::UnbindComboEvents()
{
	if (!bComboEventsBound || !BattleGameState)
	{
		return;
	}

	BattleGameState->OnBattleComboChanged.RemoveDynamic(this, &UPBBattleHUDWidget::HandleBattleComboChanged);
	if (bLaunchCountEventsBound)
	{
		BattleGameState->OnBattleLaunchCountChanged.RemoveDynamic(this, &UPBBattleHUDWidget::HandleBattleLaunchCountChanged);
		bLaunchCountEventsBound = false;
	}
	bComboEventsBound = false;
	BattleGameState = nullptr;
}

void UPBBattleHUDWidget::ScheduleBindComboEvents()
{
	if (UWorld* World = GetWorld())
	{
		ComboBindRetryTimerHandle = World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UPBBattleHUDWidget::BindComboEvents));
	}
}

void UPBBattleHUDWidget::RefreshComboText()
{
	if (!Text_Combo)
	{
		return;
	}

	CacheBattleGameState();
	const int32 CurrentCombo = BattleGameState ? BattleGameState->GetCombo() : 0;
	ApplyComboText(CurrentCombo);
}

void UPBBattleHUDWidget::ApplyComboText(const int32 CurrentCombo)
{
	if (!Text_Combo)
	{
		return;
	}

	const int32 PreviousCombo = DisplayedCombo;
	DisplayedCombo = FMath::Max(0, CurrentCombo);
	Text_Combo->SetText(FText::Format(
		NSLOCTEXT("BattleHUD", "ComboTextFormat", "COMBO {0}"),
		DisplayedCombo));

	if (DisplayedCombo <= 0)
	{
		ResetComboVisualState();
		Text_Combo->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Text_Combo->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (DisplayedCombo > PreviousCombo)
	{
		const bool bCrossedMilestone = ComboMilestoneInterval > 0
			&& DisplayedCombo / ComboMilestoneInterval > PreviousCombo / ComboMilestoneInterval;
		StartComboPulse(bCrossedMilestone);
	}
	else if (DisplayedCombo < PreviousCombo)
	{
		ResetComboVisualState();
	}
}

void UPBBattleHUDWidget::RefreshLaunchCountText()
{
	if (!Text_LaunchCount)
	{
		return;
	}

	CacheBattleGameState();
	const int32 RemainingLaunchCount = BattleGameState ? BattleGameState->GetRemainingBattleLaunchCount() : 0;
	ApplyLaunchCountText(RemainingLaunchCount);
}

void UPBBattleHUDWidget::ApplyLaunchCountText(const int32 RemainingLaunchCount)
{
	if (!Text_LaunchCount)
	{
		return;
	}

	Text_LaunchCount->SetText(FText::Format(
		NSLOCTEXT("BattleHUD", "LaunchCountTextFormat", "남은 출격: {0}"),
		FText::AsNumber(FMath::Max(0, RemainingLaunchCount))));
	Text_LaunchCount->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPBBattleHUDWidget::StartComboPulse(const bool bMilestone)
{
	if (!Text_Combo)
	{
		return;
	}

	bComboPulseActive = true;
	ComboPulseElapsed = 0.0f;
	ActiveComboPulseScale = ComboPulseScale * (bMilestone ? ComboMilestoneScaleMultiplier : 1.0f);
	ActiveComboPulseLift = ComboPulseLift * (bMilestone ? ComboMilestoneScaleMultiplier : 1.0f);
	ActiveComboPulseColor = bMilestone ? ComboMilestoneColor : ComboPulseColor;
	UpdateComboPulse(0.0f);
}

void UPBBattleHUDWidget::UpdateComboPulse(const float DeltaTime)
{
	if (!bComboPulseActive || !Text_Combo)
	{
		return;
	}

	ComboPulseElapsed += FMath::Max(0.0f, DeltaTime);
	const float Duration = FMath::Max(ComboPulseDuration, UE_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(ComboPulseElapsed / Duration, 0.0f, 1.0f);
	const float SettledAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 3.0f);
	const float Scale = FMath::Lerp(1.0f + ActiveComboPulseScale, 1.0f, SettledAlpha);

	Text_Combo->SetRenderScale(FVector2D(Scale));
	Text_Combo->SetRenderTranslation(FVector2D(0.0f, FMath::Lerp(ActiveComboPulseLift, 0.0f, SettledAlpha)));
	Text_Combo->SetRenderOpacity(FMath::Lerp(0.45f, 1.0f, SettledAlpha));
	Text_Combo->SetColorAndOpacity(FSlateColor(FMath::Lerp(ActiveComboPulseColor, FLinearColor::White, SettledAlpha)));

	if (Alpha >= 1.0f)
	{
		ResetComboVisualState();
	}
}

void UPBBattleHUDWidget::ResetComboVisualState()
{
	bComboPulseActive = false;
	ComboPulseElapsed = 0.0f;
	ActiveComboPulseScale = 0.0f;
	ActiveComboPulseLift = 0.0f;
	ActiveComboPulseColor = FLinearColor::White;

	if (Text_Combo)
	{
		Text_Combo->SetRenderScale(FVector2D(1.0f));
		Text_Combo->SetRenderTranslation(FVector2D::ZeroVector);
		Text_Combo->SetRenderOpacity(1.0f);
		Text_Combo->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
}

void UPBBattleHUDWidget::ScheduleRefreshBallPanels(const bool bResetRetryCount)
{
	if (UWorld* World = GetWorld())
	{
		if (bResetRetryCount)
		{
			BallPanelRefreshRetryCount = 0;
		}
		World->GetTimerManager().ClearTimer(BallPanelRefreshRetryTimerHandle);
		BallPanelRefreshRetryTimerHandle = World->GetTimerManager().SetTimerForNextTick(
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

APBBallBase* UPBBattleHUDWidget::FindPartyBallForDeploymentSlot(const int32 SlotIndex) const
{
	if (!DeckSubsystem || !PartyController)
	{
		return nullptr;
	}

	const int32 BallInstanceId = DeckSubsystem->GetSlotBallInstanceId(
		EPBBallDeckSlotType::Deployment,
		SlotIndex);
	if (BallInstanceId == INDEX_NONE)
	{
		return nullptr;
	}

	for (const TObjectPtr<APBBallBase>& PartyBall : PartyController->GetPartyBalls())
	{
		APBBallBase* Ball = PartyBall.Get();
		if (IsValid(Ball) && Ball->GetBallInstanceId() == BallInstanceId)
		{
			return Ball;
		}
	}

	return nullptr;
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
	if (UTexture2D* LoadedIcon = AssetLoadService ? AssetLoadService->GetLoadedBallIcon(Ball->GetBallInstanceId()) : nullptr)
	{
		return LoadedIcon;
	}

	const FPrimaryAssetId BallAssetId(PBBallAssetIds::Type::BallData, Ball->GetBallId());
	FAssetData BallAssetData;
	if (!Ball->GetBallId().IsNone() && UAssetManager::Get().GetPrimaryAssetData(BallAssetId, BallAssetData))
	{
		const UPBBallDataAsset* BallDataAsset = Cast<UPBBallDataAsset>(BallAssetData.GetAsset());
		return BallDataAsset ? BallDataAsset->BallIcon.LoadSynchronous() : nullptr;
	}

	return nullptr;
}

void UPBBattleHUDWidget::HandleDeploymentSlotChanged(const int32 SlotIndex, const int32 BallInstanceId)
{
	(void)SlotIndex;
	(void)BallInstanceId;
	ScheduleRefreshBallPanels(true);
}

void UPBBattleHUDWidget::HandleDeploymentChanged()
{
	ScheduleRefreshBallPanels(true);
}

void UPBBattleHUDWidget::HandleDisplayedBallDestroyed(AActor* DestroyedActor)
{
	(void)DestroyedActor;
	ScheduleRefreshBallPanels(true);
}

void UPBBattleHUDWidget::HandleBattleComboChanged(const int32 CurrentCombo)
{
	ApplyComboText(CurrentCombo);
}

void UPBBattleHUDWidget::HandleBattleLaunchCountChanged(const int32 PreviousCount, const int32 NewCount)
{
	(void)PreviousCount;
	ApplyLaunchCountText(NewCount);
}

void UPBBattleHUDWidget::HandleBattlePhaseChangedMessage(
	FGameplayTag Channel,
	const FPBBattlePhaseChangedMessage& Message)
{
	(void)Channel;
	UE_LOG(LogTemp, Log, TEXT("[BattleHUD] Battle phase changed. NewPhase=%d"), static_cast<int32>(Message.NewPhase));
	ApplyBattlePhaseToDeckOverview(Message.PreviousPhase, Message.NewPhase);
	ApplyBattlePhaseToLoadingScreen(Message.NewPhase);
}
