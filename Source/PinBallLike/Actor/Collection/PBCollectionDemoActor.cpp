#include "PBCollectionDemoActor.h"

#include "PinBallLike/UI/Collection/PBCollectionWidget.h"

#include "Blueprint/UserWidget.h"
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

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController || !CollectionWidgetClass)
	{
		return;
	}

	CollectionWidget = CreateWidget<UPBCollectionWidget>(PlayerController, CollectionWidgetClass);
	if (!CollectionWidget)
	{
		return;
	}

	CollectionWidget->AddToViewport(ViewportZOrder);
	ApplyUIInputMode(true);
}

void APBCollectionDemoActor::CloseCollection()
{
	if (CollectionWidget && CollectionWidget->IsInViewport())
	{
		CollectionWidget->RemoveFromParent();
	}
	CollectionWidget = nullptr;

	ApplyUIInputMode(false);
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

void APBCollectionDemoActor::ApplyUIInputMode(bool bEnableUI)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
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
