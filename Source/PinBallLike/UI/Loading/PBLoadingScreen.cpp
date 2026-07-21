#include "PBLoadingScreen.h"

#include "LoadingScreenSettings.h"
#include "SCenterLayout.h"

TSharedRef<SLoadingScreenLayout> FPBLoadingScreen::CreateLoadingScreenWidget()
{
	const ULoadingScreenSettings* LoadingScreenSettings = GetDefault<ULoadingScreenSettings>();
	check(LoadingScreenSettings);

	TSharedRef<SCenterLayout> LoadingScreenWidget = SNew(
		SCenterLayout,
		LoadingScreenSettings->DefaultLoadingScreen,
		LoadingScreenSettings->Center);
	LoadingScreenWidget->SetIsMoviePlayerFadeEnabled(false);
	return LoadingScreenWidget;
}
