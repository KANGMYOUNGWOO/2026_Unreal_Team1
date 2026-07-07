#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PBUIManagerSubsystem.generated.h"

class UPBUserWidget;

UCLASS()
class PINBALLLIKE_API UPBUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	UPBUserWidget* PushWidget(TSubclassOf<UPBUserWidget> WidgetClass, int32 ZOrder = 0);

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool PopWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void PopAllWidgets();

	UFUNCTION(BlueprintPure, Category = "UI")
	UPBUserWidget* GetTopWidget() const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPBUserWidget>> WidgetStack;
};
