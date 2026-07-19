#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "PinBallLike/Actor/Bumper/Effect/PBBumperSharedEffectAdapter.h"
#include "PinBallLike/Table/Effect/Struct/PBGameplayEffectRow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPBBumperSharedEffectContractTest,
	"PinBallLike.Bumper.Effect.SharedContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPBBumperSharedEffectContractTest::RunTest(const FString& Parameters)
{
	static_cast<void>(Parameters);

	FPBGameplayEffectRow EffectRow;
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

	return true;
}

#endif
