#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Styling/SlateBrush.h"
#include "PBBossIntroViewModel.generated.h"

class APBBossBase;

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBossIntroViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Intro ViewModel")
	void SetBoss(APBBossBase* NewBoss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Intro ViewModel")
	void ClearBoss();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro ViewModel")
	FText BossNameText = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Boss|Intro ViewModel")
	FSlateBrush BossIntroBrush;
};
