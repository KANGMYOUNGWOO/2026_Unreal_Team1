// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PinBallPlayer.generated.h"

struct FInputActionValue;
class AFlipper;
class UInputComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class PINBALLLIKE_API APinBallPlayer : public APawn
{
	GENERATED_BODY()

public:
	APinBallPlayer();

	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void UnPossessed() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputAction> FlipperAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Input")
	TArray<TObjectPtr<AFlipper>> Flippers;
	
private:
	void AddInputMappingContext();
	void RemoveInputMappingContext();
	void UpFlippers(const FInputActionValue& Value);
	void DownFlippers(const FInputActionValue& Value);
	void SetFlippersRaised(bool bRaised) const;

};
