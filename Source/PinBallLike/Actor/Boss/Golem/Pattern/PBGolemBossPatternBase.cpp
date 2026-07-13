#include "PBGolemBossPatternBase.h"

#include "PinBallLike/Actor/Boss/Golem/PBGolemBoss.h"
#include "PinBallLike/Actor/Boss/Golem/PBGolemBossHand.h"

APBGolemBoss* UPBGolemBossPatternBase::GetGolemBoss() const
{
	APBGolemBoss* GolemBoss = Cast<APBGolemBoss>(GetOwnerBoss());
	UE_LOG(LogTemp, Log, TEXT("[GolemPatternBase] GetGolemBoss. Pattern=%s Boss=%s Success=%s"),
		*GetNameSafe(this),
		*GetNameSafe(GetOwnerBoss()),
		GolemBoss ? TEXT("true") : TEXT("false"));
	return GolemBoss;
}

APBGolemBoss* UPBGolemBossPatternBase::GetGolemBoss(APBBossBase* Boss) const
{
	APBGolemBoss* GolemBoss = Cast<APBGolemBoss>(Boss);
	UE_LOG(LogTemp, Log, TEXT("[GolemPatternBase] GetGolemBossFromBoss. Pattern=%s Boss=%s Success=%s"),
		*GetNameSafe(this),
		*GetNameSafe(Boss),
		GolemBoss ? TEXT("true") : TEXT("false"));
	return GolemBoss;
}

APBGolemBossHand* UPBGolemBossPatternBase::GetGolemHand(EPBGolemBossHandType HandType) const
{
	APBGolemBoss* GolemBoss = GetGolemBoss();
	APBGolemBossHand* GolemHand = GolemBoss ? GolemBoss->GetGolemHand(HandType) : nullptr;
	UE_LOG(LogTemp, Log, TEXT("[GolemPatternBase] GetGolemHand. Pattern=%s HandType=%d Hand=%s Success=%s"),
		*GetNameSafe(this),
		static_cast<int32>(HandType),
		*GetNameSafe(GolemHand),
		GolemHand ? TEXT("true") : TEXT("false"));
	return GolemHand;
}
