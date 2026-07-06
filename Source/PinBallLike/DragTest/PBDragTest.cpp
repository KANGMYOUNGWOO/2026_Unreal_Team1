#include "PBDragTest.h"

#include "../Struct/GamePlayMessage/PBDragMessage.h"
#include "../GamePlayTag/GamePlayTags.h"

APBDragTest::APBDragTest()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APBDragTest::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& MessageSubsystem =
		UGameplayMessageSubsystem::Get(this);

	DragStartedHandle =
		MessageSubsystem.RegisterListener<FPBDragStartedMessage>(
			GameplayTags::Event_UI_Drag_Started,
			this,
			&APBDragTest::HandleDragStarted);

	

	UE_LOG(LogTemp, Warning, TEXT("PBDragTest Registered"));
}

void APBDragTest::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DragStartedHandle.Unregister();
	DragCancelledHandle.Unregister();

	Super::EndPlay(EndPlayReason);
}

void APBDragTest::HandleDragStarted(
	FGameplayTag Channel,
	const FPBDragStartedMessage& Message)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Received Drag Started / ItemId=%d / Source=%s"),
		Message.ItemId,
		*GetNameSafe(Message.SourceObject));
}

