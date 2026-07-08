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

	if (!CollectionWidgetClass)
	{
		CollectionWidgetClass = UPBCollectionWidget::StaticClass();
	}

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
	CloseCollection();
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
	if (!GameInstance || !CollectionWidgetClass)
	{
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
	if (CollectionWidget && CollectionWidget->IsInViewport())
	{
		CollectionWidget->CompletePop();
	}
	CollectionWidget = nullptr;
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
