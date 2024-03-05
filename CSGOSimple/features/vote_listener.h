#pragma once
#include "../valve_sdk/interfaces/IGameEventmanager.hpp"
#include "../singleton.hpp"

class CallVoteEvent : public IGameEventListener2, public Singleton<CallVoteEvent>
{
public:

	void FireGameEvent(IGameEvent* event);
	int  GetEventDebugID(void);

	void RegisterSelf();
	void UnregisterSelf();

private:
	int potential_votes = 0;
};