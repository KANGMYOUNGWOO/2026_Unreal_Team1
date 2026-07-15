#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionDetailViewModel.generated.h"

/**
 * 선택한 도감 항목의 상세 패널 표시값을 보관합니다.
 * 선택 없음과 비동기 로딩 상태도 상세 패널이 직접 판단하지 않도록 함께 관리합니다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBCollectionDetailViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Collection|DetailViewModel")
	void SetDisplayData(const FPBCollectionDisplayData& InDisplayData);

	UFUNCTION(BlueprintCallable, Category = "Collection|DetailViewModel")
	void SetEmpty(bool bLoading);

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	bool bHasSelection = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	bool bIsLoading = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	FText NameText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	FText MetaText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	FText DescriptionText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	FText UnlockText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	FText RecordText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|DetailViewModel")
	FLinearColor AccentColor = FLinearColor(0.22f, 0.22f, 0.22f, 1.0f);
};
