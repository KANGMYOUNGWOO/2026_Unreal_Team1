#include "PBBumperUIPresenter.h"
#include "Blueprint/UserWidget.h"
#include "PinBallLike/Actor/Bumper/PBBumperBase.h"
#include "PBBumperViewModel.h"
#include "View/MVVMView.h"

bool UPBBumperUIPresenter::Bind(APBBumperBase* InBumper, UUserWidget* InWidget)
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
		this, &UPBBumperUIPresenter::HandleTriggerCountChanged);

	// 바인딩 시점의 현재 상태를 UI에 즉시 반영한다.
	HandleTriggerCountChanged(
		Bumper->GetCurrentTriggerCount(),
		Bumper->GetRequiredTriggerCount());

	return true;
}

bool UPBBumperUIPresenter::ApplyViewModelToWidget(UUserWidget* Widget)
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

void UPBBumperUIPresenter::Unbind()
{
	if (Bumper.IsValid())
	{
		Bumper->OnBumperTriggerCountChanged.RemoveDynamic(
			this, &UPBBumperUIPresenter::HandleTriggerCountChanged);
	}

	Bumper.Reset();
}

void UPBBumperUIPresenter::BeginDestroy()
{
	Unbind();
	Super::BeginDestroy();
}

void UPBBumperUIPresenter::HandleTriggerCountChanged(
	const int32 CurrentCount,
	const int32 RequiredCount)
{
	if (!IsValid(BumperViewModel))
	{
		return;
	}

	BumperViewModel->SetTriggerCount(CurrentCount, RequiredCount);
}
