#include "PBBallStatusWidget.h"

#include "PBBallStatusViewModel.h"
#include "View/MVVMView.h"

void UPBBallStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureStatusViewModel();
}

void UPBBallStatusWidget::SetBall(APBBallBase* NewBall, UTexture2D* NewIconTexture)
{
	EnsureStatusViewModel();
	if (StatusViewModel)
	{
		StatusViewModel->SetBall(NewBall, NewIconTexture);
	}
}

void UPBBallStatusWidget::SetIconTexture(UTexture2D* NewIconTexture)
{
	EnsureStatusViewModel();
	if (StatusViewModel)
	{
		StatusViewModel->SetIconTexture(NewIconTexture);
	}
}

void UPBBallStatusWidget::ClearBall()
{
	if (StatusViewModel)
	{
		StatusViewModel->ClearBall();
	}
}

void UPBBallStatusWidget::NativeDestruct()
{
	ClearBall();
	Super::NativeDestruct();
}

void UPBBallStatusWidget::EnsureStatusViewModel()
{
	if (!StatusViewModel)
	{
		StatusViewModel = NewObject<UPBBallStatusViewModel>(this);
	}

	if (StatusViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBBallStatusWidget::ApplyViewModelToWidget()
{
	if (!StatusViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallStatusWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(StatusViewModel);
	const bool IsResult = View->SetViewModelByClass(ViewModelInterface);
	if (!IsResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BallStatusWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(StatusViewModel));
	}

	return IsResult;
}
