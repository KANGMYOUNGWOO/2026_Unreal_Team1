// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PinBallController.generated.h"

class APBBossBase;
class APinBallPlayer;
class UPBBossStatusWidget;

UCLASS()
class PINBALLLIKE_API APinBallController : public APlayerController
{
	GENERATED_BODY()

public:
	APinBallController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|UI")
	TSubclassOf<UPBBossStatusWidget> BossStatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|UI")
	TObjectPtr<APBBossBase> BossForStatusWidget;
	
private:
	void CreateBossStatusWidget();
	APBBossBase* FindBossForStatusWidget() const;

	UPROPERTY(Transient)
	TObjectPtr<APinBallPlayer> PinBallPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossStatusWidget> BossStatusWidget;
};
