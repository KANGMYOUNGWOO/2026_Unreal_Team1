#include "PBBossEnrageViewModel.h"

void UPBBossEnrageViewModel::ShowEnrageWarning()
{
	UE_MVVM_SET_PROPERTY_VALUE(EnrageWarningText, FText::FromString(TEXT("Warning!!")));
	UE_MVVM_SET_PROPERTY_VALUE(EnrageWarningVisibility, ESlateVisibility::HitTestInvisible);
	UE_MVVM_SET_PROPERTY_VALUE(VignetteOpacity, 0.3f);
}

void UPBBossEnrageViewModel::HideEnrageWarning()
{
	UE_MVVM_SET_PROPERTY_VALUE(EnrageWarningText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(EnrageWarningVisibility, ESlateVisibility::Collapsed);
}
