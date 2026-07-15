#include "PBRelicInventoryWidget.h"

#include "Components/HorizontalBox.h"

#include "PBRelicIconWidget.h"
#include "PinBallLike/Subsystem/Relic/PBRelicSubsystem.h"

void UPBRelicInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

		RelicHorizontalBox->AddChild(
			IconWidget);
	}
}