#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBGlobalToolbarWidget.generated.h"

class UPBPlayerDataSubsystem;
class UPBRelicInventoryWidget;
class UButton;
class UCanvasPanel;
class UHorizontalBox;
class UTextBlock;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBGlobalToolbarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPBGlobalToolbarWidget(const FObjectInitializer& ObjectInitializer);

	float GetConfiguredToolbarHeight() const;
	bool HasDeckButton() const;

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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> DeckButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> RightSection;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global Toolbar|Deck")
	TSoftObjectPtr<UTexture2D> DeckButtonIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global Toolbar|Deck", meta = (ClampMin = "24.0"))
	float DeckButtonSize = 34.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global Toolbar|Option")
	TSoftObjectPtr<UTexture2D> OptionButtonIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global Toolbar|Relic")
	TSoftClassPtr<UPBRelicInventoryWidget> RelicInventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global Toolbar|Relic", meta = (ClampMin = "16.0"))
	float RelicIconSize = 32.0f;

private:
	void EnsureAnchoredToolbarContent();
	void EnsureToolbarButtons();
	void EnsureRelicInventory();

	UFUNCTION()
	void HandleGoldChanged(int32 NewGold);

	UFUNCTION()
	void HandleDeckButtonClicked();

	UPROPERTY(Transient)
	TObjectPtr<UPBPlayerDataSubsystem> PlayerDataSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<UPBRelicInventoryWidget> RelicInventoryWidget;
};
