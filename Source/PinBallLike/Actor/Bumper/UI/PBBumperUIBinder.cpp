#include "PBBumperUIBinder.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Actor/Bumper/Modular/PBModularBumperBase.h"
#include "PBBumperViewModel.h"
#include "View/MVVMView.h"

bool UPBBumperUIBinder::Bind(APBModularBumperBase* InBumper, UUserWidget* InWidget)
{
	Unbind();

	if (!IsValid(InBumper) || !IsValid(InWidget))
	{
		return false;
	}

	if (!IsValid(BumperViewModel))
	{
		BumperViewModel = NewObject<UPBBumperViewModel>(this);
	}

	if (!ApplyViewModelToWidget(InWidget))
	{
		return false;
	}

	Bumper = InBumper;
	Bumper->OnBumperTriggerCountChanged.AddUniqueDynamic(
		this, &UPBBumperUIBinder::HandleTriggerCountChanged);

	// 바인딩 시점의 현재 상태를 UI에 즉시 반영한다.
	HandleTriggerCountChanged(
		Bumper->GetCurrentTriggerCount(),
		Bumper->GetRequiredTriggerCount());

	return true;
}

bool UPBBumperUIBinder::ApplyViewModelToWidget(UUserWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return false;
	}

	if (!IsValid(BumperViewModel))
	{
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(BumperViewModel);

	if (UMVVMView* View = Widget->GetExtension<UMVVMView>())
	{
		const bool IsViewModelSet = View->SetViewModelByClass(ViewModelInterface);
		return IsViewModelSet;
	}

	return false;
}

void UPBBumperUIBinder::Unbind()
{
	if (Bumper.IsValid())
	{
		Bumper->OnBumperTriggerCountChanged.RemoveDynamic(
			this, &UPBBumperUIBinder::HandleTriggerCountChanged);
	}

	Bumper.Reset();
}

void UPBBumperUIBinder::BeginDestroy()
{
	Unbind();
	Super::BeginDestroy();
}

void UPBBumperUIBinder::HandleTriggerCountChanged(
	const int32 CurrentCount,
	const int32 RequiredCount)
{
	if (!IsValid(BumperViewModel))
	{
		return;
	}

	BumperViewModel->SetTriggerCount(CurrentCount, RequiredCount);
}
