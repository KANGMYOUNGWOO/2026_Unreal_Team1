#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PBBossPatternBase.generated.h"

class APBBossBase;
class UPBBossPatternComponent;

UCLASS(Abstract, Blueprintable)
class PINBALLLIKE_API UPBBossPatternBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void InitializePattern(UPBBossPatternComponent* NewOwnerPatternComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	bool CanExecute(APBBossBase* Boss) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	void StartPattern(APBBossBase* Boss);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Boss|Pattern")
	void CancelPattern(APBBossBase* Boss);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void FinishPattern();

	UFUNCTION(BlueprintPure, Category = "Boss|Pattern")
	UPBBossPatternComponent* GetOwnerPatternComponent() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	FName PatternName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern", meta = (ClampMin = "0"))
	float CooldownSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	bool IsEnabled = true;

protected:
	virtual bool CanExecute_Implementation(APBBossBase* Boss) const;
	virtual void StartPattern_Implementation(APBBossBase* Boss);
	virtual void CancelPattern_Implementation(APBBossBase* Boss);

private:
	UPROPERTY(Transient)
	TObjectPtr<UPBBossPatternComponent> OwnerPatternComponent;
};
