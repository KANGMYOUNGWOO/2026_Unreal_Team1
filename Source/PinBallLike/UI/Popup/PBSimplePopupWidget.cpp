#include "PBSimplePopupWidget.h"

void UPBSimplePopupWidget::InitializePopup(const FText& InMessage)
{
	Message = InMessage;
	ClosedCallback.Unbind();
	bClosing = false;
	OnPopupInitialized(Message);
}

void UPBSimplePopupWidget::SetClosedCallback(FPBSimplePopupClosedDelegate InCallback)
{
	ClosedCallback = MoveTemp(InCallback);
}

void UPBSimplePopupWidget::Confirm()
{
	ClosePopup(true);
}

void UPBSimplePopupWidget::Cancel()
{
	ClosePopup(false);
}

void UPBSimplePopupWidget::ClosePopup(const bool bConfirmed)
{
	if (bClosing)
	{
		return;
	}

	bClosing = true;
	FPBSimplePopupClosedDelegate Callback = MoveTemp(ClosedCallback);
	if (!CompletePop())
	{
		ClosedCallback = MoveTemp(Callback);
		bClosing = false;
		return;
	}

	OnClosed.Broadcast(bConfirmed);
	Callback.ExecuteIfBound(bConfirmed);
}

void UPBSimplePopupWidget::OnPopRequested_Implementation()
{
	SetPopRequested(false);
}
