#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PBUISettings.generated.h"

class UFont;
class UPBGlobalToolbarWidget;

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

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Global Toolbar", meta = (ClampMin = "1.0"))
	float GlobalToolbarHeight = 64.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Global Toolbar")
	TSoftClassPtr<UPBGlobalToolbarWidget> GlobalToolbarWidgetClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Global Toolbar")
	FLinearColor GlobalToolbarColor = FLinearColor(0.025f, 0.060f, 0.078f, 0.90f);

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Global Toolbar")
	FName GlobalToolbarHiddenMapName = TEXT("Lv_MainMenu");
};
