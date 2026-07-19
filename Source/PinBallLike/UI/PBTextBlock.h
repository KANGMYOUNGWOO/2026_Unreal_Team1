#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "PBTextBlock.generated.h"

class UFont;

UCLASS(meta = (DisplayName = "PB Text Block"))
class PINBALLLIKE_API UPBTextBlock : public UTextBlock
{
	GENERATED_BODY()

public:
	UPBTextBlock(const FObjectInitializer& ObjectInitializer);

	virtual void SynchronizeProperties() override;

	UFUNCTION(BlueprintCallable, Category = "PB Text")
	void ApplyPBDefaultFont();

private:
	UFont* ResolvePBDefaultFont() const;
	bool ResolvePBDefaultTypefaceName(FName& OutTypefaceName) const;
};
