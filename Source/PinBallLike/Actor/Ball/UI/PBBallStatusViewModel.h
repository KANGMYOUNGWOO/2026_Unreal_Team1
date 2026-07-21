#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PBBallStatusViewModel.generated.h"

class APBBallBase;
class UPBBaseResourceComponent;
class UTexture2D;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBallStatusViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ball|Status ViewModel")
	void SetBall(APBBallBase* NewBall, UTexture2D* NewIconTexture);

	UFUNCTION(BlueprintCallable, Category = "Ball|Status ViewModel")
	void SetIconTexture(UTexture2D* NewIconTexture);

	UFUNCTION(BlueprintCallable, Category = "Ball|Status ViewModel")
	void ClearBall();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|Status ViewModel")
	FText BallNameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|Status ViewModel")
	FText HPText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|Status ViewModel")
	float HPPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|Status ViewModel")
	FText ManaText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|Status ViewModel")
	float ManaPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Ball|Status ViewModel")
	TObjectPtr<UTexture2D> IconTexture = nullptr;

protected:
	virtual void BeginDestroy() override;

private:
	void BindBallEvents();
	void UnbindBallEvents();
	void RefreshBallName();
	void RefreshBallStatus();
	void RefreshResource(FName ResourceName);
	void SetResourceValue(FName ResourceName, float CurrentValue, float MaxValue);
	float CalculateGaugePercent(float CurrentValue, float MaxValue) const;

	void HandleResourceCurrentChanged(FName ResourceName, float CurrentValue);
	void HandleResourceChanged(FName ResourceName);

	UPBBaseResourceComponent* GetBallResourceComponent() const;

	UPROPERTY(Transient)
	TObjectPtr<APBBallBase> Ball;

	FDelegateHandle ResourceCurrentChangedHandle;
	FDelegateHandle ResourceChangedHandle;
};
