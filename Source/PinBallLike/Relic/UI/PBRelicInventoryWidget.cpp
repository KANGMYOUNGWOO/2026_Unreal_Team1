#include "PBRelicInventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"

#include "PBRelicIconWidget.h"
#include "PinBallLike/Subsystem/Relic/PBRelicSubsystem.h"

void UPBRelicInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!bManagedByGlobalToolbar)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	BindRelicEvents();
	RefreshRelicIcons();
}

void UPBRelicInventoryWidget::NativeDestruct()
{
	UnbindRelicEvents();

	Super::NativeDestruct();
}

void UPBRelicInventoryWidget::HandleRelicsChanged()
{
	RefreshRelicIcons();
}

void UPBRelicInventoryWidget::SetIconSize(const float InIconSize)
{
	IconSize = FMath::Max(InIconSize, 16.0f);
	if (IsConstructed())
	{
		RefreshRelicIcons();
	}
}

void UPBRelicInventoryWidget::SetManagedByGlobalToolbar()
{
	bManagedByGlobalToolbar = true;
}

void UPBRelicInventoryWidget::BindRelicEvents()
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

	RelicSubsystem->OnRelicAcquired.AddUObject(
		this,
		&UPBRelicInventoryWidget::HandleRelicAcquired);

	RelicSubsystem->OnRelicRemoved.AddUObject(
		this,
		&UPBRelicInventoryWidget::HandleRelicRemoved);
}

void UPBRelicInventoryWidget::UnbindRelicEvents()
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

	RelicSubsystem->OnRelicAcquired.RemoveAll(this);
	RelicSubsystem->OnRelicRemoved.RemoveAll(this);
}

void UPBRelicInventoryWidget::HandleRelicAcquired(
	const FName RelicId)
{
	RefreshRelicIcons();
}

void UPBRelicInventoryWidget::HandleRelicRemoved(
	const FName RelicId)
{
	RefreshRelicIcons();
}

void UPBRelicInventoryWidget::RefreshRelicIcons()
{
	if (!RelicHorizontalBox ||
		!RelicIconWidgetClass)
	{
		return;
	}

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

	RelicHorizontalBox->ClearChildren();

	for (const FName RelicId :
		 RelicSubsystem->GetOwnedRelicIds())
	{
		UPBRelicIconWidget* IconWidget =
			CreateWidget<UPBRelicIconWidget>(
				GetOwningPlayer(),
				RelicIconWidgetClass);

		if (!IconWidget)
		{
			continue;
		}

		IconWidget->SetRelicId(RelicId);

		USizeBox* IconSizeBox = WidgetTree
			? WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass())
			: nullptr;
		if (!IconSizeBox)
		{
			continue;
		}

		IconSizeBox->SetWidthOverride(IconSize);
		IconSizeBox->SetHeightOverride(IconSize);
		IconSizeBox->AddChild(IconWidget);
		if (UHorizontalBoxSlot* IconSlot = Cast<UHorizontalBoxSlot>(
			RelicHorizontalBox->AddChild(IconSizeBox)))
		{
			IconSlot->SetPadding(FMargin(0.0f, 0.0f, 3.0f, 0.0f));
			IconSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
}
