#include "PBBumperViewModel.h"

void UPBBumperViewModel::SetTriggerCount(const int32 CurrentCount, const int32 RequiredCount)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentTriggerCountText, FText::AsNumber(CurrentCount));
	UE_MVVM_SET_PROPERTY_VALUE(RequiredTriggerCountText, FText::AsNumber(RequiredCount));
}
