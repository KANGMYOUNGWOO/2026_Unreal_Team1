// Fill out your copyright notice in the Description page of Project Settings.


#include "PinBallController.h"
#include "PinBallPlayer.h"

void APinBallController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PinBallPlayer = Cast<APinBallPlayer>(InPawn);
	if (!PinBallPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("PinBallController can only control APinBallPlayer. Possessed: %s"),
			*GetNameSafe(InPawn));
	}
}

void APinBallController::OnUnPossess()
{
	PinBallPlayer = nullptr;
	Super::OnUnPossess();
}
