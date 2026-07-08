#include "PBUserWidget.h"

#include "PinBallLike/Subsystem/PBUIManagerSubsystem.h"

void UPBUserWidget::OnPushed_Implementation()
{
}

void UPBUserWidget::OnPopRequested_Implementation()
{
	CompletePop();
}

void UPBUserWidget::OnPopped_Implementation()
{
}

bool UPBUserWidget::CompletePop()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return false;
	}

	UPBUIManagerSubsystem* UIManagerSubsystem = GameInstance->GetSubsystem<UPBUIManagerSubsystem>();
	if (!IsValid(UIManagerSubsystem))
	{
		return false;
	}

	return UIManagerSubsystem->CompletePopWidget(this);
}
