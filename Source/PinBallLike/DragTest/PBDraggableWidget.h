#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBDraggableWidget.generated.h"

UCLASS()
class PINBALLLIKE_API UPBDraggableWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drag")
    int32 ItemId = 100;

protected:
    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual void NativeOnDragDetected(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent,
        UDragDropOperation*& OutOperation) override;


private:
    void BroadcastDragStarted();
   
};