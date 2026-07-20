#include "PBBallStatusWidget.h"

#include "PBBallStatusViewModel.h"
#include "Components/PanelWidget.h"
#include "Engine/AssetManager.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/StatusEffect/Component/PBStatusEffectComponent.h"
#include "PinBallLike/Actor/StatusEffect/PBBaseStatusEffect.h"
#include "PinBallLike/Actor/StatusEffect/UI/View/PBStatusEffectItemWidget.h"
#include "PinBallLike/Table/StatusEffect/DataAsset/PBStatusEffectDataAsset.h"
#include "PinBallLike/Table/StatusEffect/PBStatusEffectAssetIds.h"
#include "View/MVVMView.h"

void UPBBallStatusWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	EnsureStatusViewModel();
}

void UPBBallStatusWidget::SetBall(APBBallBase* NewBall, UTexture2D* NewIconTexture)
{
	if (Ball != NewBall)
	{
		UnbindStatusEffectComponent();
		Ball = NewBall;
		BindStatusEffectComponent();
	}

	EnsureStatusViewModel();
	if (StatusViewModel)
	{
		StatusViewModel->SetBall(NewBall, NewIconTexture);
	}

	RefreshStatusEffectItems();
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
	UnbindStatusEffectComponent();
	Ball = nullptr;

	if (StatusViewModel)
	{
		StatusViewModel->ClearBall();
	}

	RefreshStatusEffectItems();
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

void UPBBallStatusWidget::BindStatusEffectComponent()
{
	StatusEffectComponent = Ball ? Ball->GetStatusEffectComponent() : nullptr;
	if (!StatusEffectComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] StatusEffectComponent bind failed. Widget=%s Ball=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Ball));
		return;
	}

	StatusEffectComponent->OnStatusEffectApplied.AddUniqueDynamic(
		this,
		&UPBBallStatusWidget::HandleStatusEffectApplied);
	StatusEffectComponent->OnStatusEffectRemoved.AddUniqueDynamic(
		this,
		&UPBBallStatusWidget::HandleStatusEffectRemoved);
	StatusEffectComponent->OnStatusEffectStackChanged.AddUniqueDynamic(
		this,
		&UPBBallStatusWidget::HandleStatusEffectStackChanged);
	UE_LOG(LogTemp, Log, TEXT("[BallStatusWidget] StatusEffectComponent bound. Widget=%s Ball=%s Component=%s"),
		*GetNameSafe(this),
		*GetNameSafe(Ball),
		*GetNameSafe(StatusEffectComponent));
}

void UPBBallStatusWidget::UnbindStatusEffectComponent()
{
	if (!StatusEffectComponent)
	{
		return;
	}

	StatusEffectComponent->OnStatusEffectApplied.RemoveDynamic(
		this,
		&UPBBallStatusWidget::HandleStatusEffectApplied);
	StatusEffectComponent->OnStatusEffectRemoved.RemoveDynamic(
		this,
		&UPBBallStatusWidget::HandleStatusEffectRemoved);
	StatusEffectComponent->OnStatusEffectStackChanged.RemoveDynamic(
		this,
		&UPBBallStatusWidget::HandleStatusEffectStackChanged);
	StatusEffectComponent = nullptr;
}

void UPBBallStatusWidget::RefreshStatusEffectItems()
{
	StatusEffectItemWidgets.Reset();
	if (StatusEffectPanel)
	{
		StatusEffectPanel->ClearChildren();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] RefreshStatusEffectItems skipped child clear. StatusEffectPanel is null. Widget=%s Ball=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Ball));
	}

	if (!StatusEffectComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] RefreshStatusEffectItems skipped. StatusEffectComponent is null. Widget=%s Ball=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Ball));
		return;
	}

	TArray<UPBBaseStatusEffect*> ActiveStatusEffects;
	StatusEffectComponent->GetActiveStatusEffects(ActiveStatusEffects);
	for (const UPBBaseStatusEffect* StatusEffect : ActiveStatusEffects)
	{
		if (IsValid(StatusEffect))
		{
			UpsertStatusEffectItem(StatusEffect->GetStatusEffectId(), StatusEffect->GetStackCount());
		}
	}
}

void UPBBallStatusWidget::UpsertStatusEffectItem(const FName StatusEffectId, const int32 StackCount)
{
	if (StatusEffectId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] UpsertStatusEffectItem skipped. StatusEffectId is none. Widget=%s Ball=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Ball));
		return;
	}

	UPBStatusEffectItemWidget* StatusEffectItem = nullptr;
	if (TObjectPtr<UPBStatusEffectItemWidget>* ExistingItem = StatusEffectItemWidgets.Find(StatusEffectId))
	{
		StatusEffectItem = ExistingItem->Get();
	}

	if (!StatusEffectItem && StatusEffectPanel)
	{
		UClass* WidgetClass = StatusEffectItemWidgetClass.Get();
		if (!WidgetClass)
		{
			WidgetClass = UPBStatusEffectItemWidget::StaticClass();
			UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] StatusEffectItemWidgetClass is not set. Using C++ base widget. Widget=%s Ball=%s StatusEffectId=%s"),
				*GetNameSafe(this),
				*GetNameSafe(Ball),
				*StatusEffectId.ToString());
		}

		StatusEffectItem = CreateWidget<UPBStatusEffectItemWidget>(this, WidgetClass);
		if (StatusEffectItem)
		{
			StatusEffectPanel->AddChild(StatusEffectItem);
			StatusEffectItemWidgets.Add(StatusEffectId, StatusEffectItem);
			UE_LOG(LogTemp, Log, TEXT("[BallStatusWidget] StatusEffect item widget added. Widget=%s Ball=%s StatusEffectId=%s Stack=%d ItemWidget=%s ItemClass=%s"),
				*GetNameSafe(this),
				*GetNameSafe(Ball),
				*StatusEffectId.ToString(),
				StackCount,
				*GetNameSafe(StatusEffectItem),
				*GetNameSafe(WidgetClass));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] CreateWidget failed. Widget=%s Ball=%s StatusEffectId=%s ItemClass=%s"),
				*GetNameSafe(this),
				*GetNameSafe(Ball),
				*StatusEffectId.ToString(),
				*GetNameSafe(WidgetClass));
		}
	}
	else if (!StatusEffectItem && !StatusEffectPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] StatusEffect item not created. StatusEffectPanel is null. Widget=%s Ball=%s StatusEffectId=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Ball),
			*StatusEffectId.ToString());
	}

	if (!StatusEffectItem)
	{
		return;
	}

	FPBStatusEffectViewData ViewData;
	ViewData.StatusEffectId = StatusEffectId;
	ViewData.StackCount = StackCount;
	ViewData.Icon = ResolveStatusEffectIcon(StatusEffectId);
	if (!ViewData.Icon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] StatusEffect icon is null. Widget=%s Ball=%s StatusEffectId=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Ball),
			*StatusEffectId.ToString());
	}
	StatusEffectItem->InitializeStatusEffectItem(ViewData);
}

void UPBBallStatusWidget::RemoveStatusEffectItem(const FName StatusEffectId)
{
	TObjectPtr<UPBStatusEffectItemWidget> RemovedItem;
	if (!StatusEffectItemWidgets.RemoveAndCopyValue(StatusEffectId, RemovedItem))
	{
		return;
	}

	if (StatusEffectPanel && RemovedItem)
	{
		StatusEffectPanel->RemoveChild(RemovedItem);
	}
}

UTexture2D* UPBBallStatusWidget::ResolveStatusEffectIcon(const FName StatusEffectId) const
{
	if (StatusEffectId.IsNone())
	{
		return nullptr;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	const FPrimaryAssetId AssetId(PBStatusEffectAssetIds::Type::StatusEffectData, StatusEffectId);
	UPBStatusEffectDataAsset* StatusEffectDataAsset =
		Cast<UPBStatusEffectDataAsset>(AssetManager.GetPrimaryAssetObject(AssetId));
	if (!StatusEffectDataAsset)
	{
		const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
		StatusEffectDataAsset = Cast<UPBStatusEffectDataAsset>(AssetPath.TryLoad());
	}
	if (!StatusEffectDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BallStatusWidget] StatusEffect data asset not found. StatusEffectId=%s PrimaryAssetId=%s"),
			*StatusEffectId.ToString(),
			*AssetId.ToString());
		return nullptr;
	}

	return StatusEffectDataAsset->Icon.LoadSynchronous();
}

void UPBBallStatusWidget::HandleStatusEffectApplied(const FName StatusEffectId, const int32 StackCount)
{
	UE_LOG(LogTemp, Log, TEXT("[BallStatusWidget] HandleStatusEffectApplied. Widget=%s Ball=%s StatusEffectId=%s Stack=%d"),
		*GetNameSafe(this),
		*GetNameSafe(Ball),
		*StatusEffectId.ToString(),
		StackCount);
	UpsertStatusEffectItem(StatusEffectId, StackCount);
}

void UPBBallStatusWidget::HandleStatusEffectRemoved(const FName StatusEffectId, const int32 StackCount)
{
	(void)StackCount;
	RemoveStatusEffectItem(StatusEffectId);
}

void UPBBallStatusWidget::HandleStatusEffectStackChanged(
	const FName StatusEffectId,
	const int32 StackCount)
{
	UpsertStatusEffectItem(StatusEffectId, StackCount);
}
