#include "PBBossHitEffectComponent.h"

#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UPBBossHitEffectComponent::UPBBossHitEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPBBossHitEffectComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeMaterials();
}

void UPBBossHitEffectComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(ElapsedTime / FMath::Max(0.01f, HitFlashDuration), 0.0f, 1.0f);
	ApplyColor(Alpha);

	if (Alpha >= 1.0f)
	{
		SetComponentTickEnabled(false);
	}
}

void UPBBossHitEffectComponent::PlayHitEffect()
{
	if (DynamicMaterials.IsEmpty())
	{
		InitializeMaterials();
	}

	if (DynamicMaterials.IsEmpty())
	{
		return;
	}

	ElapsedTime = 0.0f;
	ApplyColor(0.0f);
	SetComponentTickEnabled(true);
}

void UPBBossHitEffectComponent::InitializeMaterials()
{
	DynamicMaterials.Reset();
	OriginalColors.Reset();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || ColorParameterName.IsNone())
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	OwnerActor->GetComponents<UMeshComponent>(MeshComponents);
	const FMaterialParameterInfo ParameterInfo(ColorParameterName);

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			UMaterialInterface* Material = MeshComponent->GetMaterial(MaterialIndex);
			FLinearColor OriginalColor;
			if (!Material || !Material->GetVectorParameterValue(ParameterInfo, OriginalColor))
			{
				continue;
			}

			UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(MaterialIndex, Material);
			if (!DynamicMaterial)
			{
				continue;
			}

			DynamicMaterials.Add(DynamicMaterial);
			OriginalColors.Add(OriginalColor);
		}
	}
}

void UPBBossHitEffectComponent::ApplyColor(float Alpha)
{
	const int32 MaterialCount = FMath::Min(DynamicMaterials.Num(), OriginalColors.Num());
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInstanceDynamic* DynamicMaterial = DynamicMaterials[MaterialIndex];
		if (!DynamicMaterial)
		{
			continue;
		}

		const FLinearColor Color = FMath::Lerp(HitColor, OriginalColors[MaterialIndex], Alpha);
		DynamicMaterial->SetVectorParameterValue(ColorParameterName, Color);
	}
}
