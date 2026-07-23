// Fill out your copyright notice in the Description page of Project Settings.


#include "PinBallPlayer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Flipper/Flipper.h"
#include "PinBallLike/Controller/Component/PBCombatCameraTrackingComponent.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/Battle/PBBallDamagedMessage.h"
#include "PinBallLike/Struct/Battle/PBBallSkillActivatedMessage.h"
#include "PinBallLike/Struct/Battle/PBBattlePhaseMessage.h"

APinBallPlayer::APinBallPlayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APinBallPlayer::BeginPlay()
{
	Super::BeginPlay();
	RegisterMessageListeners();
}

void APinBallPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterMessageListeners();
	Super::EndPlay(EndPlayReason);
}

void APinBallPlayer::PawnClientRestart()
{
	Super::PawnClientRestart();
	AddInputMappingContext();
}

void APinBallPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("PinBallPlayer requires an Enhanced Input Component."));
		return;
	}

	if (FlipperAction)
	{
		EnhancedInputComponent->BindAction(
			FlipperAction, ETriggerEvent::Started, this, &APinBallPlayer::UpFlippers);
		EnhancedInputComponent->BindAction(
			FlipperAction, ETriggerEvent::Completed, this, &APinBallPlayer::DownFlippers);
		EnhancedInputComponent->BindAction(
			FlipperAction, ETriggerEvent::Canceled, this, &APinBallPlayer::DownFlippers);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FlipperAction is not assigned on %s."), *GetName());
	}

	if (LaunchAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("PinBallPlayer bound LaunchAction %s on %s."),
			*GetNameSafe(LaunchAction.Get()),
			*GetNameSafe(this));
		EnhancedInputComponent->BindAction(
			LaunchAction, ETriggerEvent::Started, this, &APinBallPlayer::LaunchParty);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LaunchAction is not assigned on %s."), *GetName());
	}

	if (ShiftAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("PinBallPlayer bound ShiftAction %s on %s."),
			*GetNameSafe(ShiftAction.Get()),
			*GetNameSafe(this));
		EnhancedInputComponent->BindAction(
			ShiftAction, ETriggerEvent::Started, this, &APinBallPlayer::RequestShiftDeploymentSlots);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShiftAction is not assigned on %s."), *GetName());
	}

	if (SkillAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("PinBallPlayer bound SkillAction %s on %s."),
			*GetNameSafe(SkillAction.Get()),
			*GetNameSafe(this));
		EnhancedInputComponent->BindAction(
			SkillAction, ETriggerEvent::Started, this, &APinBallPlayer::RequestUseSkill);
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillAction is not assigned on %s."), *GetName());
	}

	if (DashAction)
	{
		EnhancedInputComponent->BindAction(
			DashAction, ETriggerEvent::Started, this, &APinBallPlayer::RequestDash);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DashAction is not assigned on %s."), *GetName());
	}
}

void APinBallPlayer::UnPossessed()
{
	SetFlippersRaised(false);
	RemoveInputMappingContext();
	Super::UnPossessed();
}

void APinBallPlayer::RegisterMessageListeners()
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	BallDamagedListenerHandle =
		UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBallDamagedMessage>(
			GameplayTags::Event_Battle_Ball_Damaged,
			this,
			&APinBallPlayer::HandleBallDamagedMessage);
	BallSkillActivatedListenerHandle =
		UGameplayMessageSubsystem::Get(this).RegisterListener<FPBBallSkillActivatedMessage>(
			GameplayTags::Event_Battle_Skill_Activated,
			this,
			&APinBallPlayer::HandleBallSkillActivatedMessage);
}

void APinBallPlayer::UnregisterMessageListeners()
{
	if (BallDamagedListenerHandle.IsValid())
	{
		BallDamagedListenerHandle.Unregister();
		BallDamagedListenerHandle = FGameplayMessageListenerHandle();
	}
	if (BallSkillActivatedListenerHandle.IsValid())
	{
		BallSkillActivatedListenerHandle.Unregister();
		BallSkillActivatedListenerHandle = FGameplayMessageListenerHandle();
	}
}

void APinBallPlayer::HandleBallDamagedMessage(
	FGameplayTag Channel,
	const FPBBallDamagedMessage& Message)
{
	(void)Channel;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	UWorld* World = GetWorld();
	if (!BallDamageCameraShakeClass
		|| Message.AppliedDamage <= 0
		|| !PlayerController
		|| !PlayerController->IsLocalController()
		|| !World)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	if (LastBallDamageShakeTime >= 0.0
		&& CurrentTime - LastBallDamageShakeTime < FMath::Max(MinimumDamageShakeInterval, 0.0f))
	{
		return;
	}

	const int32 MaximumDamage = FMath::Max(DamageForMaximumShake, 1);
	const float DamageAlpha = MaximumDamage <= 1
		? 1.0f
		: FMath::GetRangePct(
			1.0f,
			static_cast<float>(MaximumDamage),
			static_cast<float>(FMath::Clamp(Message.AppliedDamage, 1, MaximumDamage)));
	const float MinimumScale = FMath::Max(MinimumDamageShakeScale, 0.0f);
	const float MaximumScale = FMath::Max(MaximumDamageShakeScale, MinimumScale);
	const float ShakeScale = FMath::Lerp(MinimumScale, MaximumScale, DamageAlpha);
	if (ShakeScale <= 0.0f)
	{
		return;
	}

	PlayerController->ClientStartCameraShake(BallDamageCameraShakeClass, ShakeScale);
	LastBallDamageShakeTime = CurrentTime;

	UE_LOG(LogTemp, Log,
		TEXT("[BallDamageCameraShake] Played. Damage=%d Scale=%.2f ShakeClass=%s"),
		Message.AppliedDamage,
		ShakeScale,
		*GetNameSafe(BallDamageCameraShakeClass.Get()));
}

void APinBallPlayer::HandleBallSkillActivatedMessage(
	FGameplayTag Channel,
	const FPBBallSkillActivatedMessage& Message)
{
	(void)Channel;

	APBBallBase* SkillOwnerBall = Cast<APBBallBase>(Message.SkillOwnerBall);
	UPBCombatCameraTrackingComponent* CameraTrackingComponent =
		FindComponentByClass<UPBCombatCameraTrackingComponent>();
	if (!IsValid(SkillOwnerBall) || !CameraTrackingComponent)
	{
		return;
	}

	if (CameraTrackingComponent->PlaySkillFocus(SkillOwnerBall))
	{
		UE_LOG(LogTemp, Log,
			TEXT("[BallSkillCameraFocus] Started. Ball=%s"),
			*GetNameSafe(SkillOwnerBall));
	}
}

void APinBallPlayer::AddInputMappingContext()
{
	if (!InputMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputMappingContext is not assigned on %s."), *GetName());
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(InputMappingContext);
		InputSubsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void APinBallPlayer::RemoveInputMappingContext()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !InputMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		InputSubsystem->RemoveMappingContext(InputMappingContext);
	}
}

void APinBallPlayer::UpFlippers(const FInputActionValue& Value)
{
	SetFlippersRaised(true);
}

void APinBallPlayer::DownFlippers(const FInputActionValue& Value)
{
	SetFlippersRaised(false);
}

void APinBallPlayer::LaunchParty(const FInputActionValue& Value)
{
	// UE_LOG(LogTemp, Warning, TEXT("PinBallPlayer received LaunchParty input. CombatPartyActor=%s"),
	// 	*GetNameSafe(CombatPartyActor.Get()));

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePartyLaunchRequestedMessage Message;
	Message.Requester = this;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Party_Launch_Requested,
		Message);
}

void APinBallPlayer::RequestShiftDeploymentSlots(const FInputActionValue& Value)
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattlePartyShiftRequestedMessage Message;
	Message.Requester = this;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Party_Shift_Requested,
		Message);
}

void APinBallPlayer::SetFlippersRaised(const bool bRaised) const
{
	for (AFlipper* Flipper : Flippers)
	{
		if (IsValid(Flipper))
		{
			Flipper->SetIsMove(bRaised);
		}
	}
}

void APinBallPlayer::RequestUseSkill(const FInputActionValue& Value)
{
	const int32 SkillInputValue = FMath::RoundToInt(Value.Get<float>());

	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattleSkillUseRequestedMessage Message;
	Message.Requester = this;
	Message.SkillInputValue = SkillInputValue;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Skill_Use_Requested,
		Message);
}

void APinBallPlayer::RequestDash(const FInputActionValue& Value)
{
	if (!UGameplayMessageSubsystem::HasInstance(this))
	{
		return;
	}

	FPBBattleDashRequestedMessage Message;
	Message.Requester = this;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_Battle_Dash_Requested,
		Message);
}
