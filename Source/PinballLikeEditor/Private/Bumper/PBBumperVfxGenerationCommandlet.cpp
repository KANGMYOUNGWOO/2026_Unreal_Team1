#include "Bumper/PBBumperVfxGenerationCommandlet.h"

#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture.h"
#include "HAL/FileManager.h"
#include "IAssetTools.h"
#include "ImageCore.h"
#include "ImageUtils.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionDesaturation.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionParticleColor.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraEffectType.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemEditorData.h"
#include "NiagaraSystemEmitterState.h"
#include "ObjectTools.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Stateless/NiagaraStatelessEmitter.h"
#include "Stateless/NiagaraStatelessSpawnInfo.h"
#include "Stateless/Modules/NiagaraStatelessModule_AddVelocity.h"
#include "Stateless/Modules/NiagaraStatelessModule_Drag.h"
#include "Stateless/Modules/NiagaraStatelessModule_GravityForce.h"
#include "Stateless/Modules/NiagaraStatelessModule_InitializeParticle.h"
#include "Stateless/Modules/NiagaraStatelessModule_ScaleColor.h"
#include "Stateless/Modules/NiagaraStatelessModule_ScaleSpriteSizeBySpeed.h"
#include "Stateless/Modules/NiagaraStatelessModule_ShapeLocation.h"
#include "Stateless/Modules/NiagaraStatelessModule_SpriteRotationRate.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr TCHAR BumperVfxAssetFolder[] = TEXT("/Game/Blueprints/Bumper/Effect/VFX");
	constexpr TCHAR VfxMaterialFolder[] = TEXT("/Game/Blueprints/Bumper/Effect/VFX/Materials");
	constexpr TCHAR VfxParentMaterialName[] = TEXT("M_BumperVfxSprite");
	constexpr TCHAR EffectTypeAssetName[] = TEXT("NET_BumperGameplay");
	constexpr TCHAR BurstTemplatePath[] =
		TEXT("/Niagara/DefaultAssets/Templates/Systems/DirectionalBurstLightweight.DirectionalBurstLightweight");
	constexpr TCHAR LoopTemplatePath[] =
		TEXT("/Niagara/DefaultAssets/Templates/Systems/FountainLightweight.FountainLightweight");
	constexpr TCHAR GlowMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Glow.MI_Glow");
	constexpr TCHAR SoftMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Circle_Soft01.MI_Circle_Soft01");
	constexpr TCHAR RingMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Ring02.MI_Ring02");
	constexpr TCHAR LineMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Line03.MI_Line03");
	constexpr TCHAR ArrowMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Arrow03.MI_Arrow03");
	constexpr TCHAR FlareMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Flare01.MI_Flare01");
	constexpr TCHAR SpikeMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Spikes01.MI_Spikes01");
	constexpr TCHAR HeavySpikeMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Spikes02.MI_Spikes02");
	constexpr TCHAR SickleMaterialPath[] =
		TEXT("/Game/UnityParticle/Material/MI_Sickle06.MI_Sickle06");
	constexpr float GeneralSpriteScale = 1.40f;
	constexpr float GeneralSpawnRadiusScale = 1.30f;
	constexpr float BurstSpreadScale = 1.15f;

	struct FPBVfxGenerationPolicy
	{
		bool bUpdateExisting = false;
		bool bForceReplace = false;
	};

	enum class EPBVfxStyle : uint8
	{
		Attack,
		Groggy,
		Combo,
		Mana,
		Recovery,
		Shield,
		Speed,
		Strength,
		Vulnerability,
		Summon
	};

	struct FPBEffectVisualProfile
	{
		const TCHAR* EffectId;
		const TCHAR* Token;
		const TCHAR* LegacyActivationAsset;
		EPBVfxStyle Style;
		FLinearColor PrimaryColor;
		FLinearColor AccentColor;
		bool bHasDelivery;
		bool bHasImpact;
		bool bHasStatus;
	};

	struct FPBVfxRecipe
	{
		FString AssetName;
		EPBBumperVfxStage Stage = EPBBumperVfxStage::Activation;
		EPBVfxStyle Style = EPBVfxStyle::Attack;
		FLinearColor PrimaryColor = FLinearColor::White;
		FLinearColor AccentColor = FLinearColor::White;
		float LifetimeMin = 0.3f;
		float LifetimeMax = 0.6f;
		float SizeMin = 10.0f;
		float SizeMax = 22.0f;
		float SpeedMin = 180.0f;
		float SpeedMax = 520.0f;
		float ShapeRadius = 12.0f;
		float Drag = 1.5f;
		float GravityZ = 0.0f;
		int32 SpawnAmount = 24;
		float SpawnRate = 30.0f;
		const TCHAR* MaterialPath = GlowMaterialPath;
		bool bVelocityAligned = false;
	};

	struct FPBGeneratedVfxSet
	{
		FString EffectId;
		FString Activation;
		FString Delivery;
		FString Impact;
		FString Status;
	};

	const TArray<FPBEffectVisualProfile>& GetVisualProfiles()
	{
		static const TArray<FPBEffectVisualProfile> Profiles =
		{
			{TEXT("Effect_BloodOverdrive_01"), TEXT("BloodOverdrive"), nullptr,
				EPBVfxStyle::Strength, FLinearColor(1.00f, 0.03f, 0.08f), FLinearColor(1.00f, 0.42f, 0.10f), true, true, true},
			{TEXT("Effect_BossGroggy_02"), TEXT("BossGroggy"), TEXT("NS_Bumper_BossGroggy_01"),
				EPBVfxStyle::Groggy, FLinearColor(0.58f, 0.24f, 1.00f), FLinearColor(0.92f, 0.82f, 1.00f), true, true, false},
			{TEXT("Effect_ComboArc_01"), TEXT("ComboArc"), nullptr,
				EPBVfxStyle::Combo, FLinearColor(1.00f, 0.56f, 0.04f), FLinearColor(1.00f, 0.93f, 0.38f), true, true, true},
			{TEXT("Effect_ComboCashout_02"), TEXT("ComboCashout"), nullptr,
				EPBVfxStyle::Combo, FLinearColor(1.00f, 0.36f, 0.02f), FLinearColor(1.00f, 0.90f, 0.22f), true, true, false},
			{TEXT("Effect_ComboPulse_01"), TEXT("ComboPulse"), TEXT("NS_Bumper_ComboUp_01"),
				EPBVfxStyle::Combo, FLinearColor(1.00f, 0.72f, 0.02f), FLinearColor(1.00f, 1.00f, 0.62f), false, true, false},
			{TEXT("Effect_CounterShield_01"), TEXT("CounterShield"), nullptr,
				EPBVfxStyle::Shield, FLinearColor(0.04f, 0.76f, 1.00f), FLinearColor(1.00f, 0.42f, 0.12f), true, true, true},
			{TEXT("Effect_DirectStrike_02"), TEXT("DirectStrike"), TEXT("NS_Bumper_BossDamage_01"),
				EPBVfxStyle::Attack, FLinearColor(1.00f, 0.12f, 0.04f), FLinearColor(1.00f, 0.72f, 0.20f), true, true, false},
			{TEXT("Effect_GroggyHammer_02"), TEXT("GroggyHammer"), nullptr,
				EPBVfxStyle::Groggy, FLinearColor(0.44f, 0.12f, 0.94f), FLinearColor(0.88f, 0.74f, 1.00f), true, true, false},
			{TEXT("Effect_KineticShell_01"), TEXT("KineticShell"), nullptr,
				EPBVfxStyle::Attack, FLinearColor(1.00f, 0.28f, 0.02f), FLinearColor(1.00f, 0.82f, 0.28f), true, true, false},
			{TEXT("Effect_LaunchCharge_02"), TEXT("LaunchCharge"), nullptr,
				EPBVfxStyle::Summon, FLinearColor(1.00f, 0.46f, 0.04f), FLinearColor(0.30f, 0.92f, 1.00f), true, true, true},
			{TEXT("Effect_ManaReactor_01"), TEXT("ManaReactor"), nullptr,
				EPBVfxStyle::Mana, FLinearColor(0.08f, 0.40f, 1.00f), FLinearColor(0.20f, 1.00f, 0.96f), false, true, true},
			{TEXT("Effect_PartyMana_02"), TEXT("PartyMana"), TEXT("NS_Bumper_ManaCharge_01"),
				EPBVfxStyle::Mana, FLinearColor(0.02f, 0.62f, 1.00f), FLinearColor(0.34f, 1.00f, 0.92f), true, true, false},
			{TEXT("Effect_ReactiveRepair_01"), TEXT("ReactiveRepair"), nullptr,
				EPBVfxStyle::Recovery, FLinearColor(0.08f, 0.88f, 0.48f), FLinearColor(0.66f, 1.00f, 0.78f), false, true, true},
			{TEXT("Effect_RecoveryField_02"), TEXT("RecoveryField"), nullptr,
				EPBVfxStyle::Recovery, FLinearColor(0.04f, 0.74f, 0.28f), FLinearColor(0.68f, 1.00f, 0.52f), false, true, true},
			{TEXT("Effect_RepairPickup_02"), TEXT("RepairPickup"), nullptr,
				EPBVfxStyle::Recovery, FLinearColor(0.16f, 0.92f, 0.36f), FLinearColor(1.00f, 0.90f, 0.28f), false, true, true},
			{TEXT("Effect_Shield_02"), TEXT("ShieldCharge"), TEXT("NS_Bumper_Shield_01"),
				EPBVfxStyle::Shield, FLinearColor(0.02f, 0.66f, 1.00f), FLinearColor(0.72f, 0.98f, 1.00f), true, true, false},
			{TEXT("Effect_SpeedUp_01"), TEXT("SpeedUp"), TEXT("NS_Bumper_SpeedUp_01"),
				EPBVfxStyle::Speed, FLinearColor(0.04f, 0.44f, 1.00f), FLinearColor(0.76f, 0.96f, 1.00f), false, true, true},
			{TEXT("Effect_StrengthCharge_02"), TEXT("StrengthCharge"), nullptr,
				EPBVfxStyle::Strength, FLinearColor(1.00f, 0.24f, 0.02f), FLinearColor(1.00f, 0.78f, 0.18f), true, true, true},
			{TEXT("Effect_VelocityBoost_01"), TEXT("VelocityBoost"), TEXT("NS_Bumper_VelocityBoost_01"),
				EPBVfxStyle::Speed, FLinearColor(0.08f, 0.72f, 1.00f), FLinearColor(0.90f, 1.00f, 1.00f), false, true, false},
			{TEXT("Effect_VulnerabilityShell_01"), TEXT("VulnerabilityShell"), nullptr,
				EPBVfxStyle::Vulnerability, FLinearColor(0.92f, 0.04f, 0.76f), FLinearColor(0.72f, 0.34f, 1.00f), true, true, true}
		};
		return Profiles;
	}

	bool IsGateAreaProfile(const FPBEffectVisualProfile& Profile)
	{
		return FCString::Strcmp(Profile.EffectId, TEXT("Effect_SpeedUp_01")) == 0
			|| FCString::Strcmp(Profile.EffectId, TEXT("Effect_RecoveryField_02")) == 0
			|| FCString::Strcmp(Profile.EffectId, TEXT("Effect_ReactiveRepair_01")) == 0
			|| FCString::Strcmp(Profile.EffectId, TEXT("Effect_ManaReactor_01")) == 0;
	}

	const TCHAR* GetStyleToken(const EPBVfxStyle Style)
	{
		switch (Style)
		{
		case EPBVfxStyle::Attack:
			return TEXT("Attack");
		case EPBVfxStyle::Groggy:
			return TEXT("Groggy");
		case EPBVfxStyle::Combo:
			return TEXT("Combo");
		case EPBVfxStyle::Mana:
			return TEXT("Mana");
		case EPBVfxStyle::Recovery:
			return TEXT("Recovery");
		case EPBVfxStyle::Shield:
			return TEXT("Shield");
		case EPBVfxStyle::Speed:
			return TEXT("Speed");
		case EPBVfxStyle::Strength:
			return TEXT("Strength");
		case EPBVfxStyle::Vulnerability:
			return TEXT("Vulnerability");
		case EPBVfxStyle::Summon:
			return TEXT("Summon");
		default:
			return TEXT("Attack");
		}
	}

	const TCHAR* GetStageToken(const EPBBumperVfxStage Stage)
	{
		const TCHAR* StageName = TEXT("Activation");
		switch (Stage)
		{
		case EPBBumperVfxStage::Delivery:
			StageName = TEXT("Delivery");
			break;
		case EPBBumperVfxStage::Impact:
			StageName = TEXT("Impact");
			break;
		case EPBBumperVfxStage::Status:
			StageName = TEXT("Status");
			break;
		default:
			break;
		}
		return StageName;
	}

	FLinearColor GetStylePrimaryColor(const EPBVfxStyle Style)
	{
		switch (Style)
		{
		case EPBVfxStyle::Attack:
			return FLinearColor(1.00f, 0.18f, 0.03f);
		case EPBVfxStyle::Groggy:
			return FLinearColor(0.50f, 0.18f, 1.00f);
		case EPBVfxStyle::Combo:
			return FLinearColor(1.00f, 0.58f, 0.03f);
		case EPBVfxStyle::Mana:
			return FLinearColor(0.04f, 0.52f, 1.00f);
		case EPBVfxStyle::Recovery:
			return FLinearColor(0.08f, 0.84f, 0.38f);
		case EPBVfxStyle::Shield:
			return FLinearColor(0.03f, 0.70f, 1.00f);
		case EPBVfxStyle::Speed:
			return FLinearColor(0.06f, 0.62f, 1.00f);
		case EPBVfxStyle::Strength:
			return FLinearColor(1.00f, 0.16f, 0.03f);
		case EPBVfxStyle::Vulnerability:
			return FLinearColor(0.90f, 0.04f, 0.76f);
		case EPBVfxStyle::Summon:
			return FLinearColor(0.48f, 0.92f, 0.10f);
		default:
			return FLinearColor::White;
		}
	}

	FLinearColor GetStyleAccentColor(const EPBVfxStyle Style)
	{
		switch (Style)
		{
		case EPBVfxStyle::Attack:
			return FLinearColor(1.00f, 0.76f, 0.18f);
		case EPBVfxStyle::Groggy:
			return FLinearColor(0.90f, 0.78f, 1.00f);
		case EPBVfxStyle::Combo:
			return FLinearColor(1.00f, 0.94f, 0.30f);
		case EPBVfxStyle::Mana:
			return FLinearColor(0.24f, 1.00f, 0.94f);
		case EPBVfxStyle::Recovery:
			return FLinearColor(0.72f, 1.00f, 0.62f);
		case EPBVfxStyle::Shield:
			return FLinearColor(0.78f, 0.98f, 1.00f);
		case EPBVfxStyle::Speed:
			return FLinearColor(0.84f, 1.00f, 1.00f);
		case EPBVfxStyle::Strength:
			return FLinearColor(1.00f, 0.68f, 0.12f);
		case EPBVfxStyle::Vulnerability:
			return FLinearColor(0.70f, 0.34f, 1.00f);
		case EPBVfxStyle::Summon:
			return FLinearColor(0.30f, 1.00f, 0.82f);
		default:
			return FLinearColor::White;
		}
	}

	FString MakeLegacyStageAssetName(
		const FPBEffectVisualProfile& Profile,
		const EPBBumperVfxStage Stage)
	{
		if (Stage == EPBBumperVfxStage::Activation && Profile.LegacyActivationAsset)
		{
			return Profile.LegacyActivationAsset;
		}

		return FString::Printf(TEXT("NS_Bumper_%s_%s"), GetStageToken(Stage), Profile.Token);
	}

	FString MakeStageAssetName(
		const FPBEffectVisualProfile& Profile,
		const EPBBumperVfxStage Stage)
	{
		FString StyleToken = GetStyleToken(Profile.Style);
		if (Stage == EPBBumperVfxStage::Status && IsGateAreaProfile(Profile))
		{
			StyleToken += TEXT("Area");
		}

		return FString::Printf(TEXT("NS_Bumper_%s_%s"), GetStageToken(Stage), *StyleToken);
	}

	const TCHAR* GetCoreMaterialPath(const EPBVfxStyle Style, const EPBBumperVfxStage Stage)
	{
		if (Stage == EPBBumperVfxStage::Delivery)
		{
			return Style == EPBVfxStyle::Speed ? ArrowMaterialPath : LineMaterialPath;
		}

		switch (Style)
		{
		case EPBVfxStyle::Mana:
		case EPBVfxStyle::Recovery:
		case EPBVfxStyle::Shield:
			return SoftMaterialPath;
		case EPBVfxStyle::Vulnerability:
			return SickleMaterialPath;
		case EPBVfxStyle::Speed:
			return LineMaterialPath;
		default:
			return GlowMaterialPath;
		}
	}

	const TCHAR* GetSignatureMaterialPath(const EPBVfxStyle Style, const EPBBumperVfxStage Stage)
	{
		if (Stage == EPBBumperVfxStage::Delivery)
		{
			return Style == EPBVfxStyle::Speed ? ArrowMaterialPath : FlareMaterialPath;
		}

		switch (Style)
		{
		case EPBVfxStyle::Attack:
		case EPBVfxStyle::Strength:
			return SpikeMaterialPath;
		case EPBVfxStyle::Groggy:
			return HeavySpikeMaterialPath;
		case EPBVfxStyle::Speed:
			return ArrowMaterialPath;
		case EPBVfxStyle::Vulnerability:
			return SickleMaterialPath;
		case EPBVfxStyle::Combo:
		case EPBVfxStyle::Mana:
		case EPBVfxStyle::Recovery:
		case EPBVfxStyle::Shield:
		case EPBVfxStyle::Summon:
		default:
			return RingMaterialPath;
		}
	}

	FPBVfxRecipe MakeRecipe(const FPBEffectVisualProfile& Profile, const EPBBumperVfxStage Stage)
	{
		FPBVfxRecipe Recipe;
		Recipe.AssetName = MakeStageAssetName(Profile, Stage);
		Recipe.Stage = Stage;
		Recipe.Style = Profile.Style;
		Recipe.PrimaryColor = GetStylePrimaryColor(Profile.Style);
		Recipe.AccentColor = GetStyleAccentColor(Profile.Style);
		Recipe.MaterialPath = Stage == EPBBumperVfxStage::Status
			? GetCoreMaterialPath(Profile.Style, Stage)
			: GetSignatureMaterialPath(Profile.Style, Stage);
		Recipe.bVelocityAligned = Stage == EPBBumperVfxStage::Delivery || Profile.Style == EPBVfxStyle::Speed;

		switch (Stage)
		{
		case EPBBumperVfxStage::Activation:
			Recipe.LifetimeMin = 0.32f;
			Recipe.LifetimeMax = 0.62f;
			Recipe.SizeMin = 11.0f;
			Recipe.SizeMax = 25.0f;
			Recipe.SpeedMin = 240.0f;
			Recipe.SpeedMax = 620.0f;
			Recipe.SpawnAmount = 28;
			Recipe.Drag = 1.8f;
			break;
		case EPBBumperVfxStage::Delivery:
			Recipe.LifetimeMin = 0.18f;
			Recipe.LifetimeMax = 0.42f;
			Recipe.SizeMin = 7.0f;
			Recipe.SizeMax = 15.0f;
			Recipe.SpeedMin = 45.0f;
			Recipe.SpeedMax = 110.0f;
			Recipe.ShapeRadius = 9.0f;
			Recipe.SpawnRate = 56.0f;
			Recipe.Drag = 2.4f;
			break;
		case EPBBumperVfxStage::Impact:
			Recipe.LifetimeMin = 0.22f;
			Recipe.LifetimeMax = 0.52f;
			Recipe.SizeMin = 13.0f;
			Recipe.SizeMax = 31.0f;
			Recipe.SpeedMin = 300.0f;
			Recipe.SpeedMax = 760.0f;
			Recipe.SpawnAmount = 34;
			Recipe.Drag = 2.2f;
			break;
		case EPBBumperVfxStage::Status:
			Recipe.LifetimeMin = 0.75f;
			Recipe.LifetimeMax = 1.35f;
			Recipe.SizeMin = 11.0f;
			Recipe.SizeMax = 23.0f;
			Recipe.SpeedMin = 28.0f;
			Recipe.SpeedMax = 82.0f;
			Recipe.ShapeRadius = 72.0f;
			Recipe.SpawnRate = 24.0f;
			Recipe.Drag = 1.1f;
			break;
		}

		switch (Profile.Style)
		{
		case EPBVfxStyle::Attack:
			Recipe.SpeedMin *= 1.18f;
			Recipe.SpeedMax *= 1.18f;
			Recipe.GravityZ = -120.0f;
			break;
		case EPBVfxStyle::Groggy:
			Recipe.SizeMin *= 1.25f;
			Recipe.SizeMax *= 1.35f;
			Recipe.SpeedMin *= 0.72f;
			Recipe.SpeedMax *= 0.78f;
			Recipe.GravityZ = -260.0f;
			break;
		case EPBVfxStyle::Combo:
			Recipe.SpawnAmount += 8;
			Recipe.SpawnRate += 8.0f;
			Recipe.GravityZ = 65.0f;
			break;
		case EPBVfxStyle::Mana:
			Recipe.ShapeRadius *= 0.82f;
			Recipe.GravityZ = 90.0f;
			break;
		case EPBVfxStyle::Recovery:
			Recipe.SpeedMin *= 0.62f;
			Recipe.SpeedMax *= 0.68f;
			Recipe.ShapeRadius *= 1.20f;
			Recipe.GravityZ = 115.0f;
			break;
		case EPBVfxStyle::Shield:
			Recipe.SizeMin *= 0.85f;
			Recipe.SizeMax *= 0.90f;
			Recipe.ShapeRadius *= 1.35f;
			Recipe.GravityZ = 20.0f;
			break;
		case EPBVfxStyle::Speed:
			Recipe.SpeedMin *= 1.45f;
			Recipe.SpeedMax *= 1.55f;
			if (Stage == EPBBumperVfxStage::Status)
			{
				Recipe.SizeMin *= 1.10f;
				Recipe.SizeMax *= 1.15f;
				Recipe.ShapeRadius *= 1.15f;
				Recipe.SpawnRate += 6.0f;
			}
			else
			{
				Recipe.SizeMin *= 0.78f;
				Recipe.SizeMax *= 0.84f;
			}
			break;
		case EPBVfxStyle::Strength:
			Recipe.SpawnAmount += 5;
			Recipe.SpawnRate += 5.0f;
			Recipe.GravityZ = 75.0f;
			break;
		case EPBVfxStyle::Vulnerability:
			Recipe.ShapeRadius *= 1.45f;
			Recipe.SpeedMin *= 0.55f;
			Recipe.SpeedMax *= 0.65f;
			Recipe.GravityZ = 35.0f;
			break;
		case EPBVfxStyle::Summon:
			Recipe.ShapeRadius *= 1.28f;
			Recipe.SpawnAmount += 10;
			Recipe.SpawnRate += 10.0f;
			Recipe.GravityZ = 55.0f;
			break;
		}

		Recipe.SizeMin *= GeneralSpriteScale;
		Recipe.SizeMax *= GeneralSpriteScale;
		Recipe.ShapeRadius *= GeneralSpawnRadiusScale;
		if (Stage == EPBBumperVfxStage::Activation || Stage == EPBBumperVfxStage::Impact)
		{
			Recipe.SpeedMin *= BurstSpreadScale;
			Recipe.SpeedMax *= BurstSpreadScale;
		}

		if (Stage == EPBBumperVfxStage::Status && IsGateAreaProfile(Profile))
		{
			Recipe.ShapeRadius = PBGateFieldTuning::DefaultRadius;
			Recipe.SizeMin = 18.0f;
			Recipe.SizeMax = 34.0f;
			Recipe.SpeedMin = 12.0f;
			Recipe.SpeedMax = 30.0f;
			Recipe.SpawnRate = FMath::Max(Recipe.SpawnRate, 36.0f);
			Recipe.GravityZ = 0.0f;
		}

		return Recipe;
	}

	FString MakeBumperVfxObjectPath(const FString& PackagePath)
	{
		return FString::Printf(
			TEXT("%s.%s"),
			*PackagePath,
			*FPackageName::GetLongPackageAssetName(PackagePath));
	}

	UNiagaraStatelessEmitter* GetStatelessEmitter(UNiagaraSystem* System)
	{
		if (!IsValid(System))
		{
			return nullptr;
		}

		for (FNiagaraEmitterHandle& Handle : System->GetEmitterHandles())
		{
			if (UNiagaraStatelessEmitter* Emitter = Handle.GetStatelessEmitter())
			{
				return Emitter;
			}
		}
		return nullptr;
	}

	bool DeleteExistingSystemAsset(UNiagaraSystem* ExistingSystem)
	{
		if (!IsValid(ExistingSystem))
		{
			return true;
		}

		TArray<UObject*> AssetsToDelete;
		AssetsToDelete.Add(ExistingSystem);
		const int32 DeletedCount = ObjectTools::ForceDeleteObjects(AssetsToDelete, false);
		CollectGarbage(RF_NoFlags);
		return DeletedCount == 1;
	}

	bool HasExpectedTemplateModules(UNiagaraSystem* System, const EPBBumperVfxStage Stage)
	{
		const UNiagaraStatelessEmitter* Emitter = GetStatelessEmitter(System);
		if (!IsValid(Emitter))
		{
			return false;
		}

		return Stage == EPBBumperVfxStage::Delivery
			? Emitter->GetModule<UNiagaraStatelessModule_ScaleSpriteSizeBySpeed>() != nullptr
			: Emitter->GetModule<UNiagaraStatelessModule_ShapeLocation>() != nullptr;
	}

	UNiagaraSystem* LoadOrCreateSystem(
		IAssetTools& AssetTools,
		const FPBVfxRecipe& Recipe,
		const FPBVfxGenerationPolicy& Policy,
		bool& bOutShouldConfigure)
	{
		bOutShouldConfigure = false;
		const FString PackagePath = FString::Printf(TEXT("%s/%s"), BumperVfxAssetFolder, *Recipe.AssetName);
		UNiagaraSystem* ExistingSystem = FPackageName::DoesPackageExist(PackagePath)
			? LoadObject<UNiagaraSystem>(nullptr, *MakeBumperVfxObjectPath(PackagePath))
			: nullptr;
		if (IsValid(ExistingSystem) && HasExpectedTemplateModules(ExistingSystem, Recipe.Stage))
		{
			bOutShouldConfigure = Policy.bUpdateExisting;
			return ExistingSystem;
		}

		if (IsValid(ExistingSystem) && !Policy.bForceReplace)
		{
			UE_LOG(LogTemp, Error,
				TEXT("[BumperVfx] Existing system uses an unexpected template and was preserved: %s. Use -ForceReplace only after review."),
				*PackagePath);
			return nullptr;
		}

		if (IsValid(ExistingSystem) && !DeleteExistingSystemAsset(ExistingSystem))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to replace legacy system: %s"), *PackagePath);
			return nullptr;
		}

		const TCHAR* TemplatePath = Recipe.Stage == EPBBumperVfxStage::Status
			? LoopTemplatePath
			: BurstTemplatePath;
		UNiagaraSystem* TemplateSystem = LoadObject<UNiagaraSystem>(nullptr, TemplatePath);
		if (!IsValid(TemplateSystem))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Template is missing: %s"), TemplatePath);
			return nullptr;
		}

		UNiagaraSystem* NewSystem = Cast<UNiagaraSystem>(
			AssetTools.DuplicateAsset(Recipe.AssetName, BumperVfxAssetFolder, TemplateSystem));
		bOutShouldConfigure = IsValid(NewSystem);
		return NewSystem;
	}

	void SetVector2Range(
		FNiagaraDistributionRangeVector2& Distribution,
		const FVector2f& Min,
		const FVector2f& Max)
	{
		Distribution.Mode = ENiagaraDistributionMode::NonUniformRange;
		Distribution.Min = Min;
		Distribution.Max = Max;
		Distribution.ChannelConstantsAndRanges = {Min.X, Min.Y, Max.X, Max.Y};
		Distribution.ChannelCurves.Reset();
		Distribution.UpdateValuesFromDistribution();
	}

	void SetVector3Range(
		FNiagaraDistributionRangeVector3& Distribution,
		const FVector3f& Min,
		const FVector3f& Max)
	{
		Distribution.Mode = ENiagaraDistributionMode::NonUniformRange;
		Distribution.Min = Min;
		Distribution.Max = Max;
		Distribution.ChannelConstantsAndRanges =
		{
			Min.X, Min.Y, Min.Z,
			Max.X, Max.Y, Max.Z
		};
		Distribution.ChannelCurves.Reset();
		Distribution.UpdateValuesFromDistribution();
	}

	void SetColorGradient(
		FNiagaraDistributionColor& Distribution,
		TConstArrayView<FLinearColor> Colors,
		const uint8 LookupValueMode)
	{
		Distribution.Mode = ENiagaraDistributionMode::ColorGradient;
		Distribution.ChannelConstantsAndRanges.Reset(Colors.Num() * 4);
		Distribution.ChannelCurves.Reset(4);
		Distribution.ChannelCurves.AddDefaulted(4);

		const float Step = Colors.Num() > 1 ? 1.0f / static_cast<float>(Colors.Num() - 1) : 0.0f;
		for (int32 Index = 0; Index < Colors.Num(); ++Index)
		{
			const FLinearColor& Color = Colors[Index];
			const float Time = Step * static_cast<float>(Index);
			Distribution.ChannelConstantsAndRanges.Append({Color.R, Color.G, Color.B, Color.A});
			Distribution.ChannelCurves[0].AddKey(Time, Color.R);
			Distribution.ChannelCurves[1].AddKey(Time, Color.G);
			Distribution.ChannelCurves[2].AddKey(Time, Color.B);
			Distribution.ChannelCurves[3].AddKey(Time, Color.A);
		}

		Distribution.SetLookupValueMode(LookupValueMode);
		Distribution.UpdateValuesFromDistribution();
	}

	template <typename TModule>
	TModule* FindMutableModule(UNiagaraStatelessEmitter* Emitter)
	{
		return IsValid(Emitter)
			? const_cast<TModule*>(Emitter->GetModule<TModule>())
			: nullptr;
	}

	bool EnsureEmitterLayers(UNiagaraSystem* System)
	{
		if (!IsValid(System) || System->GetEmitterHandles().IsEmpty())
		{
			return false;
		}
		System->Modify();

		while (System->GetEmitterHandles().Num() > 3)
		{
			const FNiagaraEmitterHandle HandleToRemove = System->GetEmitterHandles().Last();
			System->RemoveEmitterHandle(HandleToRemove);
		}

		if (System->GetEmitterHandles().Num() == 1)
		{
			const FNiagaraEmitterHandle SourceHandle = System->GetEmitterHandles()[0];
			System->DuplicateEmitterHandle(SourceHandle, TEXT("Accent"));
		}
		if (System->GetEmitterHandles().Num() == 2)
		{
			const FNiagaraEmitterHandle SourceHandle = System->GetEmitterHandles()[0];
			System->DuplicateEmitterHandle(SourceHandle, TEXT("Signature"));
		}

		if (System->GetEmitterHandles().Num() != 3)
		{
			return false;
		}

		for (FNiagaraEmitterHandle& Handle : System->GetEmitterHandles())
		{
			if (!IsValid(Handle.GetStatelessEmitter()))
			{
				return false;
			}
			Handle.SetIsEnabled(true, *System, false);
		}

		UNiagaraSystemEditorData* EditorData = Cast<UNiagaraSystemEditorData>(
			System->GetEditorData());
		if (!IsValid(EditorData))
		{
			return false;
		}
		EditorData->Modify();
		EditorData->SynchronizeOverviewGraphWithSystem(*System);
		return true;
	}

	FPBVfxRecipe MakeAccentRecipe(const FPBVfxRecipe& BaseRecipe)
	{
		FPBVfxRecipe AccentRecipe = BaseRecipe;
		AccentRecipe.PrimaryColor = BaseRecipe.AccentColor;
		AccentRecipe.AccentColor = BaseRecipe.PrimaryColor;
		AccentRecipe.MaterialPath = BaseRecipe.Stage == EPBBumperVfxStage::Delivery
			? FlareMaterialPath
			: GlowMaterialPath;
		AccentRecipe.LifetimeMin *= 0.72f;
		AccentRecipe.LifetimeMax *= 0.82f;
		AccentRecipe.SizeMin *= 0.42f;
		AccentRecipe.SizeMax *= 0.55f;
		AccentRecipe.SpeedMin *= 1.28f;
		AccentRecipe.SpeedMax *= 1.42f;
		AccentRecipe.ShapeRadius *= 0.72f;
		AccentRecipe.Drag *= 0.78f;
		AccentRecipe.SpawnAmount = FMath::Max(5, FMath::RoundToInt(BaseRecipe.SpawnAmount * 0.45f));
		AccentRecipe.SpawnRate = FMath::Max(4.0f, BaseRecipe.SpawnRate * 0.42f);
		return AccentRecipe;
	}

	FPBVfxRecipe MakeSignatureRecipe(const FPBVfxRecipe& BaseRecipe)
	{
		FPBVfxRecipe SignatureRecipe = BaseRecipe;
		SignatureRecipe.PrimaryColor = BaseRecipe.AccentColor;
		SignatureRecipe.AccentColor = BaseRecipe.PrimaryColor;
		SignatureRecipe.MaterialPath = GetSignatureMaterialPath(BaseRecipe.Style, BaseRecipe.Stage);
		SignatureRecipe.LifetimeMin *= BaseRecipe.Stage == EPBBumperVfxStage::Status ? 1.10f : 0.78f;
		SignatureRecipe.LifetimeMax *= BaseRecipe.Stage == EPBBumperVfxStage::Status ? 1.22f : 0.92f;
		SignatureRecipe.SizeMin *= BaseRecipe.Stage == EPBBumperVfxStage::Delivery ? 0.62f : 1.85f;
		SignatureRecipe.SizeMax *= BaseRecipe.Stage == EPBBumperVfxStage::Delivery ? 0.82f : 2.35f;
		SignatureRecipe.SpeedMin *= BaseRecipe.Stage == EPBBumperVfxStage::Delivery ? 1.18f : 0.30f;
		SignatureRecipe.SpeedMax *= BaseRecipe.Stage == EPBBumperVfxStage::Delivery ? 1.35f : 0.42f;
		SignatureRecipe.ShapeRadius *= 1.08f;
		SignatureRecipe.Drag *= 1.30f;
		SignatureRecipe.SpawnAmount = BaseRecipe.Stage == EPBBumperVfxStage::Activation
			|| BaseRecipe.Stage == EPBBumperVfxStage::Impact
			? FMath::Clamp(FMath::RoundToInt(BaseRecipe.SpawnAmount * 0.16f), 3, 7)
			: BaseRecipe.SpawnAmount;
		SignatureRecipe.SpawnRate = FMath::Clamp(BaseRecipe.SpawnRate * 0.24f, 4.0f, 12.0f);
		return SignatureRecipe;
	}

	bool ConfigureSpawn(UNiagaraStatelessEmitter* Emitter, const FPBVfxRecipe& Recipe)
	{
		if (!IsValid(Emitter))
		{
			return false;
		}

		FNiagaraStatelessSpawnInfo* SpawnInfo = Emitter->GetSpawnInfoByIndex(0);
		if (!SpawnInfo)
		{
			SpawnInfo = &Emitter->AddSpawnInfo();
		}

		SpawnInfo->bEnabled = true;
		SpawnInfo->bSpawnProbabilityEnabled = false;
		const bool bOneShot = Recipe.Stage == EPBBumperVfxStage::Activation
			|| Recipe.Stage == EPBBumperVfxStage::Impact;
		SpawnInfo->bLoopCountLimitEnabled = bOneShot;
		SpawnInfo->LoopCountLimit.InitConstant(1);
		if (bOneShot)
		{
			SpawnInfo->Type = ENiagaraStatelessSpawnInfoType::Burst;
			SpawnInfo->SpawnTime = 0.0f;
			SpawnInfo->Amount.InitConstant(Recipe.SpawnAmount);
		}
		else
		{
			SpawnInfo->Type = ENiagaraStatelessSpawnInfoType::Rate;
			SpawnInfo->Rate.InitConstant(Recipe.SpawnRate);
		}
		return true;
	}

	bool ConfigureEmitterLifecycle(UNiagaraStatelessEmitter* Emitter, const FPBVfxRecipe& Recipe)
	{
		if (!IsValid(Emitter))
		{
			return false;
		}

		const FStructProperty* EmitterStateProperty = FindFProperty<FStructProperty>(
			UNiagaraStatelessEmitter::StaticClass(),
			TEXT("EmitterState"));
		if (!EmitterStateProperty
			|| EmitterStateProperty->Struct != FNiagaraEmitterStateData::StaticStruct())
		{
			return false;
		}

		FNiagaraEmitterStateData* EmitterState =
			EmitterStateProperty->ContainerPtrToValuePtr<FNiagaraEmitterStateData>(Emitter);
		if (!EmitterState)
		{
			return false;
		}

		const bool bOneShot = Recipe.Stage == EPBBumperVfxStage::Activation
			|| Recipe.Stage == EPBBumperVfxStage::Impact;
		Emitter->Modify();
		EmitterState->InactiveResponse = ENiagaraEmitterInactiveResponse::Complete;
		EmitterState->LoopBehavior = bOneShot
			? ENiagaraLoopBehavior::Once
			: ENiagaraLoopBehavior::Infinite;
		EmitterState->LoopCount = 1;
		EmitterState->LoopDurationMode = ENiagaraLoopDurationMode::Fixed;
		EmitterState->LoopDuration.InitConstant(
			bOneShot ? FMath::Max(Recipe.LifetimeMax + 0.1f, 0.2f) : 1.0f);
		EmitterState->bLoopDelayEnabled = false;
		EmitterState->bRecalculateDurationEachLoop = false;
		EmitterState->bDelayFirstLoopOnly = false;
		return true;
	}

	UTexture* ResolveSourceTexture(const TCHAR* SourceMaterialPath)
	{
		UMaterialInterface* SourceMaterial = LoadObject<UMaterialInterface>(nullptr, SourceMaterialPath);
		if (!IsValid(SourceMaterial))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Source material is missing: %s"), SourceMaterialPath);
			return nullptr;
		}

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> ParameterIds;
		SourceMaterial->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
		for (const FMaterialParameterInfo& ParameterInfo : ParameterInfos)
		{
			UTexture* Texture = nullptr;
			if (SourceMaterial->GetTextureParameterValue(
				FHashedMaterialParameterInfo(ParameterInfo),
				Texture)
				&& IsValid(Texture))
			{
				return Texture;
			}
		}

		UE_LOG(LogTemp, Error,
			TEXT("[BumperVfx] Source material has no usable texture parameter: %s"),
			SourceMaterialPath);
		return nullptr;
	}

	bool ShouldUseAlphaMask(UTexture* Texture)
	{
		UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
		FImage SourceImage;
		if (!IsValid(Texture2D) || !FImageUtils::GetTexture2DSourceImage(Texture2D, SourceImage))
		{
			return false;
		}

		SourceImage.ChangeFormat(ERawImageFormat::BGRA8, EGammaSpace::Linear);
		const TArrayView64<FColor> Pixels = SourceImage.AsBGRA8();
		if (Pixels.IsEmpty())
		{
			return false;
		}

		uint8 MinAlpha = MAX_uint8;
		uint8 MaxAlpha = 0;
		const int64 SampleStride = FMath::Max<int64>(1, Pixels.Num() / 4096);
		for (int64 PixelIndex = 0; PixelIndex < Pixels.Num(); PixelIndex += SampleStride)
		{
			MinAlpha = FMath::Min(MinAlpha, Pixels[PixelIndex].A);
			MaxAlpha = FMath::Max(MaxAlpha, Pixels[PixelIndex].A);
		}

		return int32(MaxAlpha) - int32(MinAlpha) >= 16;
	}

	UMaterial* CreateOrUpdateNiagaraSpriteParentMaterial(UTexture* DefaultTexture)
	{
		static TWeakObjectPtr<UMaterial> CachedMaterial;
		if (CachedMaterial.IsValid())
		{
			return CachedMaterial.Get();
		}
		if (!IsValid(DefaultTexture))
		{
			return nullptr;
		}

		const FString PackagePath = FString::Printf(
			TEXT("%s/%s"),
			VfxMaterialFolder,
			VfxParentMaterialName);
		UMaterial* Material = FPackageName::DoesPackageExist(PackagePath)
			? LoadObject<UMaterial>(nullptr, *MakeBumperVfxObjectPath(PackagePath))
			: nullptr;
		if (!IsValid(Material))
		{
			UPackage* Package = CreatePackage(*PackagePath);
			Material = NewObject<UMaterial>(
				Package,
				FName(VfxParentMaterialName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!IsValid(Material))
			{
				return nullptr;
			}
			FAssetRegistryModule::AssetCreated(Material);
		}

		Material->Modify();
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Additive;
		Material->SetShadingModel(MSM_Unlit);
		Material->TwoSided = true;
		Material->bDisableDepthTest = false;
		UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);

		auto* TextureSample = Cast<UMaterialExpressionTextureSampleParameter2D>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionTextureSampleParameter2D::StaticClass(),
				-720,
				-120));
		auto* ParticleColor = Cast<UMaterialExpressionParticleColor>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionParticleColor::StaticClass(),
				-720,
				100));
		auto* RgbMask = Cast<UMaterialExpressionDesaturation>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionDesaturation::StaticClass(),
				-430,
				-190));
		auto* UseAlphaMask = Cast<UMaterialExpressionScalarParameter>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionScalarParameter::StaticClass(),
				-430,
				100));
		auto* SelectMask = Cast<UMaterialExpressionLinearInterpolate>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionLinearInterpolate::StaticClass(),
				-180,
				-150));
		auto* TintMultiply = Cast<UMaterialExpressionMultiply>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionMultiply::StaticClass(),
				70,
				-80));
		auto* FadeMultiply = Cast<UMaterialExpressionMultiply>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionMultiply::StaticClass(),
				300,
				-40));
		auto* EmissiveBoost = Cast<UMaterialExpressionMultiply>(
			UMaterialEditingLibrary::CreateMaterialExpression(
				Material,
				UMaterialExpressionMultiply::StaticClass(),
				540,
				-40));
		if (!TextureSample || !ParticleColor || !RgbMask || !UseAlphaMask || !SelectMask
			|| !TintMultiply || !FadeMultiply || !EmissiveBoost)
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to build the shared sprite material graph."));
			return nullptr;
		}

		TextureSample->ParameterName = TEXT("Texture");
		TextureSample->Texture = DefaultTexture;
		UseAlphaMask->ParameterName = TEXT("UseAlphaMask");
		UseAlphaMask->DefaultValue = 0.0f;
		UseAlphaMask->SliderMin = 0.0f;
		UseAlphaMask->SliderMax = 1.0f;
		EmissiveBoost->ConstB = 3.5f;
		RgbMask->Input.Connect(0, TextureSample);
		SelectMask->A.Connect(0, RgbMask);
		SelectMask->B.Connect(4, TextureSample);
		SelectMask->Alpha.Connect(0, UseAlphaMask);
		TintMultiply->A.Connect(0, SelectMask);
		TintMultiply->B.Connect(0, ParticleColor);
		FadeMultiply->A.Connect(0, TintMultiply);
		FadeMultiply->B.Connect(4, ParticleColor);
		EmissiveBoost->A.Connect(0, FadeMultiply);
		const bool bGraphConnected = UMaterialEditingLibrary::ConnectMaterialProperty(
				EmissiveBoost,
				TEXT(""),
				MP_EmissiveColor);
		if (!bGraphConnected)
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to connect the shared sprite material graph."));
			return nullptr;
		}

		bool bNeedsRecompile = false;
		if (!UMaterialEditingLibrary::SetMaterialUsage(
			Material,
			MATUSAGE_NiagaraSprites,
			bNeedsRecompile))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to enable Niagara sprite usage."));
			return nullptr;
		}
		UMaterialEditingLibrary::RecompileMaterial(Material);
		Material->MarkPackageDirty();

		UPackage* Package = Material->GetOutermost();
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(),
			FPackageName::GetAssetPackageExtension());
		if (!UPackage::SavePackage(Package, Material, *Filename, SaveArgs))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to save shared Niagara material."));
			return nullptr;
		}

		CachedMaterial = Material;
		return Material;
	}

	UMaterialInstanceConstant* CreateOrUpdateNiagaraSpriteMaterial(const TCHAR* SourceMaterialPath)
	{
		static TMap<FString, TWeakObjectPtr<UMaterialInstanceConstant>> MaterialCache;
		if (const TWeakObjectPtr<UMaterialInstanceConstant>* CachedMaterial =
			MaterialCache.Find(SourceMaterialPath))
		{
			if (CachedMaterial->IsValid())
			{
				return CachedMaterial->Get();
			}
		}

		UTexture* SourceTexture = ResolveSourceTexture(SourceMaterialPath);
		UMaterial* NiagaraParent = CreateOrUpdateNiagaraSpriteParentMaterial(SourceTexture);
		if (!IsValid(NiagaraParent) || !IsValid(SourceTexture))
		{
			return nullptr;
		}

		FString SourceAssetName = FPackageName::GetLongPackageAssetName(
			FPackageName::ObjectPathToPackageName(FString(SourceMaterialPath)));
		SourceAssetName.RemoveFromStart(TEXT("MI_"));
		const FString AssetName = FString::Printf(TEXT("MI_BumperVfx_%s"), *SourceAssetName);
		const FString PackagePath = FString::Printf(TEXT("%s/%s"), VfxMaterialFolder, *AssetName);
		UMaterialInstanceConstant* Material = FPackageName::DoesPackageExist(PackagePath)
			? LoadObject<UMaterialInstanceConstant>(nullptr, *MakeBumperVfxObjectPath(PackagePath))
			: nullptr;
		if (!IsValid(Material))
		{
			UPackage* Package = CreatePackage(*PackagePath);
			Material = NewObject<UMaterialInstanceConstant>(
				Package,
				FName(*AssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!IsValid(Material))
			{
				return nullptr;
			}
			FAssetRegistryModule::AssetCreated(Material);
		}

		Material->Modify();
		Material->SetParentEditorOnly(NiagaraParent, false);
		Material->ClearParameterValuesEditorOnly();
		Material->SetTextureParameterValueEditorOnly(
			FMaterialParameterInfo(TEXT("Texture")),
			SourceTexture);
		const bool bUseAlphaMask = ShouldUseAlphaMask(SourceTexture);
		Material->SetScalarParameterValueEditorOnly(
			FMaterialParameterInfo(TEXT("UseAlphaMask")),
			bUseAlphaMask ? 1.0f : 0.0f);
		UE_LOG(LogTemp, Display,
			TEXT("[BumperVfx] Material %s uses %s mask from %s"),
			*AssetName,
			bUseAlphaMask ? TEXT("Alpha") : TEXT("RGB luminance"),
			*SourceTexture->GetName());
		Material->PostEditChange();
		Material->MarkPackageDirty();

		UPackage* Package = Material->GetOutermost();
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(),
			FPackageName::GetAssetPackageExtension());
		if (!UPackage::SavePackage(Package, Material, *Filename, SaveArgs))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to save Niagara material: %s"), *AssetName);
			return nullptr;
		}

		MaterialCache.Add(SourceMaterialPath, Material);
		return Material;
	}

	bool ConfigureRenderers(UNiagaraStatelessEmitter* Emitter, const FPBVfxRecipe& Recipe)
	{
		if (!IsValid(Emitter))
		{
			return false;
		}

		UMaterialInterface* Material = CreateOrUpdateNiagaraSpriteMaterial(Recipe.MaterialPath);
		if (!IsValid(Material))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Material is missing: %s"), Recipe.MaterialPath);
			return false;
		}

		bool bConfiguredSpriteRenderer = false;
		for (UNiagaraRendererProperties* Renderer : Emitter->GetRenderers())
		{
			UNiagaraSpriteRendererProperties* SpriteRenderer = Cast<UNiagaraSpriteRendererProperties>(Renderer);
			if (!IsValid(SpriteRenderer))
			{
				continue;
			}

			SpriteRenderer->Modify();
			SpriteRenderer->SetIsEnabled(true);
			SpriteRenderer->Material = Material;
			SpriteRenderer->Alignment = Recipe.bVelocityAligned
				? ENiagaraSpriteAlignment::VelocityAligned
				: ENiagaraSpriteAlignment::Unaligned;
			SpriteRenderer->FacingMode = ENiagaraSpriteFacingMode::FaceCamera;
			SpriteRenderer->SortMode = ENiagaraSortMode::ViewDistance;
			bConfiguredSpriteRenderer = true;
		}
		return bConfiguredSpriteRenderer;
	}

	bool ConfigureModules(UNiagaraStatelessEmitter* Emitter, const FPBVfxRecipe& Recipe)
	{
		UNiagaraStatelessModule_InitializeParticle* Initialize =
			FindMutableModule<UNiagaraStatelessModule_InitializeParticle>(Emitter);
		UNiagaraStatelessModule_AddVelocity* Velocity =
			FindMutableModule<UNiagaraStatelessModule_AddVelocity>(Emitter);
		if (!Initialize || !Velocity)
		{
			return false;
		}

		Initialize->Modify();
		Initialize->SetIsModuleEnabled(true);
		Initialize->LifetimeDistribution.InitRange(Recipe.LifetimeMin, Recipe.LifetimeMax);
		FLinearColor DimColor = Recipe.PrimaryColor * 0.72f;
		DimColor.A = Recipe.PrimaryColor.A;
		FLinearColor BrightColor = Recipe.PrimaryColor * 1.65f;
		BrightColor.A = Recipe.PrimaryColor.A;
		const TArray<FLinearColor> InitialColors = {DimColor, BrightColor};
		SetColorGradient(
			Initialize->ColorDistribution,
			InitialColors,
			uint8(ENiagaraDistributionInitialLookupValueMode::Random));
		SetVector2Range(
			Initialize->SpriteSizeDistribution,
			FVector2f(Recipe.SizeMin),
			FVector2f(Recipe.SizeMax));

		Velocity->Modify();
		Velocity->SetIsModuleEnabled(true);
		if (Recipe.Stage == EPBBumperVfxStage::Activation || Recipe.Stage == EPBBumperVfxStage::Impact)
		{
			Velocity->VelocityType = ENSM_VelocityType::InCone;
			Velocity->ConeVelocityDistribution.InitRange(Recipe.SpeedMin, Recipe.SpeedMax);
			Velocity->ConeRotation = Recipe.Stage == EPBBumperVfxStage::Impact
				? FRotator(-90.0f, 0.0f, 0.0f)
				: FRotator::ZeroRotator;
			Velocity->ConeAngle = Recipe.Style == EPBVfxStyle::Speed ? 24.0f : 92.0f;
			Velocity->InnerCone = Recipe.Style == EPBVfxStyle::Shield ? 62.0f : 8.0f;
			Velocity->CoordinateSpace = ENiagaraCoordinateSpace::World;
		}
		else
		{
			Velocity->VelocityType = ENSM_VelocityType::Linear;
			SetVector3Range(
				Velocity->LinearVelocityDistribution,
				FVector3f(-Recipe.SpeedMin * 0.35f, -Recipe.SpeedMin * 0.25f, Recipe.SpeedMin),
				FVector3f(Recipe.SpeedMin * 0.35f, Recipe.SpeedMin * 0.25f, Recipe.SpeedMax));
			Velocity->LinearVelocityScale.InitConstant(1.0f);
			Velocity->CoordinateSpace = ENiagaraCoordinateSpace::Local;
		}

		if (UNiagaraStatelessModule_ScaleSpriteSizeBySpeed* ScaleBySpeed =
			FindMutableModule<UNiagaraStatelessModule_ScaleSpriteSizeBySpeed>(Emitter))
		{
			ScaleBySpeed->Modify();
			ScaleBySpeed->SetIsModuleEnabled(Recipe.bVelocityAligned);
			if (Recipe.bVelocityAligned)
			{
				ScaleBySpeed->VelocityThreshold.InitConstant(FMath::Max(Recipe.SpeedMax, 1.0f));
				ScaleBySpeed->MinScaleFactor.InitConstant(FVector2f(0.72f, 1.0f));
				ScaleBySpeed->MaxScaleFactor.InitConstant(FVector2f(0.48f, 3.2f));
			}
		}

		if (UNiagaraStatelessModule_Drag* Drag = FindMutableModule<UNiagaraStatelessModule_Drag>(Emitter))
		{
			Drag->Modify();
			Drag->DragDistribution.InitConstant(Recipe.Drag);
			Drag->SetIsModuleEnabled(true);
		}

		if (UNiagaraStatelessModule_GravityForce* Gravity =
			FindMutableModule<UNiagaraStatelessModule_GravityForce>(Emitter))
		{
			Gravity->Modify();
			Gravity->GravityDistribution.InitConstant(FVector3f(0.0f, 0.0f, Recipe.GravityZ));
			Gravity->SetIsModuleEnabled(!FMath::IsNearlyZero(Recipe.GravityZ));
		}

		if (UNiagaraStatelessModule_ShapeLocation* Shape =
			FindMutableModule<UNiagaraStatelessModule_ShapeLocation>(Emitter))
		{
			Shape->Modify();
			Shape->SetIsModuleEnabled(true);
			Shape->CoordinateSpace = ENiagaraCoordinateSpace::Local;
			const bool bUsesRing = Recipe.Style == EPBVfxStyle::Combo
				|| Recipe.Style == EPBVfxStyle::Mana
				|| Recipe.Style == EPBVfxStyle::Shield
				|| Recipe.Style == EPBVfxStyle::Vulnerability
				|| Recipe.Style == EPBVfxStyle::Summon;
			if (Recipe.Stage == EPBBumperVfxStage::Delivery || !bUsesRing)
			{
				Shape->ShapePrimitive = ENSM_ShapePrimitive::Sphere;
				Shape->SphereRadius.InitConstant(Recipe.ShapeRadius);
			}
			else
			{
				Shape->ShapePrimitive = ENSM_ShapePrimitive::Ring;
				Shape->RingRadius.InitConstant(Recipe.ShapeRadius);
				Shape->DiscCoverage.InitConstant(Recipe.Stage == EPBBumperVfxStage::Status ? 0.08f : 0.28f);
				Shape->RingUDistribution.InitRange(0.0f, 1.0f);
			}
		}

		if (UNiagaraStatelessModule_ScaleColor* ScaleColor =
			FindMutableModule<UNiagaraStatelessModule_ScaleColor>(Emitter))
		{
			ScaleColor->Modify();
			const TArray<FLinearColor> ScaleColors =
			{
				FLinearColor(0.0f, 0.0f, 0.0f, 1.0f),
				FLinearColor::White,
				FLinearColor(0.0f, 0.0f, 0.0f, 1.0f)
			};
			SetColorGradient(
				ScaleColor->ScaleDistribution,
				ScaleColors,
				uint8(ENiagaraDistributionLookupValueMode::ParticlesNormalizedAge));
			ScaleColor->SetIsModuleEnabled(true);
		}

		if (UNiagaraStatelessModule_SpriteRotationRate* Rotation =
			FindMutableModule<UNiagaraStatelessModule_SpriteRotationRate>(Emitter))
		{
			Rotation->Modify();
			const float RotationRate = Recipe.Style == EPBVfxStyle::Speed ? 260.0f : 130.0f;
			Rotation->RotationRateDistribution.InitRange(-RotationRate, RotationRate);
			Rotation->SetIsModuleEnabled(true);
		}

		return true;
	}

	bool SaveSystem(UNiagaraSystem* System)
	{
		if (!IsValid(System))
		{
			return false;
		}

		System->Modify();
		System->PostEditChange();
		System->InvalidateCachedData();
		System->RequestCompile(true);
		System->PollForCompilationComplete(true);
		System->MarkPackageDirty();

		UPackage* Package = System->GetOutermost();
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(),
			FPackageName::GetAssetPackageExtension());
		return UPackage::SavePackage(Package, System, *Filename, SaveArgs);
	}

	UNiagaraEffectType* CreateOrUpdateEffectType(const FPBVfxGenerationPolicy& Policy)
	{
		const FString PackagePath = FString::Printf(TEXT("%s/%s"), BumperVfxAssetFolder, EffectTypeAssetName);
		UNiagaraEffectType* EffectType = FPackageName::DoesPackageExist(PackagePath)
			? LoadObject<UNiagaraEffectType>(nullptr, *MakeBumperVfxObjectPath(PackagePath))
			: nullptr;
		const bool bWasCreated = !IsValid(EffectType);
		if (bWasCreated)
		{
			UPackage* Package = CreatePackage(*PackagePath);
			EffectType = NewObject<UNiagaraEffectType>(
				Package,
				FName(EffectTypeAssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!IsValid(EffectType))
			{
				return nullptr;
			}
			FAssetRegistryModule::AssetCreated(EffectType);
		}
		else if (!Policy.bUpdateExisting)
		{
			return EffectType;
		}

		EffectType->Modify();
		EffectType->bAllowCullingForLocalPlayers = false;
		EffectType->UpdateFrequency = ENiagaraScalabilityUpdateFrequency::Low;
		EffectType->CullReaction = ENiagaraCullReaction::DeactivateImmediateResume;
		if (!IsValid(EffectType->SignificanceHandler)
			|| !EffectType->SignificanceHandler->IsA<UNiagaraSignificanceHandlerDistance>())
		{
			EffectType->SignificanceHandler = NewObject<UNiagaraSignificanceHandlerDistance>(
				EffectType,
				TEXT("DistanceSignificance"),
				RF_Transactional);
		}

		EffectType->SystemScalabilitySettings.Settings.SetNum(1);
		FNiagaraSystemScalabilitySettings& Settings =
			EffectType->SystemScalabilitySettings.Settings[0];
		Settings.bCullByDistance = true;
		Settings.MaxDistance = 10000.0f;
		Settings.bCullMaxInstanceCount = true;
		Settings.MaxInstances = 160;
		Settings.bCullPerSystemMaxInstanceCount = true;
		Settings.MaxSystemInstances = 32;
		Settings.CullProxyMode = ENiagaraCullProxyMode::None;
		Settings.VisibilityCulling.bCullWhenNotRendered = true;
		Settings.VisibilityCulling.MaxTimeWithoutRender = 1.0f;
		Settings.VisibilityCulling.bCullByViewFrustum = false;

		EffectType->MarkPackageDirty();
		UPackage* Package = EffectType->GetOutermost();
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(),
			FPackageName::GetAssetPackageExtension());
		return UPackage::SavePackage(Package, EffectType, *Filename, SaveArgs)
			? EffectType
			: nullptr;
	}

	bool GenerateSystem(
		IAssetTools& AssetTools,
		UNiagaraEffectType* EffectType,
		const FPBVfxRecipe& Recipe,
		const FPBVfxGenerationPolicy& Policy,
		bool& bOutGenerated)
	{
		bOutGenerated = false;
		bool bShouldConfigure = false;
		UNiagaraSystem* System = LoadOrCreateSystem(AssetTools, Recipe, Policy, bShouldConfigure);
		if (!IsValid(System))
		{
			return false;
		}
		if (!bShouldConfigure)
		{
			UE_LOG(LogTemp, Display, TEXT("[BumperVfx] Existing system preserved: %s"), *Recipe.AssetName);
			return true;
		}
		if (!EnsureEmitterLayers(System))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Stateless layers are invalid: %s"), *Recipe.AssetName);
			return false;
		}

		UNiagaraStatelessEmitter* PrimaryEmitter =
			System->GetEmitterHandles()[0].GetStatelessEmitter();
		UNiagaraStatelessEmitter* AccentEmitter =
			System->GetEmitterHandles()[1].GetStatelessEmitter();
		UNiagaraStatelessEmitter* SignatureEmitter =
			System->GetEmitterHandles()[2].GetStatelessEmitter();
		const FPBVfxRecipe AccentRecipe = MakeAccentRecipe(Recipe);
		const FPBVfxRecipe SignatureRecipe = MakeSignatureRecipe(Recipe);
		PrimaryEmitter->Modify();
		AccentEmitter->Modify();
		SignatureEmitter->Modify();
		if (!ConfigureEmitterLifecycle(PrimaryEmitter, Recipe)
			|| !ConfigureSpawn(PrimaryEmitter, Recipe)
			|| !ConfigureRenderers(PrimaryEmitter, Recipe)
			|| !ConfigureModules(PrimaryEmitter, Recipe)
			|| !ConfigureEmitterLifecycle(AccentEmitter, AccentRecipe)
			|| !ConfigureSpawn(AccentEmitter, AccentRecipe)
			|| !ConfigureRenderers(AccentEmitter, AccentRecipe)
			|| !ConfigureModules(AccentEmitter, AccentRecipe)
			|| !ConfigureEmitterLifecycle(SignatureEmitter, SignatureRecipe)
			|| !ConfigureSpawn(SignatureEmitter, SignatureRecipe)
			|| !ConfigureRenderers(SignatureEmitter, SignatureRecipe)
			|| !ConfigureModules(SignatureEmitter, SignatureRecipe))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to configure modules: %s"), *Recipe.AssetName);
			return false;
		}
		System->SetEffectType(EffectType);

		if (!SaveSystem(System))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to save system: %s"), *Recipe.AssetName);
			return false;
		}

		UE_LOG(LogTemp, Display,
			TEXT("[BumperVfx] Generated %s Stage=%d Lifetime=%.2f-%.2f Size=%.1f-%.1f"),
			*Recipe.AssetName,
			static_cast<int32>(Recipe.Stage),
			Recipe.LifetimeMin,
			Recipe.LifetimeMax,
			Recipe.SizeMin,
			Recipe.SizeMax);
		bOutGenerated = true;
		return true;
	}

	bool WriteManifest(const TArray<FPBGeneratedVfxSet>& GeneratedSets)
	{
		TArray<TSharedPtr<FJsonValue>> Rows;
		for (const FPBGeneratedVfxSet& Set : GeneratedSets)
		{
			TSharedRef<FJsonObject> Row = MakeShared<FJsonObject>();
			Row->SetStringField(TEXT("EffectId"), Set.EffectId);
			Row->SetStringField(TEXT("ActivationVfxId"), Set.Activation);
			Row->SetStringField(TEXT("DeliveryVfxId"), Set.Delivery);
			Row->SetStringField(TEXT("ImpactVfxId"), Set.Impact);
			Row->SetStringField(TEXT("StatusVfxId"), Set.Status);
			Rows.Add(MakeShared<FJsonValueObject>(Row));
		}

		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		Root->SetStringField(TEXT("Schema"), TEXT("BumperVfxManifest.v2"));
		Root->SetArrayField(TEXT("Rows"), Rows);

		FString Json;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
		if (!FJsonSerializer::Serialize(Root, Writer))
		{
			return false;
		}

		const FString OutputDirectory = FPaths::ProjectSavedDir() / TEXT("Codex");
		IFileManager::Get().MakeDirectory(*OutputDirectory, true);
		return FFileHelper::SaveStringToFile(
			Json,
			*(OutputDirectory / TEXT("bumper_vfx_manifest.json")),
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

FName PBBumperVfxCatalog::CanonicalizeKnownVfxId(
	const FName EffectId,
	const EPBBumperVfxStage Stage,
	const FName VfxId)
{
	if (EffectId.IsNone() || VfxId.IsNone())
	{
		return VfxId;
	}

	for (const FPBEffectVisualProfile& Profile : GetVisualProfiles())
	{
		if (EffectId != FName(Profile.EffectId))
		{
			continue;
		}

		const FName LegacyId(*MakeLegacyStageAssetName(Profile, Stage));
		const FName SharedId(*MakeStageAssetName(Profile, Stage));
		return VfxId == LegacyId || VfxId == SharedId ? SharedId : VfxId;
	}

	return VfxId;
}

UPBBumperVfxGenerationCommandlet::UPBBumperVfxGenerationCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 UPBBumperVfxGenerationCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("[BumperVfx] Generation started. Params=%s"), *Params);
	FPBVfxGenerationPolicy Policy;
	Policy.bUpdateExisting = FParse::Param(*Params, TEXT("UpdateExisting"));
	Policy.bForceReplace = FParse::Param(*Params, TEXT("ForceReplace"));
	Policy.bUpdateExisting |= Policy.bForceReplace;
	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	UNiagaraEffectType* EffectType = CreateOrUpdateEffectType(Policy);
	if (!IsValid(EffectType))
	{
		UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to create or update %s."), EffectTypeAssetName);
		return 1;
	}
	TArray<FPBGeneratedVfxSet> GeneratedSets;
	TSet<FString> ProcessedAssetIds;
	int32 GeneratedSystemCount = 0;

	for (const FPBEffectVisualProfile& Profile : GetVisualProfiles())
	{
		FPBGeneratedVfxSet& Set = GeneratedSets.AddDefaulted_GetRef();
		Set.EffectId = Profile.EffectId;

		auto GenerateStage = [&](const EPBBumperVfxStage Stage, FString& OutAssetId) -> bool
		{
			const FPBVfxRecipe Recipe = MakeRecipe(Profile, Stage);
			OutAssetId = Recipe.AssetName;
			if (ProcessedAssetIds.Contains(Recipe.AssetName))
			{
				return true;
			}

			bool bGenerated = false;
			if (!GenerateSystem(AssetTools, EffectType, Recipe, Policy, bGenerated))
			{
				return false;
			}
			ProcessedAssetIds.Add(Recipe.AssetName);
			GeneratedSystemCount += bGenerated ? 1 : 0;
			return true;
		};

		if (!GenerateStage(EPBBumperVfxStage::Activation, Set.Activation)
			|| (Profile.bHasDelivery && !GenerateStage(EPBBumperVfxStage::Delivery, Set.Delivery))
			|| (Profile.bHasImpact && !GenerateStage(EPBBumperVfxStage::Impact, Set.Impact))
			|| (Profile.bHasStatus && !GenerateStage(EPBBumperVfxStage::Status, Set.Status)))
		{
			UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Generation aborted at EffectId=%s"), Profile.EffectId);
			return 1;
		}
	}

	if (!WriteManifest(GeneratedSets))
	{
		UE_LOG(LogTemp, Error, TEXT("[BumperVfx] Failed to write manifest."));
		return 1;
	}

	UE_LOG(LogTemp, Display,
		TEXT("[BumperVfx] Generation completed. Effects=%d Systems=%d"),
		GeneratedSets.Num(),
		GeneratedSystemCount);
	return 0;
}
