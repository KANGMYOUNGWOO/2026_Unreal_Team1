#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperSharedEffectAdapter.h"
#include "PinBallLike/Effect/Handlers/PBEffectHandler.h"
#include "PinBallLike/Struct/Effect/PBEffectTypes.h"
#include "PinBallLike/Table/Effect/Struct/PBEffectTableRow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperSharedEffectContractTest,
	"PinBallLike.Bumper.Effect.SharedContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperSharedEffectContractTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FPBEffectTableRow EffectRow;
	EffectRow.EffectType = TEXT("StatBuff");
	EffectRow.TargetType = TEXT("Ball");
	EffectRow.TargetFilter = TEXT("All");

	FString Error;
	TestTrue(
		TEXT("Matching Effect contract is accepted"),
		PBBumperSharedEffectAdapter::ValidateContract(
			EffectRow,
			TEXT("StatBuff"),
			TEXT("Ball"),
			TEXT("All"),
			Error));
	TestTrue(TEXT("Accepted contract has no error"), Error.IsEmpty());

	TestFalse(
		TEXT("Mismatched TargetType is rejected"),
		PBBumperSharedEffectAdapter::ValidateContract(
			EffectRow,
			TEXT("StatBuff"),
			TEXT("Battle"),
			TEXT("All"),
			Error));
	TestTrue(TEXT("TargetType failure identifies the broken field"), Error.Contains(TEXT("TargetType mismatch")));

	TestFalse(
		TEXT("Mismatched TargetFilter is rejected"),
		PBBumperSharedEffectAdapter::ValidateContract(
			EffectRow,
			TEXT("StatBuff"),
			TEXT("Ball"),
			TEXT("Leader"),
			Error));
	TestTrue(
		TEXT("TargetFilter failure identifies the broken field"),
		Error.Contains(TEXT("TargetFilter mismatch")));

	EffectRow.EffectType = TEXT("UnknownEffectType");
	TestFalse(
		TEXT("Unsupported EffectType is rejected"),
		PBBumperSharedEffectAdapter::ValidateContract(
			EffectRow,
			NAME_None,
			TEXT("Ball"),
			TEXT("All"),
			Error));
	TestTrue(
		TEXT("Unsupported EffectType failure is explicit"),
		Error.Contains(TEXT("not supported")));

	UPBEffectHandler* Handler = NewObject<UPBEffectHandler>();
	const FName HandlerCompatibleTypes[] =
	{
		PBEffectTypes::EffectType::ResourceCostStatBuff,
		PBEffectTypes::EffectType::ComboExtraDamage,
		PBEffectTypes::EffectType::PostDamageHeal,
		PBEffectTypes::EffectType::StatBuff
	};
	for (const FName EffectType : HandlerCompatibleTypes)
	{
		TestTrue(
			*FString::Printf(TEXT("Bumper shared type remains supported by UPBEffectHandler: %s"), *EffectType.ToString()),
			Handler->IsEffectTypeSupported(EffectType));
		TestTrue(
			*FString::Printf(TEXT("Bumper adapter classifies the type as Handler-compatible: %s"), *EffectType.ToString()),
			PBBumperSharedEffectAdapter::IsHandlerCompatibleEffectType(EffectType));
	}

	const FName VelocityScaledDamage(TEXT("VelocityScaledDamage"));
	TestFalse(
		TEXT("VelocityScaledDamage is not silently registered in the team Handler"),
		Handler->IsEffectTypeSupported(VelocityScaledDamage));
	TestTrue(
		TEXT("VelocityScaledDamage remains an explicit Bumper extension"),
		PBBumperSharedEffectAdapter::IsBumperExtensionEffectType(VelocityScaledDamage));

	return true;
}

#endif
