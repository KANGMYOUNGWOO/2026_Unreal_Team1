#include "PBBossStateTreeTags.h"

namespace PBBossStateTreeTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(RequestIdle, "Boss.State.Request.Idle", "Requests the boss StateTree to enter idle state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(RequestPattern, "Boss.State.Request.Pattern", "Requests the boss StateTree to enter pattern state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(RequestGroggy, "Boss.State.Request.Groggy", "Requests the boss StateTree to enter groggy state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(RequestEnraged, "Boss.State.Request.Enraged", "Requests the boss StateTree to enter enraged state.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(RequestDead, "Boss.State.Request.Dead", "Requests the boss StateTree to enter dead state.");
}
