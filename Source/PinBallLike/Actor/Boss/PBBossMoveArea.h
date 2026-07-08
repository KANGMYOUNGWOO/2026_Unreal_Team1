#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PBBossMoveArea.generated.h"

class UBoxComponent;

UCLASS(Blueprintable)
class PINBALLLIKE_API APBBossMoveArea : public AActor
{
	GENERATED_BODY()

public:
	APBBossMoveArea();

	UFUNCTION(BlueprintPure, Category = "Boss|Move Area")
	FVector GetAreaCenter() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Move Area")
	FVector GetAreaExtent() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Move Area")
	FVector ClampLocation(const FVector& SourceLocation) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Move Area")
	TObjectPtr<UBoxComponent> AreaBox;
};
