// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBetActor.h"
#include "Kismet/GameplayStatics.h"
#include  "PinBallLike/Choice/UI/PBBettingWidget.h"
#include  "PinBallLike/GamePlayTag/GamePlayTags.h"

void APBBetActor::OpenAbility()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;	
	}
	
	if (!BetWidget)
	{
		if (!BetWidgetClass)
		{
			return;
		}

		BetWidget = CreateWidget<UPBBettingWidget>(PC,BetWidgetClass);
		if (!BetWidget)
		{
			return;
		}
		
		BetWidget->AddToViewport();
	}
	
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeGameAndUI());
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	
}

void APBBetActor::HandleExitStart(FGameplayTag Exit, const FPBChoiceType& Message)
{
	
	if (Message.Exit == 1)
	{
		if (BetWidget)
		{
			BetWidget->RemoveFromViewport();
			BetWidget = nullptr;
		}
		
		APlayerController* PC = UGameplayStatics::GetPlayerController(this,0);
		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
}

void APBBetActor::BindWidget(UPBBettingWidget* InWidget)
{
	if (!IsValid(InWidget))
	{
		return;
	}

	BettingWidget = InWidget;

	BettingWidget->OnBetSelected.AddUniqueDynamic(
		this,
		&APBBetActor::HandleBetSelected);
	

}

void APBBetActor::HandleBetSelected(int32 SelectedIndex)
{
	const FPBBettingResult Result = ResolveBet(SelectedIndex);

	FinishBet(Result);
}

FPBBettingResult APBBetActor::ResolveBet(int32 SelectedIndex)
{
	FPBBettingResult Result;

	Result.SelectedIndex = SelectedIndex;

	// 현재는 단순 50 : 50
	Result.WinnerIndex = FMath::RandRange(0, 1);

	Result.bWin =
		Result.SelectedIndex == Result.WinnerIndex;

	return Result;
}

void APBBetActor::FinishBet(const FPBBettingResult& Result)
{
	UE_LOG(
	   LogTemp,
	   Warning,
	   TEXT(
		   "Bet Result | Selected: %d | Winner: %d | Win: %s"),
	   Result.SelectedIndex,
	   Result.WinnerIndex,
	   Result.bWin ? TEXT("True") : TEXT("False"));

	FPBChoiceType Message;
	Message.Exit = 1;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Choice_Exit,
		Message);
}

void APBBetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Sets default values
APBBetActor::APBBetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APBBetActor::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayMessageSubsystem& MessageSubsystem =
	UGameplayMessageSubsystem::Get(this);
	
	ExitStartHandle =
		MessageSubsystem.RegisterListener<FPBChoiceType>(
			GameplayTags::Event_UI_Choice_Exit,
			this,
			&APBBetActor::HandleExitStart);
}

// Called every frame
