#include "PBRelicChoiceActor.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "PinBallLike/Struct/Choice/PBChoiceType.h"
#include "PinBallLike/Relic/UI/PBRelicChoiceWidget.h"
#include "PinBallLike/Subsystem/PBPlayerDataSubsystem.h"
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

	RelicSubsystem->GetRandomRelicIds(3, CurrentRelicChoices);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController ||
		!RelicChoiceWidgetClass)
	{
		UE_LOG(
	LogTemp,
	Warning,
	TEXT("[RelicChoiceNode] PlayerController is invalid."));
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

	RelicChoiceWidget->SetRelicChoices(CurrentRelicChoices, FallbackGoldAmount);

	RelicChoiceWidget->AddToViewport();

	PlayerController->bShowMouseCursor = true;
	PlayerController->SetInputMode(
		FInputModeGameAndUI());
}

void APBRelicChoiceActor::OpenAbility()
{
	OpenRelicChoice();
}

void APBRelicChoiceActor::HandleRelicSelected(const FPBRelicViewData SelectedReward)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	if (SelectedReward.RewardType == EPBRelicChoiceRewardType::Gold)
	{
		UPBPlayerDataSubsystem* PlayerDataSubsystem = GameInstance->GetSubsystem<UPBPlayerDataSubsystem>();
		if (!PlayerDataSubsystem)
		{
			return;
		}

		PlayerDataSubsystem->GainGold(SelectedReward.GoldAmount);
		UE_LOG(LogTemp, Warning, TEXT("[RelicChoice] Gold selected. Amount=%d CurrentGold=%d"), SelectedReward.GoldAmount, PlayerDataSubsystem->GetCurrentGold());
		CloseRelicChoice();
		return;
	}

	const FName RelicId = SelectedReward.RelicId;
	UPBRelicSubsystem* RelicSubsystem = GameInstance->GetSubsystem<UPBRelicSubsystem>();

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
