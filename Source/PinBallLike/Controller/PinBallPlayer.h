// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallPlayer.generated.h"

struct FInputActionValue;
struct FPBBallDamagedMessage;
class APBCombatPartyController;
class AFlipper;
class UCameraShakeBase;
class UInputComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class PINBALLLIKE_API APinBallPlayer : public APawn
{
	GENERATED_BODY()

public:
	APinBallPlayer();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void UnPossessed() override;

protected:
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Input")
	TObjectPtr<UInputAction> DashAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Input")
	TArray<TObjectPtr<AFlipper>> Flippers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinBall|Launch")
	TObjectPtr<APBCombatPartyController> CombatPartyActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Damage Feedback")
	TSubclassOf<UCameraShakeBase> BallDamageCameraShakeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Damage Feedback",
		meta = (ClampMin = "1"))
	int32 DamageForMaximumShake = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Damage Feedback",
		meta = (ClampMin = "0.0"))
	float MinimumDamageShakeScale = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Damage Feedback",
		meta = (ClampMin = "0.0"))
	float MaximumDamageShakeScale = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PinBall|Damage Feedback",
		meta = (ClampMin = "0.0", Units = "s"))
	float MinimumDamageShakeInterval = 0.08f;
	
private:
	void RegisterMessageListeners();
	void UnregisterMessageListeners();
	void HandleBallDamagedMessage(FGameplayTag Channel, const FPBBallDamagedMessage& Message);
	void AddInputMappingContext();
	void RemoveInputMappingContext();
	void UpFlippers(const FInputActionValue& Value);
	void DownFlippers(const FInputActionValue& Value);
	void LaunchParty(const FInputActionValue& Value);
	void RequestShiftDeploymentSlots(const FInputActionValue& Value);
	void SetFlippersRaised(bool bRaised) const;
	void RequestUseSkill(const FInputActionValue& Value);
	void RequestDash(const FInputActionValue& Value);

	FGameplayMessageListenerHandle BallDamagedListenerHandle;
	double LastBallDamageShakeTime = -1.0;
};
