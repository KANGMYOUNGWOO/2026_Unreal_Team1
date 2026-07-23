#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "PBBumperCatalogEntryWidget.generated.h"

class UDragDropOperation;
class UBorder;
class UTextBlock;
class UPBBumperListItemObject;

UCLASS(Abstract, BlueprintType, Blueprintable)
class PINBALLLIKE_API UPBBumperCatalogEntryWidget
	: public UUserWidget
	, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|ListItem", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BumperName;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|ListItem", meta = (BindWidgetOptional))
	TObjectPtr<UBorder> CatalogIconFrameBorder;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPBBumperListItemObject> BumperListItem;
};
