#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PBUISettings.generated.h"

class UFont;

UCLASS(Config = Game, DefaultConfig, DisplayName = "PB UI")
class PINBALLLIKE_API UPBUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Text")
	TSoftObjectPtr<UFont> DefaultTextFont;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Text")
	bool bOverrideDefaultTypefaceName = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (EditCondition = "bOverrideDefaultTypefaceName"))
	FName DefaultTypefaceName = TEXT("Pixel_Text");
};
