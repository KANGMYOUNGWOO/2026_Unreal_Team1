// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PinBallPlayer.generated.h"

struct FInputActionValue;
class APBCombatPartyController;
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
	UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
	void Debug_OnSkillInputReceived(int32 SkillInputValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputAction> FlipperAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputAction> LaunchAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputAction> ShiftAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputAction> SkillAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Input")
	TArray<TObjectPtr<AFlipper>> Flippers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Launch")
	TObjectPtr<APBCombatPartyController> CombatPartyActor;
	
private:
	void AddInputMappingContext();
	void RemoveInputMappingContext();
	void UpFlippers(const FInputActionValue& Value);
	void DownFlippers(const FInputActionValue& Value);
	void LaunchParty(const FInputActionValue& Value);
	void RequestShiftDeploymentSlots(const FInputActionValue& Value);
	void SetFlippersRaised(bool bRaised) const;
	void RequestUseSkill(const FInputActionValue& Value);

};
