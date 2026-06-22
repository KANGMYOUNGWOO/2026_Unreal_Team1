// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Comboable.generated.h"

UINTERFACE()
class UComboable : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IComboable
{
	GENERATED_BODY()

public:
	virtual int32 GetCombo() const = 0;
	virtual void SetCombo(int32 Value) = 0;
	virtual void AddCombo(int32 Delta) = 0;
	virtual bool TryConsumeCombo(int32 Cost) = 0;
	virtual void ResetCombo() = 0;
};
