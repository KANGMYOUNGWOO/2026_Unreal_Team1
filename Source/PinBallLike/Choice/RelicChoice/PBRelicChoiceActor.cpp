#include "PBRelicChoiceActor.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"
#include "PinBallLike/Relic/UI/PBRelicChoiceWidget.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Subsystem/Relic/PBRelicSubsystem.h"
#include "PinBallLike/GamePlayTag/GamePlayTags.h"
APBRelicChoiceActor::APBRelicChoiceActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APBRelicChoiceActor::BeginPlay()
{
	Super::BeginPlay();
}

void APBRelicChoiceActor::OpenRelicChoice()
{
	UGameInstance* GameInstance = GetGameInstance();
	
	if (!GameInstance)
	{
		UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RelicChoiceNode] GameInstace is invalid."));
		return;
	}

	UPBRelicSubsystem* RelicSubsystem =
	GameInstance->GetSubsystem<UPBRelicSubsystem>();

	if (!RelicSubsystem)
	{
		UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RelicChoiceNode] RelicSubSystem is invalid."));
		return;
	}

	CurrentRelicChoices.Reset();

	if (!RelicSubsystem->GetRandomRelicIds(
		3,
		CurrentRelicChoices))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicChoice] Failed to get random relic ids."));

		return;
	}
	

	APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(
			this,
			0);

	if (!PlayerController ||
		!RelicChoiceWidgetClass)
	{
		UE_LOG(
	LogTemp,
	Warning,
	TEXT("[RelicChoiceNode] PlayerController is invalid."));
		return;
	}

	GameInstance->GetSubsystem<UPBRelicSubsystem>();

	if (!RelicSubsystem)
	{
		return;
	}

	CurrentRelicChoices.Reset();

	if (!RelicSubsystem->GetRandomRelicIds(
		3,
		CurrentRelicChoices))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicChoice] Failed to get random relic ids."));

		return;
	}
	
	if (!RelicChoiceWidget)
	{
		RelicChoiceWidget =
			CreateWidget<UPBRelicChoiceWidget>(
				PlayerController,
				RelicChoiceWidgetClass);

		if (!RelicChoiceWidget)
		{
			return;
		}

		RelicChoiceWidget->OnRelicSelected.BindUObject(
			this,
			&APBRelicChoiceActor::HandleRelicSelected);
	}

	RelicChoiceWidget->SetRelicChoices(
		CurrentRelicChoices);

	RelicChoiceWidget->AddToViewport();

	PlayerController->bShowMouseCursor = true;
	PlayerController->SetInputMode(
		FInputModeGameAndUI());
}

void APBRelicChoiceActor::OpenAbility()
{
	OpenRelicChoice();
}

void APBRelicChoiceActor::HandleRelicSelected(
	const FName RelicId)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPBRelicSubsystem* RelicSubsystem =
		GameInstance->GetSubsystem<UPBRelicSubsystem>();

	if (!RelicSubsystem)
	{
		return;
	}

	if (!RelicSubsystem->AcquireRelic(RelicId))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[RelicChoice] Acquire failed. RelicId=%s"),
			*RelicId.ToString());

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[RelicChoice] Relic selected. RelicId=%s"),
		*RelicId.ToString());

	CloseRelicChoice();
}

void APBRelicChoiceActor::CloseRelicChoice()
{
	if (RelicChoiceWidget)
	{
		RelicChoiceWidget->RemoveFromParent();
	}

	APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(
			this,
			0);

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(
			FInputModeGameOnly());
	}
	
	FPBChoiceType Message;
	Message.Exit = 1;
	

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		GameplayTags::Event_UI_Choice_Exit,Message);
}