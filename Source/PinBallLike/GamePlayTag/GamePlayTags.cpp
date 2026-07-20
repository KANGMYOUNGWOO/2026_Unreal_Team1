#include "GameplayTags.h"

namespace GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Deck_Drag_Started, "Event.UI.Deck.Drag.Started");
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Deck_Drag_Ended, "Event.UI.Deck.Drag.Ended");
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Collection_Notification, "Event.UI.Collection.Notification");
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_DamageLog_Requested, "Event.UI.DamageLog.Requested");

	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Phase_Prepare_Completed, "Event.Battle.Phase.Prepare.Completed");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Phase_Changed, "Event.Battle.Phase.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_LaunchCount_Changed, "Event.Battle.LaunchCount.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_ShiftCount_Changed, "Event.Battle.ShiftCount.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Party_Deployment_Started, "Event.Battle.Party.Deployment.Started");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Party_Launch_Requested, "Event.Battle.Party.Launch.Requested");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Party_Launch_Approved, "Event.Battle.Party.Launch.Approved");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Party_Launched, "Event.Battle.Party.Launched");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Party_AllBallsDead, "Event.Battle.Party.AllBallsDead");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Party_Shift_Requested, "Event.Battle.Party.Shift.Requested");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Skill_Use_Requested, "Event.Battle.Skill.Use.Requested");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Bumper_Triggered, "Event.Battle.Bumper.Triggered");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Boss_Intro_Completed, "Event.Battle.Boss.Intro.Completed");
	UE_DEFINE_GAMEPLAY_TAG(Event_Battle_Boss_Dead, "Event.Battle.Boss.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Event_UI_Choice_Exit, "Event.UI.Choice.Exit");

	UE_DEFINE_GAMEPLAY_TAG(TriggerEvent_Battle_Started, "TriggerEvent.Battle.Started");
	UE_DEFINE_GAMEPLAY_TAG(TriggerEvent_Battle_PartyBuilt, "TriggerEvent.Battle.PartyBuilt");
	UE_DEFINE_GAMEPLAY_TAG(TriggerEvent_Battle_PartySwitched, "TriggerEvent.Battle.PartySwitched");
	UE_DEFINE_GAMEPLAY_TAG(TriggerEvent_Battle_EnemyHit, "TriggerEvent.Battle.EnemyHit");
	UE_DEFINE_GAMEPLAY_TAG(TriggerEvent_Battle_AfterDamage, "TriggerEvent.Battle.AfterDamage");
	UE_DEFINE_GAMEPLAY_TAG(TriggerEvent_Battle_Revived, "TriggerEvent.Battle.Revived");
	UE_DEFINE_GAMEPLAY_TAG(TriggerEvent_Shop_Opened, "TriggerEvent.Shop.Opened");
}
