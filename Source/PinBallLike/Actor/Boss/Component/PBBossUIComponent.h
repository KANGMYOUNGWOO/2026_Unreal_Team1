#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PBBossUIComponent.generated.h"

class APBBossBase;
class UPBBossIntroWidget;
class UPBBossUILayerWidget;

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PINBALLLIKE_API UPBBossUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPBBossUIComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Boss|UI Component")
	void ShowEnrageWarning();

	UFUNCTION(BlueprintCallable, Category = "Boss|UI Component")
	void HideEnrageWarning();

	UPBBossIntroWidget* GetBossIntroWidget() const;
	void ConfigureBossUILayerClass(TSoftClassPtr<UPBBossUILayerWidget> NewBossUILayerClass);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|UI Component")
	TSoftClassPtr<UPBBossUILayerWidget> BossUILayerClass;

private:
	void CreateBossUILayer();
	void RemoveBossUILayer();

	UPROPERTY(Transient)
	TObjectPtr<UPBBossUILayerWidget> BossUILayer;
};
