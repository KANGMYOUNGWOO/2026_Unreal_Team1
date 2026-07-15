#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionEntryViewModel.generated.h"

/**
 * 도감 카드 한 장에 필요한 표시값을 보관합니다.
 * 원본 도감 데이터를 UMG가 바로 바인딩할 수 있는 텍스트와 색상으로 변환합니다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class PINBALLLIKE_API UPBCollectionEntryViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Collection|EntryViewModel")
	void SetDisplayData(const FPBCollectionDisplayData& InDisplayData);

	UFUNCTION(BlueprintCallable, Category = "Collection|EntryViewModel")
	void Clear();

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|EntryViewModel")
	FName CollectionId = NAME_None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|EntryViewModel")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|EntryViewModel")
	FText MetaText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|EntryViewModel")
	FText ShortDescription;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|EntryViewModel")
	FLinearColor CardColor = FLinearColor::Transparent;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|EntryViewModel")
	FLinearColor AccentColor = FLinearColor::Transparent;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Collection|EntryViewModel")
	FLinearColor CategoryColor = FLinearColor::Transparent;

private:
	static FLinearColor ResolveCardColor(EPBCollectionState State);
	static FLinearColor ResolveAccentColor(EPBCollectionCategory Category, EPBCollectionState State);
	static FLinearColor ResolveCategoryColor(EPBCollectionCategory Category, EPBCollectionState State);
};
