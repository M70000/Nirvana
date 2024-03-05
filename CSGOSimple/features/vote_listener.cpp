#include "vote_listener.h"
#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"

void CallVoteEvent::FireGameEvent(IGameEvent* event)
{
	if (!g_LocalPlayer || !event)
		return;

	if (!strcmp(event->GetName(), "vote_changed"))
	{
		potential_votes = event->GetInt("potentialVotes");
	}
	else if (!strcmp(event->GetName(), "vote_cast"))
	{
		auto vote_option = event->GetInt("vote_option");
		auto vote_player_id = event->GetInt("entityid");

		if (vote_player_id == g_EngineClient->GetLocalPlayer() && vote_option == 1)
		{
			if (g_Options.misc_antivotekick)
				g_EngineClient->ExecuteClientCmd("callvote ScrambleTeams");
		}
	}
}

int CallVoteEvent::GetEventDebugID(void)
{
	return EVENT_DEBUG_ID_INIT;
}

void CallVoteEvent::RegisterSelf()
{
	g_GameEvents->AddListener(this, "vote_changed", false);
	g_GameEvents->AddListener(this, "vote_cast", false);
}

void CallVoteEvent::UnregisterSelf()
{
	g_GameEvents->RemoveListener(this);
}