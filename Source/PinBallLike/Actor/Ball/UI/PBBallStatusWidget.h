#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PBBallStatusWidget.generated.h"

class APBBallBase;
class UPBStatusEffectComponent;
class UPBStatusEffectItemWidget;
class UPBBallStatusViewModel;
class UImage;
class UPanelWidget;
class UTexture2D;

UCLASS()
class PINBALLLIKE_API UPBBallStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Status UI")
	void SetBall(APBBallBase* NewBall, UTexture2D* NewIconTexture);

	UFUNCTION(BlueprintCallable, Category = "Ball|Status UI")
	void SetIconTexture(UTexture2D* NewIconTexture);

	UFUNCTION(BlueprintCallable, Category = "Ball|Status UI")
	void ClearBall();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ball|StatusEffect UI")
	TSubclassOf<UPBStatusEffectItemWidget> StatusEffectItemWidgetClass;

private:
	void EnsureStatusViewModel();
	bool ApplyViewModelToWidget();
	void BindStatusEffectComponent();
	void UnbindStatusEffectComponent();
	void RefreshStatusEffectItems();
	void UpsertStatusEffectItem(FName StatusEffectId, int32 StackCount);
	void RemoveStatusEffectItem(FName StatusEffectId);
	UTexture2D* ResolveStatusEffectIcon(FName StatusEffectId) const;
	void RefreshBallIconState(UTexture2D* IconTexture);

	UFUNCTION()
	void HandleStatusEffectApplied(FName StatusEffectId, int32 StackCount);

	UFUNCTION()
	void HandleStatusEffectRemoved(FName StatusEffectId, int32 StackCount);

	UFUNCTION()
	void HandleStatusEffectStackChanged(FName StatusEffectId, int32 StackCount);

	UPROPERTY(Transient)
	TObjectPtr<UPBBallStatusViewModel> StatusViewModel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> StatusEffectPanel;

	UPROPERTY(Transient)
	TObjectPtr<UImage> BallIconImage;

	UPROPERTY(EditDefaultsOnly, Category = "Ball|Status UI")
	FLinearColor EmptyBallIconColor = FLinearColor(0.035f, 0.045f, 0.055f, 0.95f);

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> Ball;

	UPROPERTY(Transient)
	TObjectPtr<UPBStatusEffectComponent> StatusEffectComponent;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UPBStatusEffectItemWidget>> StatusEffectItemWidgets;
};
