#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/PBChoiceNodeAction.h"
#include "PBRelicChoiceActor.generated.h"

class UPBRelicChoiceWidget;

UCLASS()
class PINBALLLIKE_API APBRelicChoiceActor : public AActor ,  public IPBChoiceNodeAction
{
	GENERATED_BODY()

public:
	APBRelicChoiceActor();

	UFUNCTION(BlueprintCallable, Category = "Relic")
	void OpenRelicChoice();
	void OpenAbility() override;
protected:
	virtual void BeginPlay() override;

private:
	void HandleRelicSelected(FName RelicId);
	void CloseRelicChoice();

private:
	UPROPERTY(EditAnywhere, Category = "Relic|UI")
	TSubclassOf<UPBRelicChoiceWidget> RelicChoiceWidgetClass;

	UPROPERTY()
	TObjectPtr<UPBRelicChoiceWidget> RelicChoiceWidget;

	UPROPERTY()
	TArray<FName> CurrentRelicChoices;
};