// Fill out your copyright notice in the Description page of Project Settings.


#include "PBBettingWidget.h"
#include "Components//Button.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Choice/PBChoiceRouteActor.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"


void UPBBettingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UPBBettingWidget::OnExitButtonClicked);
	}
}

void UPBBettingWidget::OnExitButtonClicked()
{
	FPBChoiceType Message;
	Message.Exit = 1;
	
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Choice_Exit,Message);
}
