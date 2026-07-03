// Fill out your copyright notice in the Description page of Project Settings.


#include "PBChoiceWidget.h"
#include "Components/Button.h"
#include "PinBallLike/Choice/PBChoiceRouteActor.h"


void UPBChoiceWidget::SetChoiceRouteActor(APBChoiceRouteActor* choiceRouteActor)
{
	this->ChoiceRouteActor = choiceRouteActor;
}

void UPBChoiceWidget::NativeConstruct()
{
	Super::NativeConstruct();
    IsSelected = false;
	if (LeftButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("Left"));
		LeftButton->OnClicked.AddDynamic(
			this,
			&UPBChoiceWidget::HandleLeftButtonClicked);
	}

	if (RightButton)
	{
		RightButton->OnClicked.AddDynamic(
			this,
			&UPBChoiceWidget::HandleRightButtonClicked);
	}
}

void UPBChoiceWidget::HandleLeftButtonClicked()
{
	this->SetVisibility(ESlateVisibility::Collapsed);
	
	if (IsSelected) {return;}
	if (ChoiceRouteActor)
	{
		
		IsSelected = true;
		ChoiceRouteActor->ChooseLeft();
	}
}

void UPBChoiceWidget::HandleRightButtonClicked()
{	
	this->SetVisibility(ESlateVisibility::Collapsed);
	
	if (IsSelected) {return;}
	if (ChoiceRouteActor)
	{
		IsSelected = true;
		ChoiceRouteActor->ChooseRight();
	}
}
