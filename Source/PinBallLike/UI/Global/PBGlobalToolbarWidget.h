#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBGlobalToolbarWidget.generated.h"

class UPBPlayerDataSubsystem;
class UTextBlock;

UCLASS()
class PINBALLLIKE_API UPBGlobalToolbarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	float GetConfiguredToolbarHeight() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// C++ subclasses override only the section they own.
	virtual TSharedRef<SWidget> BuildLeftSection();
	virtual TSharedRef<SWidget> BuildCenterSection();
	virtual TSharedRef<SWidget> BuildRightSection();
	virtual TSharedRef<SWidget> BuildLowerSection();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GoldText;

private:
	UFUNCTION()
	void HandleGoldChanged(int32 NewGold);

	UPROPERTY(Transient)
	TObjectPtr<UPBPlayerDataSubsystem> PlayerDataSubsystem;
};
