#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "PinBallLike/Struct/Collection/PBCollectionTypes.h"
#include "PBCollectionEntryViewModel.generated.h"

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
	static FLinearColor ResolveCardColor();
	static FLinearColor ResolveAccentColor(EPBCollectionCategory Category);
	static FLinearColor ResolveCategoryColor(EPBCollectionCategory Category);
};
