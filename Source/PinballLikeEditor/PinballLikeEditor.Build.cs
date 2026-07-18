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

        PrivateIncludePaths.Add(
            Path.Combine(EngineDirectory, "Plugins", "FX", "Niagara", "Source", "Niagara", "Internal")
        );
        PrivateIncludePaths.Add(
            Path.Combine(EngineDirectory, "Plugins", "FX", "Niagara", "Source", "NiagaraShader", "Internal")
        );
    }
}
