// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/GameBootstrapSubsystem.h"
#include "SubSystem/DataSubsystem.h"


void UGameBootstrapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameBootstrapSubsystem::StartBootstrap()
{
	UDataSubsystem* DataSubsystem = 
	  GetGameInstance()-> GetSubsystem<UDataSubsystem>();
}
