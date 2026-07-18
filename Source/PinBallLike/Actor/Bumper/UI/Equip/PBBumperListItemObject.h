// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Bumper/PBBumperTypes.h"
#include "UObject/PrimaryAssetId.h"
#include "PBBumperListItemObject.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPBBumperListItemClickedSignature, FName, RowName);

UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBumperListItemObject : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Bumper|ListItem")
	void SetBumperListItemData(
		FName InRowName,
		FText InDisplayName,
		EPBBumperType InBumperType,
		FPrimaryAssetId InPrimaryAssetId,
		UTexture2D* InIconTexture,
		bool bInEquip);

	UFUNCTION(BlueprintCallable, Category = "Bumper|ListItem")
	void SetEquip(bool bInEquip);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Bumper|ListItem")
	ESlateVisibility GetEquipFrameVisibility() const;

	UFUNCTION(BlueprintCallable, Category = "Bumper|ListItem")
	void BroadcastClicked();

	UPROPERTY(BlueprintAssignable, Category = "Bumper|ListItem")
	FPBBumperListItemClickedSignature OnClicked;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|ListItem")
	FName RowName = NAME_None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|ListItem")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|ListItem")
	EPBBumperType BumperType = EPBBumperType::Rebound;

	UPROPERTY(BlueprintReadOnly, Category = "Bumper|ListItem")
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|ListItem")
	TObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|ListItem")
	bool bIsEquip = false;
};
