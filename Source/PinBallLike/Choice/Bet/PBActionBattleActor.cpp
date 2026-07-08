// Fill out your copyright notice in the Description page of Project Settings.


#include "PBActionBattleActor.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
APBActionBattleActor::APBActionBattleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void APBActionBattleActor::OpenAbility()
{
	
	UGameplayStatics::OpenLevel(this, FName(TEXT("Lv_Battle")));
}

// Called when the game starts or when spawned
void APBActionBattleActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APBActionBattleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

