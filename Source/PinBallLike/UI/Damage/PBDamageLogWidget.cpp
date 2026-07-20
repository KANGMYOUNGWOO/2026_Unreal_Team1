#include "PBDamageLogWidget.h"

#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"
#include "Components/PanelWidget.h"

void UPBDamageLogWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	InitializePool();
}

void UPBDamageLogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameplayMessageSubsystem::HasInstance(this) && !DamageLogRequestedHandle.IsValid())
	{
		DamageLogRequestedHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FPBDamageLogMessage>(
			GameplayTags::Event_UI_DamageLog_Requested,
			this,
			&UPBDamageLogWidget::HandleDamageLogRequested);
	}
}

void UPBDamageLogWidget::NativeDestruct()
{
	DamageLogRequestedHandle.Unregister();
	DamageLogRequestedHandle = FGameplayMessageListenerHandle();

	Super::NativeDestruct();
}

bool UPBDamageLogWidget::ShowDamage(
	const EPBDamageLogStyle InLogType,
	const int32 Damage,
	const FVector WorldLocation)
{
	UUserWidget* Entry = AcquireEntry();
	if (!Entry)
	{
		return false;
	}

	OnEntryActivated(Entry, InLogType, Damage, WorldLocation);
	return true;
}

void UPBDamageLogWidget::ReleaseEntry(UUserWidget* Entry)
{
	if (!Entry || ActiveEntries.RemoveSingle(Entry) == 0)
	{
		return;
	}

	Entry->SetVisibility(ESlateVisibility::Collapsed);
	InactiveEntries.Add(Entry);
}

void UPBDamageLogWidget::HandleDamageLogRequested(
	FGameplayTag,
	const FPBDamageLogMessage& Message)
{
	ShowDamage(Message.Style, Message.DamageAmount, Message.HitLocation);
}

void UPBDamageLogWidget::InitializePool()
{
	if (!EntryContainer || !EntryWidgetClass || InactiveEntries.Num() + ActiveEntries.Num() > 0)
	{
		return;
	}

	InactiveEntries.Reserve(PoolSize);
	ActiveEntries.Reserve(PoolSize);

	for (int32 Index = 0; Index < PoolSize; ++Index)
	{
		UUserWidget* Entry = CreateWidget<UUserWidget>(GetOwningPlayer(), EntryWidgetClass);
		if (!Entry)
		{
			continue;
		}

		EntryContainer->AddChild(Entry);
		Entry->SetVisibility(ESlateVisibility::Collapsed);
		InactiveEntries.Add(Entry);
		OnEntryCreated(Entry);
	}
}

UUserWidget* UPBDamageLogWidget::AcquireEntry()
{
	if (InactiveEntries.IsEmpty())
	{
		return nullptr;
	}

	UUserWidget* Entry = InactiveEntries.Pop(EAllowShrinking::No);
	ActiveEntries.Add(Entry);
	Entry->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Entry;
}
