#include "PBCollectionEntryWidget.h"

#include "PinBallLike/Collection/ViewModel/PBCollectionEntryViewModel.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "View/MVVMView.h"

void UPBCollectionEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureEntryViewModel();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddUniqueDynamic(this, &UPBCollectionEntryWidget::HandleClicked);
	}

	ValidateRequiredWidgetBindings();
	Refresh();
}

bool UPBCollectionEntryWidget::ValidateRequiredWidgetBindings() const
{
	bool bAllWidgetsBound = true;
	const auto CheckBinding = [this, &bAllWidgetsBound](const UObject* Widget, const TCHAR* WidgetName)
	{
		if (!IsValid(Widget))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("%s: Widget Blueprint에 필수 위젯 '%s'가 없거나 이름/타입이 일치하지 않습니다."),
				*GetName(),
				WidgetName);
			bAllWidgetsBound = false;
		}
	};

	CheckBinding(EntryButton, TEXT("EntryButton"));
	CheckBinding(CardBorder, TEXT("CardBorder"));
	CheckBinding(AccentBorder, TEXT("AccentBorder"));
	CheckBinding(CategoryColorBorder, TEXT("CategoryColorBorder"));
	CheckBinding(NameText, TEXT("NameText"));
	CheckBinding(MetaText, TEXT("MetaText"));

	return bAllWidgetsBound;
}

void UPBCollectionEntryWidget::SetDisplayData(const FPBCollectionDisplayData& InDisplayData)
{
	DisplayData = InDisplayData;
	Refresh();
}

void UPBCollectionEntryWidget::EnsureEntryViewModel()
{
	if (!EntryViewModel)
	{
		EntryViewModel = NewObject<UPBCollectionEntryViewModel>(this);
	}

	if (EntryViewModel && !bIsEntryViewModelApplied)
	{
		bIsEntryViewModelApplied = ApplyViewModelToWidget();
	}
}

bool UPBCollectionEntryWidget::ApplyViewModelToWidget()
{
	if (!EntryViewModel)
	{
		return false;
	}

	UMVVMView* View = GetExtension<UMVVMView>();
	if (!View)
	{
		return false;
	}

	TScriptInterface<INotifyFieldValueChanged> ViewModelInterface(EntryViewModel);
	const bool bApplied = View->SetViewModelByClass(ViewModelInterface);
	if (!bApplied)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s: PBCollectionEntryViewModel을 WBP MVVM 컨텍스트에 주입하지 못했습니다."),
			*GetName());
	}
	return bApplied;
}

void UPBCollectionEntryWidget::Refresh()
{
	EnsureEntryViewModel();
	if (!EntryViewModel)
	{
		return;
	}

	EntryViewModel->SetDisplayData(DisplayData);

	BP_OnCollectionDisplayDataChanged(DisplayData);
}

void UPBCollectionEntryWidget::HandleClicked()
{
	OnEntryClicked.Broadcast(DisplayData.CollectionId);
}
