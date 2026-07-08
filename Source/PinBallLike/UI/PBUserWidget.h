#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBUserWidget.generated.h"

UCLASS()
class PINBALLLIKE_API UPBUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void OnPushed();

	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void OnPopRequested();

	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void OnPopped();

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool CompletePop();

	bool IsPopRequested() const { return bIsPopRequested; }
	void SetPopRequested(bool bInPopRequested) { bIsPopRequested = bInPopRequested; }

private:
	UPROPERTY(Transient)
	bool bIsPopRequested = false;
};
