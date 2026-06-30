#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossUIComponent.generated.h"

class APBBossBase;
class UPBBossStatusWidget;

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossUIComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|UI Component")
	void CreateBossStatusWidget();

	UFUNCTION(BlueprintCallable, Category = "Boss|UI Component")
	void RemoveBossStatusWidget();

	void ConfigureBossStatusWidget(TSubclassOf<UPBBossStatusWidget> NewBossStatusWidgetClass, int32 NewBossStatusWidgetZOrder);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI Component")
	TSubclassOf<UPBBossStatusWidget> BossStatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI Component", meta = (ClampMin = "0"))
	int32 BossStatusWidgetZOrder = 0;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPBBossStatusWidget> BossStatusWidget;
};
