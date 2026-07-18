#if WITH_DEV_AUTOMATION_TESTS

#include "AdvancedPreviewScene.h"
#include "AssetCompilingManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/App.h"
#include "Misc/AutomationTest.h"
#include "NiagaraComponent.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraGpuComputeDispatchInterface.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraSystemInstanceController.h"
#include "NiagaraWorldManager.h"
#include "PinBallLike/Actor/Bumper/Summon/PBGateFieldTuning.h"
#include "RenderingThread.h"
#include "Stateless/NiagaraStatelessEmitter.h"
#include "Stateless/Modules/NiagaraStatelessModule_ShapeLocation.h"

namespace
{
	constexpr TCHAR VfxFolder[] = TEXT("/Game/Blueprints/Bumper/Effect/VFX");
	constexpr int32 PreviewWidth = 1600;
	constexpr int32 PreviewHeight = 900;
	constexpr float PreviewDeltaSeconds = 1.0f / 60.0f;

	struct FPBPreviewEntry
	{
		const TCHAR* AssetName;
		FVector Location;
	};

	const TArray<FPBPreviewEntry>& GetBurstPreviewEntries()
	{
		static const TArray<FPBPreviewEntry> Entries =
		{
			{TEXT("NS_Bumper_Impact_DirectStrike"), FVector(-510.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Impact_BossGroggy"), FVector(-170.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Impact_ComboArc"), FVector(170.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Impact_PartyMana"), FVector(510.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Impact_RecoveryField"), FVector(-510.0, 210.0, 0.0)},
			{TEXT("NS_Bumper_Impact_ShieldCharge"), FVector(-170.0, 210.0, 0.0)},
			{TEXT("NS_Bumper_Impact_SpeedUp"), FVector(170.0, 210.0, 0.0)},
			{TEXT("NS_Bumper_Impact_VulnerabilityShell"), FVector(510.0, 210.0, 0.0)}
		};
		return Entries;
	}

	const TArray<FPBPreviewEntry>& GetSustainedPreviewEntries()
	{
		static const TArray<FPBPreviewEntry> Entries =
		{
			{TEXT("NS_Bumper_Status_BloodOverdrive"), FVector(-510.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Status_CounterShield"), FVector(-170.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Status_ManaReactor"), FVector(170.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Status_RecoveryField"), FVector(510.0, -210.0, 0.0)},
			{TEXT("NS_Bumper_Status_SpeedUp"), FVector(-510.0, 210.0, 0.0)},
			{TEXT("NS_Bumper_Status_VulnerabilityShell"), FVector(-170.0, 210.0, 0.0)},
			{TEXT("NS_Bumper_Delivery_DirectStrike"), FVector(170.0, 210.0, 0.0)},
			{TEXT("NS_Bumper_Delivery_LaunchCharge"), FVector(510.0, 210.0, 0.0)}
		};
		return Entries;
	}

	FString MakeSystemObjectPath(const TCHAR* AssetName)
	{
		if (AssetName[0] == TEXT('/'))
		{
			return AssetName;
		}
		return FString::Printf(TEXT("%s/%s.%s"), VfxFolder, AssetName, AssetName);
	}

	bool ValidateGateAreaVfxRadius(FAutomationTestBase& Test)
	{
		static constexpr const TCHAR* GateAreaStatusSystems[] =
		{
			TEXT("NS_Bumper_Status_SpeedUp"),
			TEXT("NS_Bumper_Status_RecoveryField"),
			TEXT("NS_Bumper_Status_ReactiveRepair"),
			TEXT("NS_Bumper_Status_ManaReactor")
		};

		bool bAllValid = true;
		for (const TCHAR* AssetName : GateAreaStatusSystems)
		{
			UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *MakeSystemObjectPath(AssetName));
			if (!Test.TestNotNull(
				*FString::Printf(TEXT("Gate area VFX resolves: %s"), AssetName),
				System))
			{
				bAllValid = false;
				continue;
			}

			const TArray<FNiagaraEmitterHandle>& EmitterHandles = System->GetEmitterHandles();
			if (!Test.TestTrue(
				*FString::Printf(TEXT("Gate area VFX has a primary layer: %s"), AssetName),
				!EmitterHandles.IsEmpty()))
			{
				bAllValid = false;
				continue;
			}

			const UNiagaraStatelessEmitter* PrimaryEmitter =
				EmitterHandles[0].GetStatelessEmitter();
			const UNiagaraStatelessModule_ShapeLocation* Shape = PrimaryEmitter
				? PrimaryEmitter->GetModule<UNiagaraStatelessModule_ShapeLocation>()
				: nullptr;
			if (!Test.TestNotNull(
				*FString::Printf(TEXT("Gate area VFX has a Shape Location module: %s"), AssetName),
				Shape))
			{
				bAllValid = false;
				continue;
			}

			const bool bUsesAreaShape = Shape->ShapePrimitive == ENSM_ShapePrimitive::Ring
				|| Shape->ShapePrimitive == ENSM_ShapePrimitive::Sphere;
			bAllValid &= Test.TestTrue(
				*FString::Printf(TEXT("Gate area VFX uses a radial shape: %s"), AssetName),
				bUsesAreaShape);
			if (!bUsesAreaShape)
			{
				continue;
			}

			const FNiagaraDistributionRangeFloat& ActiveRadius =
				Shape->ShapePrimitive == ENSM_ShapePrimitive::Ring
					? Shape->RingRadius
					: Shape->SphereRadius;
			bAllValid &= Test.TestEqual(
				*FString::Printf(TEXT("Gate area VFX minimum radius: %s"), AssetName),
				ActiveRadius.Min,
				PBGateFieldTuning::DefaultRadius);
			bAllValid &= Test.TestEqual(
				*FString::Printf(TEXT("Gate area VFX maximum radius: %s"), AssetName),
				ActiveRadius.Max,
				PBGateFieldTuning::DefaultRadius);
		}
		return bAllValid;
	}

	bool SaveRenderTargetAsPng(UTextureRenderTarget2D* RenderTarget, const FString& OutputPath)
	{
		if (!IsValid(RenderTarget))
		{
			return false;
		}

		TUniquePtr<FArchive> Writer(IFileManager::Get().CreateFileWriter(*OutputPath));
		if (!Writer)
		{
			return false;
		}

		const bool bExported = FImageUtils::ExportRenderTarget2DAsPNG(RenderTarget, *Writer);
		Writer->Close();
		return bExported && IFileManager::Get().FileSize(*OutputPath) > 0;
	}

	bool ValidatePreviewCoverage(
		FAutomationTestBase& Test,
		UTextureRenderTarget2D* RenderTarget,
		const FString& SetName)
	{
		if (!IsValid(RenderTarget))
		{
			return false;
		}

		FTextureRenderTargetResource* Resource = RenderTarget->GameThread_GetRenderTargetResource();
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s render resource resolves"), *SetName), Resource))
		{
			return false;
		}

		TArray<FColor> Pixels;
		if (!Test.TestTrue(*FString::Printf(TEXT("%s pixels are readable"), *SetName), Resource->ReadPixels(Pixels)))
		{
			return false;
		}

		const int32 ExpectedPixelCount = PreviewWidth * PreviewHeight;
		if (!Test.TestEqual(
			*FString::Printf(TEXT("%s pixel count"), *SetName),
			Pixels.Num(),
			ExpectedPixelCount))
		{
			return false;
		}

		constexpr int32 ColumnCount = 4;
		constexpr int32 RowCount = 2;
		constexpr uint8 BrightnessThreshold = 48;
		constexpr int32 MinimumBrightPixelCount = 8;
		bool bAllCellsCovered = true;
		FString CoverageSummary;
		for (int32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
		{
			for (int32 ColumnIndex = 0; ColumnIndex < ColumnCount; ++ColumnIndex)
			{
				const int32 MinX = ColumnIndex * PreviewWidth / ColumnCount;
				const int32 MaxX = (ColumnIndex + 1) * PreviewWidth / ColumnCount;
				const int32 MinY = RowIndex * PreviewHeight / RowCount;
				const int32 MaxY = (RowIndex + 1) * PreviewHeight / RowCount;
				int32 BrightPixelCount = 0;
				for (int32 Y = MinY; Y < MaxY; ++Y)
				{
					for (int32 X = MinX; X < MaxX; ++X)
					{
						const FColor& Pixel = Pixels[Y * PreviewWidth + X];
						if (FMath::Max3(Pixel.R, Pixel.G, Pixel.B) >= BrightnessThreshold)
						{
							++BrightPixelCount;
						}
					}
				}

				const bool bCellCovered = BrightPixelCount >= MinimumBrightPixelCount;
				CoverageSummary += FString::Printf(
					TEXT("[%d,%d]=%d "),
					ColumnIndex,
					RowIndex,
					BrightPixelCount);
				bAllCellsCovered &= Test.TestTrue(
					*FString::Printf(
						TEXT("%s cell %d,%d contains rendered VFX (%d bright pixels)"),
						*SetName,
						ColumnIndex,
						RowIndex,
						BrightPixelCount),
					bCellCovered);
			}
		}
		Test.AddInfo(FString::Printf(TEXT("%s coverage: %s"), *SetName, *CoverageSummary));
		return bAllCellsCovered;
	}

	void TickPreviewWorld(UWorld* World, const float DurationSeconds)
	{
		const int32 FrameCount = FMath::Max(1, FMath::CeilToInt(DurationSeconds / PreviewDeltaSeconds));
		for (int32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
		{
			World->Tick(LEVELTICK_All, PreviewDeltaSeconds);
		}
		FlushRenderingCommands();
	}

	bool RenderPreviewSet(
		FAutomationTestBase& Test,
		FAdvancedPreviewScene& PreviewScene,
		UWorld* World,
		USceneCaptureComponent2D* CaptureComponent,
		UTextureRenderTarget2D* RenderTarget,
		const TArray<FPBPreviewEntry>& Entries,
		const float WarmupSeconds,
		const FString& SetName,
		const FString& OutputPath)
	{
		TArray<UNiagaraSystem*> LoadedSystems;
		LoadedSystems.Reserve(Entries.Num());
		for (const FPBPreviewEntry& Entry : Entries)
		{
			UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *MakeSystemObjectPath(Entry.AssetName));
			if (!Test.TestNotNull(
				*FString::Printf(TEXT("Preview system resolves: %s"), Entry.AssetName),
				System))
			{
				return false;
			}
			LoadedSystems.Add(System);
		}

		// 첫 시스템만 비동기 로드/컴파일 대기 상태로 남아 즉시 완료되는 순서 의존성을 제거합니다.
		FlushAsyncLoading();
		UMaterialInterface::SubmitRemainingJobsForWorld(World);
		FAssetCompilingManager::Get().FinishAllCompilation();

		TArray<UNiagaraComponent*> SpawnedComponents;
		SpawnedComponents.Reserve(Entries.Num());

		for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
		{
			const FPBPreviewEntry& Entry = Entries[EntryIndex];
			UNiagaraSystem* System = LoadedSystems[EntryIndex];
			UNiagaraComponent* Component = NewObject<UNiagaraComponent>(
				GetTransientPackage(),
				NAME_None,
				RF_Transient);
			if (!Test.TestNotNull(
				*FString::Printf(TEXT("Preview component spawns: %s"), Entry.AssetName),
				Component))
			{
				return false;
			}

			Component->SetAsset(System);
			Component->SetForceSolo(true);
			Component->SetAllowScalability(false);
			Component->SetRenderingEnabled(true);
			Component->SetAgeUpdateMode(ENiagaraAgeUpdateMode::DesiredAge);
			Component->SetCanRenderWhileSeeking(true);
			Component->SetMaxSimTime(0.0f);
			PreviewScene.AddComponent(Component, FTransform(Entry.Location));
			// 컴포넌트를 World에 등록한 뒤 활성화해야 Burst가 등록 전 완료되는 순서 의존성을 피할 수 있습니다.
			Component->Activate(true);
			if (!Test.TestTrue(
				*FString::Printf(TEXT("Preview component registers: %s"), Entry.AssetName),
				Component->IsRegistered()))
			{
				return false;
			}
			// Stateless Burst는 활성화 직후 아직 DesiredAge를 적용하지 않은 시점에 Complete로 보일 수 있습니다.
			// 실제 유효성은 아래의 지정 시점 시뮬레이션 결과(파티클 수와 렌더 픽셀)로 검증합니다.
			SpawnedComponents.Add(Component);
		}

		World->TimeSeconds = WarmupSeconds;
		World->UnpausedTimeSeconds = WarmupSeconds;
		World->RealTimeSeconds = WarmupSeconds;
		World->DeltaRealTimeSeconds = PreviewDeltaSeconds;
		World->DeltaTimeSeconds = PreviewDeltaSeconds;
		for (UNiagaraComponent* Component : SpawnedComponents)
		{
			Component->SetSeekDelta(PreviewDeltaSeconds);
			Component->SeekToDesiredAge(WarmupSeconds);
		}
		World->Tick(LEVELTICK_PauseTick, 0.0f);
		for (UNiagaraComponent* Component : SpawnedComponents)
		{
			Component->TickComponent(PreviewDeltaSeconds, LEVELTICK_All, nullptr);
		}
		World->SendAllEndOfFrameUpdates();
		if (FNiagaraWorldManager* WorldManager = FNiagaraWorldManager::Get(World))
		{
			WorldManager->FlushComputeAndDeferredQueues(false);
		}
		FlushRenderingCommands();
		for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
		{
			const UNiagaraComponent* Component = SpawnedComponents[EntryIndex];
			const FNiagaraSystemInstanceControllerConstPtr Controller =
				Component->GetSystemInstanceController();
			const FNiagaraSystemInstance* Instance = Controller.IsValid()
				? Controller->GetSystemInstance_Unsafe()
				: nullptr;
			int32 ParticleCount = 0;
			if (Instance)
			{
				for (const FNiagaraEmitterInstanceRef& Emitter : Instance->GetEmitters())
				{
					ParticleCount += Emitter->GetNumParticles();
				}
			}
			Test.TestTrue(
				*FString::Printf(TEXT("Preview emits particles: %s"), Entries[EntryIndex].AssetName),
				ParticleCount > 0);
			Test.AddInfo(FString::Printf(
				TEXT("%s particles=%d age=%.3f"),
				Entries[EntryIndex].AssetName,
				ParticleCount,
				Instance ? Instance->GetAge() : -1.0f));
		}
		CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		CaptureComponent->ShowOnlyComponents.Empty(SpawnedComponents.Num());
		for (UNiagaraComponent* Component : SpawnedComponents)
		{
			CaptureComponent->ShowOnlyComponents.Add(Component);
		}
		// 첫 렌더는 새로 생성된 머티리얼의 셰이더 요청을 발생시키는 준비 프레임이다.
		CaptureComponent->CaptureScene();
		FlushRenderingCommands();
		FAssetCompilingManager::Get().FinishAllCompilation();
		CaptureComponent->CaptureScene();
		FlushRenderingCommands();
		const bool bCoverageValid = ValidatePreviewCoverage(Test, RenderTarget, SetName);
		const bool bSaved = SaveRenderTargetAsPng(RenderTarget, OutputPath);
		Test.TestTrue(*FString::Printf(TEXT("Preview PNG saves: %s"), *OutputPath), bSaved);

		for (UNiagaraComponent* Component : SpawnedComponents)
		{
			if (IsValid(Component))
			{
				Component->DeactivateImmediate();
				PreviewScene.RemoveComponent(Component);
				Component->DestroyComponent();
			}
		}
		CaptureComponent->ShowOnlyComponents.Empty();
		TickPreviewWorld(World, PreviewDeltaSeconds);
		return bCoverageValid && bSaved;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperVfxVisualPreviewTest,
	"PinBallLike.Bumper.Vfx.VisualPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperVfxVisualPreviewTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);
	if (!ValidateGateAreaVfxRadius(*this))
	{
		return false;
	}
	if (!FApp::CanEverRender())
	{
		AddInfo(TEXT("NullRHI 환경에서는 구조 검증만 수행하고 렌더 프리뷰는 건너뜁니다."));
		return true;
	}
	if (!TestNotNull(TEXT("Engine is available"), GEngine))
	{
		return false;
	}
	TSharedPtr<FAdvancedPreviewScene> PreviewScene = MakeShared<FAdvancedPreviewScene>(
		FPreviewScene::ConstructionValues().SetCreatePhysicsScene(false).AllowAudioPlayback(false));
	PreviewScene->SetFloorVisibility(false);
	UWorld* World = PreviewScene->GetWorld();
	if (!TestNotNull(TEXT("Preview world is created"), World))
	{
		return false;
	}

	const bool bHasWorldManager = FNiagaraWorldManager::Get(World) != nullptr;
	const bool bHasComputeDispatch = FNiagaraGpuComputeDispatchInterface::Get(World) != nullptr;
	if (!TestTrue(TEXT("Preview world has a Niagara world manager"), bHasWorldManager)
		|| !TestTrue(TEXT("Preview world has a Niagara compute dispatch interface"), bHasComputeDispatch))
	{
		return false;
	}

	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(
		GetTransientPackage(),
		TEXT("RT_BumperVfxPreview"),
		RF_Transient);
	RenderTarget->RenderTargetFormat = RTF_RGBA8_SRGB;
	RenderTarget->ClearColor = FLinearColor(0.008f, 0.012f, 0.018f, 1.0f);
	RenderTarget->InitAutoFormat(PreviewWidth, PreviewHeight);
	RenderTarget->UpdateResourceImmediate(true);

	USceneCaptureComponent2D* CaptureComponent = NewObject<USceneCaptureComponent2D>(
		GetTransientPackage(),
		NAME_None,
		RF_Transient);
	if (!TestNotNull(TEXT("Scene capture component is created"), CaptureComponent))
	{
		return false;
	}

	CaptureComponent->RegisterComponentWithWorld(World);
	CaptureComponent->SetWorldLocationAndRotation(
		FVector(0.0, 0.0, 1200.0),
		FRotator(-90.0, 0.0, 90.0));
	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureComponent->OrthoWidth = 1500.0f;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	CaptureComponent->ShowFlags.SetParticles(true);
	CaptureComponent->ShowFlags.SetNiagara(true);
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->PostProcessSettings.bOverride_AutoExposureMethod = true;
	CaptureComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	CaptureComponent->PostProcessSettings.bOverride_AutoExposureBias = true;
	CaptureComponent->PostProcessSettings.AutoExposureBias = 0.0f;

	const FString OutputDirectory = FPaths::ProjectSavedDir() / TEXT("Codex");
	IFileManager::Get().MakeDirectory(*OutputDirectory, true);
	const FString BurstOutputPath = OutputDirectory / TEXT("bumper_vfx_burst_preview.png");
	const FString SustainedOutputPath = OutputDirectory / TEXT("bumper_vfx_sustained_preview.png");

	const bool bBurstSaved = RenderPreviewSet(
		*this,
		*PreviewScene,
		World,
		CaptureComponent,
		RenderTarget,
		GetBurstPreviewEntries(),
		0.18f,
		TEXT("Burst preview"),
		BurstOutputPath);
	const bool bSustainedSaved = bBurstSaved && RenderPreviewSet(
		*this,
		*PreviewScene,
		World,
		CaptureComponent,
		RenderTarget,
		GetSustainedPreviewEntries(),
		0.82f,
		TEXT("Sustained preview"),
		SustainedOutputPath);

	CaptureComponent->UnregisterComponent();
	CaptureComponent->DestroyComponent();
	PreviewScene.Reset();

	if (bBurstSaved && bSustainedSaved)
	{
		AddInfo(FString::Printf(TEXT("Burst preview: %s"), *BurstOutputPath));
		AddInfo(FString::Printf(TEXT("Sustained preview: %s"), *SustainedOutputPath));
	}
	return bBurstSaved && bSustainedSaved;
}

#endif
