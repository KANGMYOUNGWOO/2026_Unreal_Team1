#include "PBGolemHandStatusWidget.h"

#include "PBGolemHandStatusViewModel.h"
#include "View/MVVMView.h"

void UPBGolemHandStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureViewModel();
}

void UPBGolemHandStatusWidget::SetGolemBoss(APBGolemBoss* NewGolemBoss)
{
	EnsureViewModel();
	if (HandStatusViewModel)
	{
		HandStatusViewModel->SetGolemBoss(NewGolemBoss);
	}
}

void UPBGolemHandStatusWidget::ClearGolemBoss()
{
	if (HandStatusViewModel)
	{
		HandStatusViewModel->ClearGolemBoss();
	}
}

void UPBGolemHandStatusWidget::NativeDestruct()
{
	ClearGolemBoss();
	Super::NativeDestruct();
}

void UPBGolemHandStatusWidget::EnsureViewModel()
{
	if (!HandStatusViewModel)
	{
		HandStatusViewModel = NewObject<UPBGolemHandStatusViewModel>(this);
	}

	if (HandStatusViewModel)
	{
		ApplyViewModelToWidget();
	}
}

bool UPBGolemHandStatusWidget::ApplyViewModelToWidget()
{
	if (!HandStatusViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		UE_LOG(LogTemp, Warning, TEXT("GolemHandStatusWidget failed. Widget=%s MVVMView extension is null"),
			*GetNameSafe(this));
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(HandStatusViewModel);
	return View->SetViewModelByClass(ViewModelInterface);
}
