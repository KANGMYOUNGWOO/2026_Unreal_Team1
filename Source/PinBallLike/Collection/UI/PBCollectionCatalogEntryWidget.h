#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "PBCollectionCatalogEntryWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UTexture2D;
class UWidget;
enum class EPBCollectionCategory : uint8;

UCLASS()
class PINBALLLIKE_API UPBCollectionCatalogEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> CardBorder;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SubtitleText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> AccentBorder;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> IconLetterText;

	UPROPERTY(BlueprintReadOnly, Category = "Collection|Entry", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> GlyphBorder;

	UPROPERTY(Transient)
	TObjectPtr<UImage> GlyphIconImage;

	void ApplyIconTexture(UTexture2D* IconTexture);
	void ApplyFallbackGlyph(EPBCollectionCategory Category);
	void SetGlyphContent(UWidget* Content);

	FSlateBrush DefaultGlyphBrush;
	FLinearColor DefaultGlyphBrushColor = FLinearColor::White;
	FMargin DefaultGlyphPadding;
	bool bHasDefaultGlyphStyle = false;
};
