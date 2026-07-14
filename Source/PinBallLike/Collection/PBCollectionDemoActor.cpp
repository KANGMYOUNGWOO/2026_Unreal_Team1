#include "PBCollectionDemoActor.h"

#include "PinBallLike/Collection/PBCollectionWidget.h"
#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"
#include "PinBallLike/UI/PBUserWidget.h"

#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

APBCollectionDemoActor::APBCollectionDemoActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APBCollectionDemoActor::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (bBindToggleKey && PlayerController)
	{
		EnableInput(PlayerController);
		if (InputComponent)
		{
			InputComponent->BindKey(ToggleKey, IE_Pressed, this, &APBCollectionDemoActor::ToggleCollection);
		}
	}

	if (bOpenOnBeginPlay)
	{
		OpenCollection();
	}
}

void APBCollectionDemoActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 레벨 종료 중에는 닫기 애니메이션을 기다리지 않고 스택에서 즉시 정리합니다.
	if (CollectionWidget && CollectionWidget->IsInViewport())
	{
		CollectionWidget->CompletePop();
	}
	CollectionWidget = nullptr;

	Super::EndPlay(EndPlayReason);
}

void APBCollectionDemoActor::OpenCollection()
{
	if (CollectionWidget && CollectionWidget->IsInViewport())
	{
		return;
	}
	CollectionWidget = nullptr;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}
	if (!CollectionWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s: CollectionWidgetClass에 WBP_CollectionWidget 계열 클래스를 지정해야 합니다."),
			*GetName());
		return;
	}

	UPBUIManagerSubsystem* UIManagerSubsystem = GameInstance->GetSubsystem<UPBUIManagerSubsystem>();
	if (!IsValid(UIManagerSubsystem))
	{
		return;
	}

	CollectionWidget = UIManagerSubsystem->PushWidget(CollectionWidgetClass, ViewportZOrder);
}

void APBCollectionDemoActor::CloseCollection()
{
	if (!CollectionWidget || !CollectionWidget->IsInViewport())
	{
		CollectionWidget = nullptr;
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UPBUIManagerSubsystem* UIManagerSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPBUIManagerSubsystem>()
		: nullptr;
	if (IsValid(UIManagerSubsystem) && UIManagerSubsystem->GetTopWidget() == CollectionWidget)
	{
		if (!CollectionWidget->IsPopRequested())
		{
			UIManagerSubsystem->RequestPopWidget();
		}
		return;
	}

	// 스택 상태가 예상과 다를 때도 UIManager가 제거 가능 여부를 최종 판단하도록 요청합니다.
	if (CollectionWidget->CompletePop())
	{
		CollectionWidget = nullptr;
	}
}

void APBCollectionDemoActor::ToggleCollection()
{
	if (CollectionWidget && CollectionWidget->IsInViewport())
	{
		CloseCollection();
		return;
	}

	OpenCollection();
}
