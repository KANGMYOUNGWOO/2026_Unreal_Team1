#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBossUILayerWidget.generated.h"

class APBBossBase;
class UPBBossEnrageWidget;
class UPBBossIntroWidget;
class UPBBossStatusWidget;
class UPBGolemHandStatusWidget;

UCLASS()
class PINBALLLIKE_API UPBBossUILayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetBoss(APBBossBase* NewBoss);
	void ClearBoss();
	void ShowEnrageWarning();
	void HideEnrageWarning();
	UPBBossIntroWidget* GetBossIntroWidget() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void CacheBossWidgets();

	UPROPERTY(Transient)
	TObjectPtr<UPBBossStatusWidget> BossStatusWidget;

	UPROPERTY(Transient)
	TObjectPtr<UPBGolemHandStatusWidget> GolemHandStatusWidget;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossEnrageWidget> BossEnrageWidget;

	UPROPERTY(Transient)
	TObjectPtr<UPBBossIntroWidget> BossIntroWidget;
};
