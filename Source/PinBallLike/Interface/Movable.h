// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Movable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UMovable : public UInterface
{
	GENERATED_BODY()
};

class PINBALLLIKE_API IMovable
{
	GENERATED_BODY()

public:
	virtual FVector GetVelocity() const = 0;
	virtual void AddVelocity(FVector VelocityToAdd) = 0;
	virtual void AddImpulse(FVector Impulse) = 0;
	virtual void StopMovement() = 0;
	virtual void PauseMovement() = 0;
	virtual void ResumeMovement() = 0;
	virtual bool IsMovementPaused() const = 0;
};
