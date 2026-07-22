#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinBallLike/Interface/PBChoiceNodeAction.h"
#include "PinBallLike/Struct/Relic/PBRelicViewData.h"
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
	void HandleRelicSelected(FPBRelicViewData SelectedReward);
	void CloseRelicChoice();

private:
	UPROPERTY(EditAnywhere, Category = "Relic|UI")
	TSubclassOf<UPBRelicChoiceWidget> RelicChoiceWidgetClass;

	UPROPERTY()
	TObjectPtr<UPBRelicChoiceWidget> RelicChoiceWidget;

	UPROPERTY()
	TArray<FName> CurrentRelicChoices;

	UPROPERTY(EditAnywhere, Category = "Relic|Reward", meta = (ClampMin = "0"))
	int32 FallbackGoldAmount = 100;
};
