#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/UI/Loading/PBLoadingScreenController.h"
#include "TimerManager.h"
#include "PBBattleHUDWidget.generated.h"

class UPBDeckOverviewWidget;
class APBBattleGameState;
class APBCombatPartyController;
class APBBallBase;
class UPBBallDeckSubsystem;
class UPBBallStatusWidget;
class UPanelWidget;
class UTextBlock;
class UTexture2D;
class AActor;
enum class EPBBattleLevelPhase : uint8;
struct FPBBattlePhaseChangedMessage;

UCLASS()
class PINBALLLIKE_API UPBBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BattleHUD|Ball")
	void RefreshBallPanels();

	UFUNCTION(BlueprintCallable, Category = "BattleHUD|Synergy")
	void RefreshSynergyPanels();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	static constexpr int32 MaxBallPanelCount = 3;

	void CacheBallPanels();
	void UnbindDisplayedBallEvents();
	void CacheDeckSubsystem();
	void CachePartyController();
	void BindDeckEvents();
	void UnbindDeckEvents();
	void EnsureDeckOverviewWidget();
	void ApplyBattlePhaseToDeckOverview(EPBBattleLevelPhase PreviousPhase, EPBBattleLevelPhase NewPhase);
	void ApplyBattlePhaseToLoadingScreen(EPBBattleLevelPhase NewPhase);
	void RegisterBattleMessageListeners();
	void UnregisterBattleMessageListeners();
	void CacheBattleGameState();
	void BindComboEvents();
	void UnbindComboEvents();
	void ScheduleBindComboEvents();
	void RefreshComboText();
	void ApplyComboText(int32 CurrentCombo);
	void RefreshLaunchCountText();
	void ApplyLaunchCountText(int32 RemainingLaunchCount);
	void StartComboPulse(bool bMilestone);
	void UpdateComboPulse(float DeltaTime);
	void ResetComboVisualState();
	void ScheduleRefreshBallPanels(bool bResetRetryCount = false);
	void RefreshDeckOverview();
	APBBallBase* FindPartyBallForDeploymentSlot(int32 SlotIndex) const;
	void SetBallPanel(int32 PanelIndex, APBBallBase* Ball);
	UPBBallStatusWidget* GetBallPanel(int32 PanelIndex) const;
	UTexture2D* GetBallIcon(APBBallBase* Ball) const;

	UFUNCTION()
	void HandleDeploymentSlotChanged(int32 SlotIndex, int32 BallInstanceId);

	UFUNCTION()
	void HandleDeploymentChanged();

	UFUNCTION()
	void HandleDisplayedBallDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleBattleComboChanged(int32 CurrentCombo);

	UFUNCTION()
	void HandleBattleLaunchCountChanged(int32 PreviousCount, int32 NewCount);

	void HandleBattlePhaseChangedMessage(FGameplayTag Channel, const FPBBattlePhaseChangedMessage& Message);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPBDeckOverviewWidget> DeckOverviewWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> BallPanelContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Combo;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_LaunchCount;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBBallStatusWidget>> BallPanels;

	TArray<TWeakObjectPtr<APBBallBase>> DisplayedBalls;

	UPROPERTY(Transient)
	TObjectPtr<UPBBallDeckSubsystem> DeckSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<APBCombatPartyController> PartyController;

	UPROPERTY(Transient)
	TObjectPtr<APBBattleGameState> BattleGameState;

	FGameplayMessageListenerHandle BattlePhaseChangedListenerHandle;
	TUniquePtr<FPBLoadingScreenController> LoadingScreenController;

	FTimerHandle ComboBindRetryTimerHandle;
	FTimerHandle BallPanelRefreshRetryTimerHandle;
	int32 BallPanelRefreshRetryCount = 0;

	bool bDeckEventsBound = false;
	bool bComboEventsBound = false;
	bool bLaunchCountEventsBound = false;
	bool bComboPulseActive = false;
	float ComboPulseElapsed = 0.0f;
	float ActiveComboPulseScale = 0.0f;
	float ActiveComboPulseLift = 0.0f;
	int32 DisplayedCombo = 0;
	FLinearColor ActiveComboPulseColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "BattleHUD|Combo Animation", meta = (ClampMin = "0.05"))
	float ComboPulseDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "BattleHUD|Combo Animation", meta = (ClampMin = "0.0"))
	float ComboPulseScale = 0.24f;

	UPROPERTY(EditDefaultsOnly, Category = "BattleHUD|Combo Animation", meta = (ClampMin = "0.0"))
	float ComboPulseLift = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "BattleHUD|Combo Animation")
	FLinearColor ComboPulseColor = FLinearColor(0.25f, 0.85f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "BattleHUD|Combo Animation", meta = (ClampMin = "1"))
	int32 ComboMilestoneInterval = 10;

	UPROPERTY(EditDefaultsOnly, Category = "BattleHUD|Combo Animation", meta = (ClampMin = "1.0"))
	float ComboMilestoneScaleMultiplier = 1.45f;

	UPROPERTY(EditDefaultsOnly, Category = "BattleHUD|Combo Animation")
	FLinearColor ComboMilestoneColor = FLinearColor(1.0f, 0.65f, 0.08f, 1.0f);

	static constexpr int32 MaxBallPanelRefreshRetryCount = 30;
};
