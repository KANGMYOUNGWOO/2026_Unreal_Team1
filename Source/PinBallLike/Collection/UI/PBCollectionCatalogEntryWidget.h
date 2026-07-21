#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "PBCollectionCatalogEntryWidget.generated.h"

class UBorder;
class UTextBlock;

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

	FSlateBrush DefaultGlyphBrush;
	FLinearColor DefaultGlyphBrushColor = FLinearColor::White;
	bool bHasDefaultGlyphStyle = false;
};
