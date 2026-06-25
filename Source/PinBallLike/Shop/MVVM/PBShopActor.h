// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBShopActor.generated.h"

class UPBShopManager;
class UPBShopViewModel;
class UPBSHopWidget;
class APBShopDisplayActor;

UCLASS()
class PINBALLLIKE_API APBShopActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APBShopActor();
	
	UFUNCTION(BlueprintCallable)
	void OpenShop();
	
	UFUNCTION(BlueprintCallable)
	void ClostShop();
	
private:
	void RefreshShopView();

private:
	UPROPERTY(EditAnywhere, Category = "Shop")
	TObjectPtr<APBShopDisplayActor> ShopDisplayActor;
	
	UPROPERTY(EditAnywhere, Category = "Shop")
	TSubclassOf<UPBShopWidget> ShopWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UPBShopManager> ShopManager;
	
	UPROPERTY()
	TObjectPtr<UPBShopViewModel> ShopViewModel;
	
	UPROPERTY()
	TObjectPtr<UPBShopWidget> ShopWidget;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
