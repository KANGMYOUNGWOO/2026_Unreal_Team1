#include "PBBallStatusViewModel.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "PinBallLike/Actor/Ball/PBBallBase.h"
#include "PinBallLike/Actor/Common/Component/Resource/PBBaseResourceComponent.h"
#include "PinBallLike/Struct/Common/PBResourceTypes.h"
#include "PinBallLike/Subsystem/PBTableDataSubsystem.h"
#include "PinBallLike/Table/Ball/Struct/PBBallTableRow.h"

void UPBBallStatusViewModel::SetBall(APBBallBase* NewBall, UTexture2D* NewIconTexture)
{
	if (Ball == NewBall)
	{
		SetIconTexture(NewIconTexture);
		RefreshBallStatus();
		return;
	}

	UnbindBallEvents();
	Ball = NewBall;
	SetIconTexture(NewIconTexture);
	BindBallEvents();
	RefreshBallStatus();
}

void UPBBallStatusViewModel::SetIconTexture(UTexture2D* NewIconTexture)
{
	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, NewIconTexture);
}

void UPBBallStatusViewModel::ClearBall()
{
	UnbindBallEvents();
	Ball = nullptr;

	UE_MVVM_SET_PROPERTY_VALUE(IconTexture, nullptr);
	RefreshBallName();
	SetResourceValue(PBResourceNames::Health, 0.0f, 1.0f);
	SetResourceValue(PBResourceNames::Mana, 0.0f, 1.0f);
}

void UPBBallStatusViewModel::BeginDestroy()
{
	ClearBall();
	Super::BeginDestroy();
}

void UPBBallStatusViewModel::BindBallEvents()
{
	if (UPBBaseResourceComponent* ResourceComponent = GetBallResourceComponent())
	{
		ResourceCurrentChangedHandle = ResourceComponent->OnResourceCurrentChanged.AddUObject(
			this,
			&UPBBallStatusViewModel::HandleResourceCurrentChanged);
		ResourceChangedHandle = ResourceComponent->OnResourceChanged.AddUObject(
			this,
			&UPBBallStatusViewModel::HandleResourceChanged);
	}
}

void UPBBallStatusViewModel::UnbindBallEvents()
{
	if (UPBBaseResourceComponent* ResourceComponent = GetBallResourceComponent())
	{
		if (ResourceCurrentChangedHandle.IsValid())
		{
			ResourceComponent->OnResourceCurrentChanged.Remove(ResourceCurrentChangedHandle);
			ResourceCurrentChangedHandle.Reset();
		}

		if (ResourceChangedHandle.IsValid())
		{
			ResourceComponent->OnResourceChanged.Remove(ResourceChangedHandle);
			ResourceChangedHandle.Reset();
		}
	}
}

void UPBBallStatusViewModel::RefreshBallName()
{
	FText BallName = FText::GetEmpty();
	if (Ball && !Ball->GetBallId().IsNone())
	{
		UWorld* World = Ball->GetWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		const UPBTableDataSubsystem* TableDataSubsystem =
			GameInstance ? GameInstance->GetSubsystem<UPBTableDataSubsystem>() : nullptr;

		FPBBallTableRow BallRow;
		if (TableDataSubsystem && TableDataSubsystem->FindBallRow(Ball->GetBallId(), BallRow))
		{
			BallName = BallRow.DisplayName;
		}
		else
		{
			BallName = FText::FromName(Ball->GetBallId());
		}
	}

	UE_MVVM_SET_PROPERTY_VALUE(BallNameText, BallName);
}

void UPBBallStatusViewModel::RefreshBallStatus()
{
	RefreshBallName();
	RefreshResource(PBResourceNames::Health);
	RefreshResource(PBResourceNames::Mana);
}

void UPBBallStatusViewModel::RefreshResource(const FName ResourceName)
{
	const UPBBaseResourceComponent* ResourceComponent = GetBallResourceComponent();
	if (!ResourceComponent)
	{
		SetResourceValue(ResourceName, 0.0f, 1.0f);
		return;
	}

	SetResourceValue(
		ResourceName,
		ResourceComponent->GetResourceCurrent(ResourceName),
		ResourceComponent->GetResourceMax(ResourceName));
}

void UPBBallStatusViewModel::SetResourceValue(
	const FName ResourceName,
	const float CurrentValue,
	const float MaxValue)
{
	const float Percent = CalculateGaugePercent(CurrentValue, MaxValue);
	const FText ResourceText = FText::Format(
		NSLOCTEXT("BallStatusViewModel", "ResourceTextFormat", "{0} / {1}"),
		FMath::FloorToInt(CurrentValue),
		FMath::FloorToInt(MaxValue));

	if (ResourceName == PBResourceNames::Health)
	{
		UE_MVVM_SET_PROPERTY_VALUE(HPPercent, Percent);
		UE_MVVM_SET_PROPERTY_VALUE(HPText, ResourceText);
	}
	else if (ResourceName == PBResourceNames::Mana)
	{
		UE_MVVM_SET_PROPERTY_VALUE(ManaPercent, Percent);
		UE_MVVM_SET_PROPERTY_VALUE(ManaText, ResourceText);
	}
}

float UPBBallStatusViewModel::CalculateGaugePercent(const float CurrentValue, const float MaxValue)
{
	if (MaxValue <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f);
}

void UPBBallStatusViewModel::HandleResourceCurrentChanged(const FName ResourceName, const float CurrentValue)
{
	const UPBBaseResourceComponent* ResourceComponent = GetBallResourceComponent();
	const float MaxValue = ResourceComponent ? ResourceComponent->GetResourceMax(ResourceName) : 0.0f;
	SetResourceValue(ResourceName, CurrentValue, MaxValue);
}

void UPBBallStatusViewModel::HandleResourceChanged(const FName ResourceName)
{
	RefreshResource(ResourceName);
}

UPBBaseResourceComponent* UPBBallStatusViewModel::GetBallResourceComponent() const
{
	return Ball ? Ball->GetResourceComponent() : nullptr;
}
