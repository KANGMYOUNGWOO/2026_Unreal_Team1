#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBRelicInventoryWidget.generated.h"

class UHorizontalBox;
class UPBRelicIconWidget;

UCLASS()
class PINBALLLIKE_API UPBRelicInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void HandleRelicsChanged();
	
private:
	void BindRelicEvents();
	void UnbindRelicEvents();

	void RefreshRelicIcons();

	void HandleRelicAcquired(FName RelicId);
	void HandleRelicRemoved(FName RelicId);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> RelicHorizontalBox;

	UPROPERTY(EditAnywhere, Category = "Relic|UI")
	TSubclassOf<UPBRelicIconWidget> RelicIconWidgetClass;
};