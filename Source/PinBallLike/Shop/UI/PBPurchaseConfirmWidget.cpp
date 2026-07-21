// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPurchaseConfirmWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include  "PinBallLike/Choice/UI/PBBettingWidget.h"
#include  "PinBallLike/Struct/Shop/PBPurChaseMessaage.h"
#include  "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "Kismet/GameplayStatics.h"
void UPBPurchaseConfirmWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddUniqueDynamic(
			this,
			&UPBPurchaseConfirmWidget::OnConfirmButtonClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddUniqueDynamic(
			this,
			&UPBPurchaseConfirmWidget::OnCancelButtonClicked);
	}

	OpenAnimationFinishedEvent.BindDynamic(
		this,
		&UPBPurchaseConfirmWidget::OnOpenAnimationFinished);

	if (OpenAnim)
	{
		BindToAnimationFinished(
			OpenAnim,
			OpenAnimationFinishedEvent);
	}
}

void UPBPurchaseConfirmWidget::NativeDestruct()
{
	if (OpenAnim)
	{
		UnbindFromAnimationFinished(
			OpenAnim,
			OpenAnimationFinishedEvent);
	}

	Super::NativeDestruct();
}

void UPBPurchaseConfirmWidget::OnConfirmButtonClicked()
{
	FPBPurChaseMessaage Message;
	Message.SlotIndex = SlotIndex;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Shop_Purchase,
		Message);
	
	
}

void UPBPurchaseConfirmWidget::OnCancelButtonClicked()
{
	this->SetVisibility(ESlateVisibility::Hidden);
}

void UPBPurchaseConfirmWidget::OnOpenAnimationFinished()
{
	ConfirmButton->SetIsEnabled(true);
	CancelButton->SetIsEnabled(true);
}

void UPBPurchaseConfirmWidget::SetButtonsEnabled(bool bEnabled)
{

}

void UPBPurchaseConfirmWidget::SetInfo(int32 Index, FText Name, int32 Price, FText Synergy, UTexture2D* Icon)
{
	SlotIndex = Index;
	ItemNameText->SetText(Name);
	
	PriceText->SetText(
	FText::Format(
		FText::FromString(TEXT("아이템 가격 : {0} Gold")),
		FText::AsNumber(Price)
	));
	
		
	SynergyText->SetText(Synergy);
	
	PlayOpenAnimation();
	
}
void UPBPurchaseConfirmWidget::PlayOpenAnimation()
{
	SetButtonsEnabled(false);

	if (!OpenAnim)
	{
		SetButtonsEnabled(true);
		return;
	}

	PlayAnimation(
		OpenAnim,
		0.f,
		1,
		EUMGSequencePlayMode::Forward,
		1.f);
}
