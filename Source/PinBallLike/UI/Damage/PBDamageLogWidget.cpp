#include "PBDamageLogWidget.h"

#include "PinBallLike/GamePlayTag/GamePlayTags.h"
#include "PinBallLike/Struct/UI/PBDamageLogMessage.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

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

	ApplyEntryOutlineColor(Entry, InLogType);
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

void UPBDamageLogWidget::ApplyEntryOutlineColor(
	UUserWidget* Entry,
	const EPBDamageLogStyle InLogType) const
{
	if (!Entry)
	{
		return;
	}

	UTextBlock* DamageText = Cast<UTextBlock>(Entry->GetWidgetFromName(TEXT("DamageText")));
	if (!DamageText)
	{
		return;
	}

	FSlateFontInfo FontInfo = DamageText->GetFont();
	FontInfo.OutlineSettings.OutlineColor = InLogType == EPBDamageLogStyle::EnemyAttack
		? FLinearColor::FromSRGBColor(FColor(0x3F, 0x00, 0x04, 0xFF))
		: FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
	DamageText->SetFont(FontInfo);
}
