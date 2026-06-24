// Fill out your copyright notice in the Description page of Project Settings.


#include "PinBallController.h"

#include "Blueprint/UserWidget.h"
#include "EngineUtils.h"
#include "PinBallPlayer.h"
#include "PinBallLike/Actor/Boss/PBBossBase.h"
#include "PinBallLike/Actor/Boss/UI/PBBossStatusWidget.h"
#include "UObject/ConstructorHelpers.h"

APinBallController::APinBallController()
{
	static ConstructorHelpers::FClassFinder<UPBBossStatusWidget> BossStatusWidgetFinder(
		TEXT("/Game/Blueprints/Boss/UI/WBP_BossStatus"));

	if (BossStatusWidgetFinder.Succeeded())
	{
		BossStatusWidgetClass = BossStatusWidgetFinder.Class;
	}
}

void APinBallController::BeginPlay()
{
	Super::BeginPlay();

	CreateBossStatusWidget();
}

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

void APinBallController::CreateBossStatusWidget()
{
	if (!IsLocalController() || !BossStatusWidgetClass)
	{
		return;
	}

	BossStatusWidget = CreateWidget<UPBBossStatusWidget>(this, BossStatusWidgetClass);
	if (!BossStatusWidget)
	{
		return;
	}

	BossStatusWidget->AddToViewport();
	BossStatusWidget->SetBoss(FindBossForStatusWidget());
}

APBBossBase* APinBallController::FindBossForStatusWidget() const
{
	if (BossForStatusWidget)
	{
		return BossForStatusWidget;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<APBBossBase> BossIterator(World); BossIterator; ++BossIterator)
	{
		return *BossIterator;
	}

	return nullptr;
}
