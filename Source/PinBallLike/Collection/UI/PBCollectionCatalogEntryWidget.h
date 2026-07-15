#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "PBCollectionCatalogEntryWidget.generated.h"

class UBorder;
class UTextBlock;

/** 다섯 탭이 공통으로 재사용하는 목록 카드의 데이터 바인딩만 담당합니다. */
UCLASS()
class PINBALLLIKE_API UPBCollectionCatalogEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
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
};
