#include "PBBettingWidget.h"

#include "Components/Button.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"

void UPBBettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnExitButtonClicked);
	}

	if (NationButton1)
	{
		NationButton1->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnNationButton1Clicked);
	}

	if (NationButton2)
	{
		NationButton2->OnClicked.AddUniqueDynamic(
			this,
			&UPBBettingWidget::OnNationButton2Clicked);
	}
}

void UPBBettingWidget::OnNationButton1Clicked()
{
	OnBetSelected.Broadcast(0);
}

void UPBBettingWidget::OnNationButton2Clicked()
{
	OnBetSelected.Broadcast(1);
}

void UPBBettingWidget::OnExitButtonClicked()
{
	FPBChoiceType Message;
	Message.Exit = 1;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Choice_Exit,
		Message);
}