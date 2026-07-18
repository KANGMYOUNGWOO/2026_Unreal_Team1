#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "MVVMViewModelBase.h"
#include "PBBumperInfoPanelViewModel.generated.h"

class UTexture2D;

/** 선택한 범퍼의 상세 정보와 장착 상태를 정보 패널에 제공하는 ViewModel입니다. */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBBumperInfoPanelViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Bumper|InfoPanel")
	void SetBumperInfoPanelData(
		FText InTitleName,
		UTexture2D* InIconImage,
		FText InDescription,
		bool bInEquip);

	UFUNCTION(BlueprintCallable, Category = "Bumper|InfoPanel")
	void ClearBumperInfoPanelData();

	UFUNCTION(BlueprintCallable, Category = "Bumper|InfoPanel")
	void SetEquip(bool bInEquip);

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Bumper|InfoPanel")
	ESlateVisibility GetIsEquipVisibility() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Bumper|InfoPanel")
	ESlateVisibility GetIsNotEquipVisibility() const;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|InfoPanel")
	FText TitleName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|InfoPanel")
	TObjectPtr<UTexture2D> IconImage;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|InfoPanel")
	FText Description;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|InfoPanel")
	bool bIsEquip = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Bumper|InfoPanel")
	bool bIsNotEquip = true;
};
