#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBSimplePopupWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FPBSimplePopupClosedSignature,
	bool,
	bConfirmed);
DECLARE_DELEGATE_OneParam(FPBSimplePopupClosedDelegate, bool);

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBSimplePopupWidget : public UPBUserWidget
{
	GENERATED_BODY()

public:
	void InitializePopup(const FText& InMessage);
	void SetClosedCallback(FPBSimplePopupClosedDelegate InCallback);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Confirm();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Cancel();

	UFUNCTION(BlueprintPure, Category = "UI")
	FText GetMessage() const { return Message; }

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FPBSimplePopupClosedSignature OnClosed;

protected:
	virtual void OnPopRequested_Implementation() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnPopupInitialized(const FText& InMessage);

private:
	void ClosePopup(bool bConfirmed);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	FText Message;

	FPBSimplePopupClosedDelegate ClosedCallback;
	bool bClosing = false;
};
