#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBRelicIconWidget.generated.h"

class UImage;

UCLASS()
class PINBALLLIKE_API UPBRelicIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetRelicId(FName InRelicId);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RelicIcon;

	UPROPERTY()
	FName RelicId = NAME_None;
};