using UnrealBuildTool;

using System.IO;

public class PinballLikeEditor : ModuleRules
{
    public PinballLikeEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "PinBallLike",
                "GoogleSheetLoader",
                "Paper2D",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "AdvancedPreviewScene",
                "AssetTools",
                "BlueprintEditorLibrary",
                "HTTP",
                "ImageCore",
                "Json",
                "KismetCompiler",
                "MaterialEditor",
                "Niagara",
                "NiagaraEditor",
                "NiagaraShader",
                "RenderCore",
                "Slate",
                "SlateCore"
            }
        );

        // UE 5.7의 Stateless Niagara 편집 API는 아직 Internal에 있다.
        // 런타임 모듈에는 노출하지 않고, 범퍼 VFX 생성용 Editor 모듈에서만 사용한다.
        PrivateIncludePaths.Add(
            Path.Combine(EngineDirectory, "Plugins", "FX", "Niagara", "Source", "Niagara", "Internal")
        );
        PrivateIncludePaths.Add(
            Path.Combine(EngineDirectory, "Plugins", "FX", "Niagara", "Source", "NiagaraShader", "Internal")
        );
    }
}
