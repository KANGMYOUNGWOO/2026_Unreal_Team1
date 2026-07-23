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
#include "PinBallLike/Actor/Ball/UI/PBBallDetailInfoRowWidget.h"
#include "PinBallLike/Actor/Ball/UI/PBBallDetailTagListRowWidget.h"
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

    //------------------------------------
    // 이름
    //------------------------------------

    if (ItemNameText)
    {
        ItemNameText->SetText(Data.BallNameText);
    }

    //------------------------------------
    // 가격
    //------------------------------------

    if (PriceText)
    {
        PriceText->SetText(FText::Format(
            NSLOCTEXT("Shop", "Price", "{0} 골드"),
            FText::AsNumber(Data.Price)));
    }

    //------------------------------------
    // 아이콘
    //------------------------------------

    if (ItemIcon)
    {
        ItemIcon->SetBrushFromTexture(Data.BallIconTexture);
    }

    //------------------------------------
    // 능력치
    //------------------------------------

    if (HPText)
    {
        HPText->SetText(Data.HpRow.ValueText);
    }

    if (MPText)
    {
        MPText->SetText(Data.MpRow.ValueText);
    }

    if (AttackText)
    {
        AttackText->SetText(Data.AttackRow.ValueText);
    }

    if (ManaRegenText)
    {
        ManaRegenText->SetText(Data.ManaRegenRow.ValueText);
    }

	if (Tag_Type)
	{
		Tag_Type->SetIconTextRow(NSLOCTEXT("PurchaseConfirm", "TypeLabel", "타입"), Data.PowerFlipData);
	}

	if (Tag_Job)
	{
		Tag_Job->SetIconTextRow(NSLOCTEXT("PurchaseConfirm", "JobLabel", "직업"), Data.ClassData);
	}

	if (TagListRow_Race)
	{
		TagListRow_Race->SetTagList(NSLOCTEXT("PurchaseConfirm", "RaceLabel", "종족"), Data.RaceDataList);
	}

	//------------------------------------
	// 태그
	//------------------------------------
	
	
	
    if (SkillNameText)
    {
        SkillNameText->SetText(Data.SkillNameText);
    }
	
	if (SkillExplainText)
	{
		SkillExplainText->SetText(Data.SkillDescriptionText);
	}

    if (SkillIcon)
    {
        SkillIcon->SetBrushFromTexture(Data.SkillIconTexture);
    }
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
