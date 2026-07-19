#include "PBSimplePopupWidget.h"

void UPBSimplePopupWidget::InitializePopup(const FText& InMessage)
{
	Message = InMessage;
	bConfirming = false;
	OnPopupInitialized(Message);
}

void UPBSimplePopupWidget::Confirm()
{
	if (bConfirming)
	{
		return;
	}

	bConfirming = true;
	if (!CompletePop())
	{
		bConfirming = false;
		return;
	}

	OnConfirmed.Broadcast();
}

void UPBSimplePopupWidget::OnPopRequested_Implementation()
{
	SetPopRequested(false);
}
