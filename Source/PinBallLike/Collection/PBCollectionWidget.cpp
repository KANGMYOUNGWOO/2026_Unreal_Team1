#include "PBCollectionWidget.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "PinBallLike/Collection/UI/PBCollectionTabWidgetBase.h"
#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"
#include "UObject/SoftObjectPath.h"

namespace
{
const FSoftClassPath CollectionWidgetBlueprintPath(
	TEXT("/Game/Blueprints/UI/Collection/WBP_CollectionWidget.WBP_CollectionWidget_C"));

const FLinearColor InactiveTabColor(0.14f, 0.18f, 0.21f, 1.0f);

FLinearColor GetCollectionTabColor(const EPBCollectionCategory Category)
{
	switch (Category)
	{
	case EPBCollectionCategory::Ball: return FLinearColor(0.88f, 0.35f, 0.35f, 1.0f);
	case EPBCollectionCategory::Synergy: return FLinearColor(0.27f, 0.70f, 0.61f, 1.0f);
	case EPBCollectionCategory::Relic: return FLinearColor(0.82f, 0.65f, 0.28f, 1.0f);
	case EPBCollectionCategory::Bumper: return FLinearColor(0.26f, 0.60f, 0.85f, 1.0f);
	case EPBCollectionCategory::Boss: return FLinearColor(0.69f, 0.36f, 0.76f, 1.0f);
	default: return InactiveTabColor;
	}
}
}

void UPBCollectionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (GetClass() == StaticClass())
	{
		return;
	}

	BindWidgetEvents();
	ValidateRequiredWidgetBindings();
}

void UPBCollectionWidget::NativeDestruct()
{
	UnbindWidgetEvents();
	Super::NativeDestruct();
}

void UPBCollectionWidget::OnPushed_Implementation()
{
	Super::OnPushed_Implementation();
	if (RedirectLegacyNativeWidget())
	{
		return;
	}

	ApplyCollectionInputMode(true);
	ShowCollectionTab(CurrentCategory);
}

void UPBCollectionWidget::OnPopped_Implementation()
{
	if (UPBCollectionTabWidgetBase* CurrentTab = ResolveTabWidget(CurrentCategory))
	{
		CurrentTab->DeactivateTab();
	}
	ApplyCollectionInputMode(false);
	Super::OnPopped_Implementation();
}

void UPBCollectionWidget::RefreshCollection()
{
	if (UPBCollectionTabWidgetBase* CurrentTab = ResolveTabWidget(CurrentCategory))
	{
		CurrentTab->RefreshTab();
	}
}

void UPBCollectionWidget::ShowCollectionTab(const EPBCollectionCategory Category)
{
	const EPBCollectionCategory ResolvedCategory =
		Category == EPBCollectionCategory::Ball
		|| Category == EPBCollectionCategory::Synergy
		|| Category == EPBCollectionCategory::Relic
		|| Category == EPBCollectionCategory::Bumper
		|| Category == EPBCollectionCategory::Boss
			? Category
			: EPBCollectionCategory::Ball;

	UPBCollectionTabWidgetBase* NewTab = ResolveTabWidget(ResolvedCategory);
	if (!TabSwitcher || !NewTab)
	{
		return;
	}

	if (UPBCollectionTabWidgetBase* PreviousTab = ResolveTabWidget(CurrentCategory))
	{
		if (PreviousTab != NewTab)
		{
			PreviousTab->DeactivateTab();
		}
	}

	CurrentCategory = ResolvedCategory;
	TabSwitcher->SetActiveWidget(NewTab);
	ApplyTabButtonStyles();
	NewTab->ActivateTab();
	BP_OnCollectionCategoryChanged(CurrentCategory);
}

void UPBCollectionWidget::BindWidgetEvents()
{
	if (BallTabButton) BallTabButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleBallTabClicked);
	if (SynergyTabButton) SynergyTabButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleSynergyTabClicked);
	if (RelicTabButton) RelicTabButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleRelicTabClicked);
	if (BumperTabButton) BumperTabButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleBumperTabClicked);
	if (BossTabButton) BossTabButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleBossTabClicked);
	if (CloseButton) CloseButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCloseClicked);
}

void UPBCollectionWidget::UnbindWidgetEvents()
{
	if (BallTabButton) BallTabButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleBallTabClicked);
	if (SynergyTabButton) SynergyTabButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleSynergyTabClicked);
	if (RelicTabButton) RelicTabButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleRelicTabClicked);
	if (BumperTabButton) BumperTabButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleBumperTabClicked);
	if (BossTabButton) BossTabButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleBossTabClicked);
	if (CloseButton) CloseButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleCloseClicked);
}

void UPBCollectionWidget::ApplyTabButtonStyles()
{
	const TArray<TPair<UButton*, EPBCollectionCategory>> TabButtons = {
		{BallTabButton, EPBCollectionCategory::Ball},
		{SynergyTabButton, EPBCollectionCategory::Synergy},
		{RelicTabButton, EPBCollectionCategory::Relic},
		{BumperTabButton, EPBCollectionCategory::Bumper},
		{BossTabButton, EPBCollectionCategory::Boss}
	};

	for (const TPair<UButton*, EPBCollectionCategory>& TabButton : TabButtons)
	{
		if (TabButton.Key)
		{
			TabButton.Key->SetBackgroundColor(
				TabButton.Value == CurrentCategory
					? GetCollectionTabColor(TabButton.Value)
					: InactiveTabColor);
		}
	}
}

bool UPBCollectionWidget::ValidateRequiredWidgetBindings() const
{
	bool bValid = true;
	const auto Check = [this, &bValid](const UObject* Widget, const TCHAR* Name)
	{
		if (!IsValid(Widget))
		{
			UE_LOG(LogTemp, Error, TEXT("%s: 필수 도감 위젯 '%s'의 이름 또는 타입이 맞지 않습니다."), *GetName(), Name);
			bValid = false;
		}
	};

	Check(TabSwitcher, TEXT("TabSwitcher"));
	Check(BallTabButton, TEXT("BallTabButton"));
	Check(SynergyTabButton, TEXT("SynergyTabButton"));
	Check(RelicTabButton, TEXT("RelicTabButton"));
	Check(BumperTabButton, TEXT("BumperTabButton"));
	Check(BossTabButton, TEXT("BossTabButton"));
	Check(CloseButton, TEXT("CloseButton"));
	Check(BallTabContent, TEXT("BallTabContent"));
	Check(SynergyTabContent, TEXT("SynergyTabContent"));
	Check(RelicTabContent, TEXT("RelicTabContent"));
	Check(BumperTabContent, TEXT("BumperTabContent"));
	Check(BossTabContent, TEXT("BossTabContent"));
	return bValid;
}

UPBCollectionTabWidgetBase* UPBCollectionWidget::ResolveTabWidget(const EPBCollectionCategory Category) const
{
	switch (Category)
	{
	case EPBCollectionCategory::Ball: return BallTabContent;
	case EPBCollectionCategory::Synergy: return SynergyTabContent;
	case EPBCollectionCategory::Relic: return RelicTabContent;
	case EPBCollectionCategory::Bumper: return BumperTabContent;
	case EPBCollectionCategory::Boss: return BossTabContent;
	default: return BallTabContent;
	}
}

bool UPBCollectionWidget::RedirectLegacyNativeWidget()
{
	if (GetClass() != StaticClass())
	{
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UPBUIManagerSubsystem* UIManager = GameInstance
		? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
		: nullptr;
	if (!IsValid(UIManager))
	{
		UE_LOG(LogTemp, Error, TEXT("%s: 도감 WBP 전환에 필요한 UIManager를 찾지 못했습니다."), *GetName());
		return true;
	}

	const TSubclassOf<UPBCollectionWidget> BlueprintClass =
		CollectionWidgetBlueprintPath.TryLoadClass<UPBCollectionWidget>();
	if (!BlueprintClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: 도감 WBP를 불러오지 못했습니다. Path=%s"), *GetName(), *CollectionWidgetBlueprintPath.ToString());
		UIManager->CompletePopWidget(this);
		return true;
	}

	if (!UIManager->CompletePopWidget(this))
	{
		UE_LOG(LogTemp, Error, TEXT("%s: C++ 도감 위젯을 UI 스택에서 제거하지 못했습니다."), *GetName());
		return true;
	}

	if (!IsValid(UIManager->PushWidget(BlueprintClass, 0)))
	{
		UE_LOG(LogTemp, Error, TEXT("%s: 도감 WBP를 UI 스택에 추가하지 못했습니다."), *GetName());
	}
	return true;
}

APlayerController* UPBCollectionWidget::ResolvePlayerController() const
{
	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		return OwningPlayer;
	}
	return GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
}

void UPBCollectionWidget::ApplyCollectionInputMode(const bool bEnableUI) const
{
	if (!bEnableUI)
	{
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			const UPBUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UPBUIManagerSubsystem>();
			if (IsValid(UIManager) && IsValid(UIManager->GetTopWidget()))
			{
				return;
			}
		}
	}

	APlayerController* PlayerController = ResolvePlayerController();
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = bEnableUI;
	if (bEnableUI)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void UPBCollectionWidget::HandleBallTabClicked() { ShowCollectionTab(EPBCollectionCategory::Ball); }
void UPBCollectionWidget::HandleSynergyTabClicked() { ShowCollectionTab(EPBCollectionCategory::Synergy); }
void UPBCollectionWidget::HandleRelicTabClicked() { ShowCollectionTab(EPBCollectionCategory::Relic); }
void UPBCollectionWidget::HandleBumperTabClicked() { ShowCollectionTab(EPBCollectionCategory::Bumper); }
void UPBCollectionWidget::HandleBossTabClicked() { ShowCollectionTab(EPBCollectionCategory::Boss); }

void UPBCollectionWidget::HandleCloseClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPBUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UPBUIManagerSubsystem>())
		{
			if (UIManager->GetTopWidget() == this && (IsPopRequested() || UIManager->RequestPopWidget()))
			{
				return;
			}
		}
	}

	CompletePop();
}
