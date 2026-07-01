// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// 이 파일은 PinBallLike 모듈이 어떤 언리얼 엔진 기능을 사용할지 빌드 시스템에 알려줍니다.
public class PinBallLike : ModuleRules
{
	// Target은 에디터 빌드인지, 게임 빌드인지 같은 빌드 대상을 담고 있습니다.
	public PinBallLike(ReadOnlyTargetRules Target) : base(Target)
	{
		// 명시적 PCH 설정은 필요한 헤더를 각 cpp에서 직접 include하도록 유도해 의존성을 파악하기 쉽게 만듭니다.
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// PublicDependencyModuleNames는 이 모듈의 헤더와 cpp에서 공개적으로 사용할 엔진 모듈 목록입니다.
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"UMG", 
			"ModelViewViewModel",
			"StateTreeModule",
			"GameplayStateTreeModule"
		});

		// Slate와 SlateCore는 도감 UI에서 FSlateBrush, FButtonStyle 같은 Slate 타입을 직접 만들기 위해 필요합니다.
		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore"
		});
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
