#include "PBBossIntroBallWidget.h"

#include "PBBossIntroBallViewModel.h"
#include "View/MVVMView.h"

void UPBBossIntroBallWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CreateBallIntroViewModel();
}

UPBBossIntroBallViewModel* UPBBossIntroBallWidget::CreateBallIntroViewModel()
{
	if (!BallIntroViewModel)
	{
		BallIntroViewModel = NewObject<UPBBossIntroBallViewModel>(this);
	}

	ApplyViewModelToWidget();
	return BallIntroViewModel;
}

void UPBBossIntroBallWidget::SetBallIntroViewModel(UPBBossIntroBallViewModel* NewBallIntroViewModel)
{
	BallIntroViewModel = NewBallIntroViewModel;
	ApplyViewModelToWidget();
}

UPBBossIntroBallViewModel* UPBBossIntroBallWidget::GetBallIntroViewModel() const
{
	return BallIntroViewModel;
}

bool UPBBossIntroBallWidget::ApplyViewModelToWidget()
{
	if (!BallIntroViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossIntroBallWidget ApplyViewModelToWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(BallIntroViewModel);
	const bool IsResult = View->SetViewModelByClass(ViewModelInterface);
	if (!IsResult)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossIntroBallWidget ApplyViewModelToWidget failed. Widget=%s ViewModel=%s"),
			*GetNameSafe(this),
			*GetNameSafe(BallIntroViewModel));
	}

	return IsResult;
}
