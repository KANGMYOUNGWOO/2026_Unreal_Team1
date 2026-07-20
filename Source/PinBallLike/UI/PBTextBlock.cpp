#include "PBTextBlock.h"

#include "Engine/Font.h"
#include "PinBallLike/DeveloperSettings/PBUISettings.h"

UPBTextBlock::UPBTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPBTextBlock::SynchronizeProperties()
{
	ApplyPBDefaultFont();

	Super::SynchronizeProperties();
}

void UPBTextBlock::ApplyPBDefaultFont()
{
	UFont* DefaultFont = ResolvePBDefaultFont();
	if (!DefaultFont)
	{
		return;
	}

	FSlateFontInfo FontInfo = GetFont();
	bool bChanged = false;

	if (FontInfo.FontObject != DefaultFont)
	{
		FontInfo.FontObject = DefaultFont;
		bChanged = true;
	}

	FName ResolvedTypefaceName = NAME_None;
	if (ResolvePBDefaultTypefaceName(ResolvedTypefaceName)
		&& FontInfo.TypefaceFontName != ResolvedTypefaceName)
	{
		FontInfo.TypefaceFontName = ResolvedTypefaceName;
		bChanged = true;
	}

	if (bChanged)
	{
		SetFont(FontInfo);
	}
}

UFont* UPBTextBlock::ResolvePBDefaultFont() const
{
	const UPBUISettings* UISettings = GetDefault<UPBUISettings>();
	if (!UISettings || UISettings->DefaultTextFont.IsNull())
	{
		return nullptr;
	}

	return UISettings->DefaultTextFont.LoadSynchronous();
}

bool UPBTextBlock::ResolvePBDefaultTypefaceName(FName& OutTypefaceName) const
{
	OutTypefaceName = NAME_None;

	const UPBUISettings* UISettings = GetDefault<UPBUISettings>();
	if (!UISettings || !UISettings->bOverrideDefaultTypefaceName)
	{
		return false;
	}

	OutTypefaceName = UISettings->DefaultTypefaceName;
	return !OutTypefaceName.IsNone();
}
