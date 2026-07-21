// Fill out your copyright notice in the Description page of Project Settings.


#include "PBShopRefuseWidget.h"
#include "Animation/WidgetAnimation.h"

void UPBShopRefuseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefuseAnimationFinishedEvent.BindDynamic(
		this,
		&UPBShopRefuseWidget::OnRefuseAnimationFinished);

	if (RefuseAnim)
	{
		BindToAnimationFinished(
			RefuseAnim,
			RefuseAnimationFinishedEvent);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UPBShopRefuseWidget::NativeDestruct()
{
	if (RefuseAnim)
	{
		UnbindFromAnimationFinished(
			RefuseAnim,
			RefuseAnimationFinishedEvent);
	}

	Super::NativeDestruct();
}

void UPBShopRefuseWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);

	if (RefuseAnim)
	{
		StopAnimation(RefuseAnim);
		PlayAnimation(RefuseAnim);
	}
}

void UPBShopRefuseWidget::OnRefuseAnimationFinished()
{
	SetVisibility(ESlateVisibility::Collapsed);
}