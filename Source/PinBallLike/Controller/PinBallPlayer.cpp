// Fill out your copyright notice in the Description page of Project Settings.


#include "PinBallPlayer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "PinBallLike/Actor/Flipper/Flipper.h"

APinBallPlayer::APinBallPlayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APinBallPlayer::PawnClientRestart()
{
	Super::PawnClientRestart();
	AddInputMappingContext();
}

void APinBallPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("PinBallPlayer requires an Enhanced Input Component."));
		return;
	}

	if (!FlipperAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("FlipperAction is not assigned on %s."), *GetName());
		return;
	}

	EnhancedInputComponent->BindAction(
		FlipperAction, ETriggerEvent::Started, this, &APinBallPlayer::UpFlippers);
	EnhancedInputComponent->BindAction(
		FlipperAction, ETriggerEvent::Completed, this, &APinBallPlayer::DownFlippers);
	EnhancedInputComponent->BindAction(
		FlipperAction, ETriggerEvent::Canceled, this, &APinBallPlayer::DownFlippers);
}

void APinBallPlayer::UnPossessed()
{
	SetFlippersRaised(false);
	RemoveInputMappingContext();
	Super::UnPossessed();
}

void APinBallPlayer::AddInputMappingContext()
{
	if (!InputMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputMappingContext is not assigned on %s."), *GetName());
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(InputMappingContext);
		InputSubsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void APinBallPlayer::RemoveInputMappingContext()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !InputMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(InputMappingContext);
	}
}

void APinBallPlayer::UpFlippers(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("raise"));
	SetFlippersRaised(true);
}

void APinBallPlayer::DownFlippers(const FInputActionValue& Value)
{
	SetFlippersRaised(false);
}

void APinBallPlayer::SetFlippersRaised(const bool bRaised) const
{
	for (AFlipper* Flipper : Flippers)
	{
		if (IsValid(Flipper))
		{
			Flipper->SetIsMove(bRaised);
		}
	}
}
