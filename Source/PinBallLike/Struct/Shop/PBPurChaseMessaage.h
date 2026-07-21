
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBPurChaseMessaage.generated.h"


USTRUCT(BlueprintType)
struct FPBPurChaseMessaage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;
	
};