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

void UPBPurchaseConfirmWidget::SetInfo(
	const FPBPurchaseConfirmData& Data)
{
	SlotIndex = Data.SlotIndex;

	//--------------------------------------------------
	// 이름
	//--------------------------------------------------

	if (ItemNameText)
	{
		ItemNameText->SetText(Data.BallName);
	}

	//--------------------------------------------------
	// 가격
	//--------------------------------------------------

	if (PriceText)
	{
		PriceText->SetText(
			FText::Format(
				FText::FromString(TEXT("아이템 가격 : {0} Gold")),
				FText::AsNumber(Data.Price)));
	}

	//--------------------------------------------------
	// 시너지
	//--------------------------------------------------

	
	//--------------------------------------------------
	// 아이콘
	//--------------------------------------------------

	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(Data.BallIcon);
	}

	//--------------------------------------------------
	// HP
	//--------------------------------------------------

	if (HPText)
	{
		HPText->SetText(FText::AsNumber(Data.HP));
	}

	//--------------------------------------------------
	// MP
	//--------------------------------------------------

	if (MPText)
	{
		MPText->SetText(FText::AsNumber(Data.MP));
	}

	//--------------------------------------------------
	// Attack
	//--------------------------------------------------

	if (AttackText)
	{
		AttackText->SetText(FText::AsNumber(Data.Attack));
	}

	//--------------------------------------------------
	// Mana Regen
	//--------------------------------------------------

	if (ManaRegenText)
	{
		ManaRegenText->SetText(
			FText::AsNumber(Data.ManaRegen));
	}

	//--------------------------------------------------
	// 애니메이션
	//--------------------------------------------------

	
	SynergyIcon1->SetVisibility(ESlateVisibility::Hidden);
	SynergyIcon2->SetVisibility(ESlateVisibility::Hidden);
	SynergyIcon3->SetVisibility(ESlateVisibility::Hidden);
	SynergyText1->SetVisibility(ESlateVisibility::Hidden);
	SynergyText2->SetVisibility(ESlateVisibility::Hidden);
	SynergyText3->SetVisibility(ESlateVisibility::Hidden);
	if(Data.Synergies.Num() > 0)
	{
		SynergyIcon1->SetBrushFromTexture(
			Data.Synergies[0].Icon);
		SynergyIcon1->SetVisibility(ESlateVisibility::Visible);
		SynergyText1->SetText(
	 Data.Synergies[0].SynergyName);
		
		SynergyText1->SetVisibility(ESlateVisibility::Visible);
	}

	if(Data.Synergies.Num() > 1)
	{
		SynergyIcon2->SetBrushFromTexture(
			Data.Synergies[1].Icon);
		SynergyIcon2->SetVisibility(ESlateVisibility::Visible);
		SynergyText2->SetText(
	  Data.Synergies[1].SynergyName);
		SynergyText2->SetVisibility(ESlateVisibility::Visible);
	}
	
	if(Data.Synergies.Num() > 2)
	{
		SynergyIcon3->SetBrushFromTexture(
			Data.Synergies[2].Icon);
		SynergyIcon3->SetVisibility(ESlateVisibility::Visible);
		SynergyText3->SetText(
	  Data.Synergies[2].SynergyName);
		SynergyText3->SetVisibility(ESlateVisibility::Visible);
	}
	
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
