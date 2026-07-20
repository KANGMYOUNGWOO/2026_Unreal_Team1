#pragma once

#include "CoreMinimal.h"
#include "PinBallLike/UI/PBUserWidget.h"
#include "PBSimplePopupWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPBSimplePopupConfirmedSignature);

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBSimplePopupWidget : public UPBUserWidget
{
	GENERATED_BODY()

public:
	void InitializePopup(const FText& InMessage);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Confirm();

	UFUNCTION(BlueprintPure, Category = "UI")
	FText GetMessage() const { return Message; }

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FPBSimplePopupConfirmedSignature OnConfirmed;

protected:
	virtual void OnPopRequested_Implementation() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnPopupInitialized(const FText& InMessage);

private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	FText Message;

	bool bConfirming = false;
};
